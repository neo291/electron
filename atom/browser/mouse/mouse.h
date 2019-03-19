// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_MOUSE_MOUSE_H_
#define ATOM_BROWSER_MOUSE_MOUSE_H_

#include "base/memory/weak_ptr.h"
#include "uv.h"  // NOLINT(build/include)

#include <queue>

namespace atom {

class MouseDelegate;

struct MouseEvent {
  enum class Button { None = 0, Left = 0, Middle = 1, Right = 2 };

  enum class ButtonMask {
    None = 0x00,
    Left = 0x01,
    Middle = 0x04,
    Right = 0x02
  };

  enum class Which { None = 0, Left = 1, Middle = 2, Right = 3 };

  enum class Type {
    None = 0x00,
    Click = 0x01,
    DblClick = 0x02,
    Down = 0x04,
    Move = 0x08,
    Up = 0x10
  };

  Button button;
  ButtonMask buttons;
  int movement_x;
  int movement_y;
  int screen_x;
  int screen_y;
  Which which;
  Type type;
};

class Mouse {
 public:
  static Mouse* GetMouse();
  static Mouse* Create();

  virtual ~Mouse();

  void set_delegate(MouseDelegate* delegate) { delegate_ = delegate; }

  bool HasFullMoveResolution();
  void SetFullMoveResolution(bool value);

 protected:
  enum class MessageType {
    None,
    LButtonDown,
    LButtonUp,
    MButtonDown,
    MButtonUp,
    RButtonDown,
    RButtonUp,
    Move
  };

 protected:
  Mouse();

  bool Init(long initial_x, long initial_y);
  void AddMessageToQueue(MessageType type, long x, long y, unsigned long time);

  virtual void ProcessMessages() = 0;

  uv_cond_t init_cond;
  uv_mutex_t init_lock;
  uv_mutex_t event_lock;

 private:
  struct Message {
    MessageType type;
    long x;
    long y;
    unsigned long time;
  };

 private:
  void HandleSend();

  static void RunThread(void* arg);
  static void OnSend(uv_async_t* async);
  static void OnClose(uv_handle_t* handle);

  static Mouse* instance_;

  MouseDelegate* delegate_;

  bool has_full_move_resolution;

  uv_async_t* async;
  uv_thread_t thread;

  MouseEvent::Which last_click_which;
  UINT last_click_time;
  MouseEvent last_event;
  std::queue<Message> message_queue;

  DISALLOW_COPY_AND_ASSIGN(Mouse);
};

}  // namespace atom

#endif  // ATOM_BROWSER_MOUSE_MOUSE_H_
