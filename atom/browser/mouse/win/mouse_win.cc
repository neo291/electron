// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/mouse/win/mouse_win.h"

#define WM_STOP (WM_USER + 0)

namespace atom {

// static
Mouse* Mouse::Create() {
  std::unique_ptr<MouseWin> mouse(new MouseWin);
  if (!mouse->Init())
    return nullptr;

  return mouse.release();
}

MouseWin::MouseWin() {}

MouseWin::~MouseWin() {
  uv_mutex_lock(&init_lock);

  while (thread_id == NULL)
    uv_cond_wait(&init_cond, &init_lock);

  DWORD id = thread_id;

  uv_mutex_unlock(&init_lock);

  PostThreadMessage(id, WM_STOP, NULL, NULL);
}

bool MouseWin::Init() {
  POINT point;
  if (!GetCursorPos(&point)) {
    point.x = 0;
    point.y = 0;
  }

  return Mouse::Init(point.x, point.y);
}

void MouseWin::ProcessMessages() {
  MSG msg;

  uv_mutex_lock(&init_lock);

  PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
  thread_id = GetCurrentThreadId();

  uv_cond_signal(&init_cond);
  uv_mutex_unlock(&init_lock);

  HHOOK hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

  while (GetMessage(&msg, NULL, 0, 0)) {
    if (msg.message == WM_STOP)
      break;
  }

  UnhookWindowsHookEx(hook);

  uv_mutex_lock(&init_lock);
  thread_id = NULL;
  uv_mutex_unlock(&init_lock);
}

void MouseWin::HandleMessage(WPARAM wParam, LPARAM lParam) {
  uv_mutex_lock(&event_lock);

  MessageType type;
  if ((type = WinToMouseMessageType(wParam)) != MessageType::None) {
    MSLLHOOKSTRUCT* data = (MSLLHOOKSTRUCT*)lParam;
    POINT point = data->pt;

    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST),
                   &monitor_info);
    RECT rcWork = monitor_info.rcWork;
    point.x = std::max(rcWork.left, std::min(rcWork.right, point.x));
    point.y = std::max(rcWork.top, std::min(rcWork.bottom, point.y));

    AddMessageToQueue(type, point.x, point.y, data->time);
  }

  uv_mutex_unlock(&event_lock);
}

// static
Mouse::MessageType MouseWin::WinToMouseMessageType(WPARAM wParam) {
  switch (wParam) {
    case WM_LBUTTONDOWN:
      return MessageType::LButtonDown;

    case WM_LBUTTONUP:
      return MessageType::LButtonUp;

    case WM_MBUTTONDOWN:
      return MessageType::MButtonDown;

    case WM_MBUTTONUP:
      return MessageType::MButtonUp;

    case WM_RBUTTONDOWN:
      return MessageType::RButtonDown;

    case WM_RBUTTONUP:
      return MessageType::RButtonUp;

    case WM_MOUSEMOVE:
      return MessageType::Move;
  }

  return MessageType::None;
}

// static
LRESULT CALLBACK MouseWin::LowLevelMouseProc(int nCode,
                                             WPARAM wParam,
                                             LPARAM lParam) {
  MouseWin* mouse;
  if (nCode >= 0 && (mouse = (MouseWin*)Mouse::GetMouse()))
    mouse->HandleMessage(wParam, lParam);

  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

}  // namespace atom
