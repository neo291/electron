// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/mouse/mouse.h"

#include "atom/browser/mouse/mouse_delegate.h"

namespace atom {

Mouse* Mouse::instance_ = nullptr;

Mouse* Mouse::GetMouse() {
  if (!instance_) {
    instance_ = Mouse::Create();
  }

  return instance_;
}

Mouse::Mouse() {
  has_full_move_resolution = false;

  last_event.button = MouseEvent::Button::None;
  last_event.buttons = MouseEvent::ButtonMask::None;
  last_event.movement_x = 0;
  last_event.movement_y = 0;
  last_event.screen_x = 0;
  last_event.screen_y = 0;
  last_event.which = MouseEvent::Which::None;
  last_event.type = MouseEvent::Type::None;

  last_click_which = MouseEvent::Which::None;
  last_click_time = 0;
}

Mouse::~Mouse() {
  uv_thread_join(&thread);

  uv_close((uv_handle_t*)async, OnClose);

  uv_mutex_destroy(&event_lock);
  uv_mutex_destroy(&init_lock);
  uv_cond_destroy(&init_cond);
}

bool Mouse::Init(long initial_x, long initial_y) {
  last_event.screen_x = initial_x;
  last_event.screen_y = initial_y;

  uv_mutex_init(&init_lock);
  uv_mutex_init(&event_lock);
  uv_cond_init(&init_cond);

  async = new uv_async_t;
  async->data = this;

  uv_async_init(uv_default_loop(), async, OnSend);
  uv_thread_create(&thread, RunThread, this);

  return true;
}

void Mouse::AddMessageToQueue(MessageType type,
                              long x,
                              long y,
                              unsigned long time) {
  Message event;
  event.type = type;
  event.x = x;
  event.y = y;
  event.time = time;

  message_queue.push(event);

  if (message_queue.size() == 1) {
    uv_async_send(async);
  }
}

bool Mouse::HasFullMoveResolution() {
  return has_full_move_resolution;
}

void Mouse::SetFullMoveResolution(bool value) {
  has_full_move_resolution = value;
}

void Mouse::HandleSend() {
  uv_mutex_lock(&event_lock);

  while (message_queue.size() > 0) {
    Message message = message_queue.front();
    message_queue.pop();

    bool button_pressed = false;
    MouseEvent::Button button_number = MouseEvent::Button::None;
    int buttons = static_cast<int>(last_event.buttons);
    long point_x = message.x;
    long point_y = message.y;
    int movement_x = 0;
    int movement_y = 0;
    MouseEvent::Which button_which = MouseEvent::Which::None;
    int type = static_cast<int>(MouseEvent::Type::None);
    bool process_buttons = false;

    MouseEvent::ButtonMask button_mask = MouseEvent::ButtonMask::None;
    switch (message.type) {
      case MessageType::LButtonDown:
        button_pressed = true;
        button_number = MouseEvent::Button::Left;
        button_mask = MouseEvent::ButtonMask::Left;
        button_which = MouseEvent::Which::Left;
        process_buttons = true;
        break;

      case MessageType::LButtonUp:
        button_pressed = false;
        button_number = MouseEvent::Button::Left;
        button_mask = MouseEvent::ButtonMask::Left;
        button_which = MouseEvent::Which::Left;
        process_buttons = true;
        break;

      case MessageType::MButtonDown:
        button_pressed = true;
        button_number = MouseEvent::Button::Middle;
        button_mask = MouseEvent::ButtonMask::Middle;
        button_which = MouseEvent::Which::Middle;
        process_buttons = true;
        break;

      case MessageType::MButtonUp:
        button_pressed = false;
        button_number = MouseEvent::Button::Middle;
        button_mask = MouseEvent::ButtonMask::Middle;
        button_which = MouseEvent::Which::Middle;
        process_buttons = true;
        break;

      case MessageType::RButtonDown:
        button_pressed = true;
        button_number = MouseEvent::Button::Right;
        button_mask = MouseEvent::ButtonMask::Right;
        button_which = MouseEvent::Which::Right;
        process_buttons = true;
        break;

      case MessageType::RButtonUp:
        button_pressed = false;
        button_number = MouseEvent::Button::Right;
        button_mask = MouseEvent::ButtonMask::Right;
        button_which = MouseEvent::Which::Right;
        process_buttons = true;
        break;

      case MessageType::Move:
        movement_x = point_x - last_event.screen_x;
        movement_y = point_y - last_event.screen_y;
        type |= static_cast<int>(MouseEvent::Type::Move);

        if (!has_full_move_resolution &&
            last_event.type == MouseEvent::Type::Move) {
          movement_x += last_event.movement_x;
          movement_y += last_event.movement_y;
        }

        last_click_which = MouseEvent::Which::None;
        last_click_time = 0;
        break;

      case MessageType::None:
        break;
    }

    if (process_buttons) {
      if (button_pressed) {
        buttons |= static_cast<int>(button_mask);
        type |= static_cast<int>(MouseEvent::Type::Down);
      } else {
        buttons &= ~(static_cast<int>(button_mask));
        type |= static_cast<int>(MouseEvent::Type::Up);

        unsigned long time = message.time;
        if (last_click_which == button_which &&
            (time - last_click_time) <= GetDoubleClickTime()) {
          last_click_which = MouseEvent::Which::None;
          last_click_time = 0;
          type |= static_cast<int>(MouseEvent::Type::DblClick);
        } else {
          last_click_which = button_which;
          last_click_time = time;
          type |= static_cast<int>(MouseEvent::Type::Click);
        }
      }
    }

    if (delegate_ && (!has_full_move_resolution &&
                      last_event.type == MouseEvent::Type::Move &&
                      type != static_cast<int>(MouseEvent::Type::Move)))
      delegate_->EmitMouseEvent(last_event);

    last_event.button = button_number;
    last_event.buttons = static_cast<MouseEvent::ButtonMask>(buttons);
    last_event.movement_x = movement_x;
    last_event.movement_y = movement_y;
    last_event.screen_x = point_x;
    last_event.screen_y = point_y;
    last_event.which = button_which;
    last_event.type = static_cast<MouseEvent::Type>(type);

    if (delegate_ && (has_full_move_resolution ||
                      last_event.type != MouseEvent::Type::Move ||
                      message_queue.size() == 0))
      delegate_->EmitMouseEvent(last_event);
  }

  uv_mutex_unlock(&event_lock);
}

// static
void Mouse::RunThread(void* arg) {
  if (arg)
    ((Mouse*)arg)->ProcessMessages();
}

// static
void Mouse::OnSend(uv_async_t* async) {
  if (async && async->data)
    ((Mouse*)async->data)->HandleSend();
}

// static
void Mouse::OnClose(uv_handle_t* handle) {
  uv_async_t* async = (uv_async_t*)handle;
  delete async;
}

}  // namespace atom
