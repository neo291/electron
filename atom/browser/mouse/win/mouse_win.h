// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_MOUSE_WIN_MOUSE_WIN_H_
#define ATOM_BROWSER_MOUSE_WIN_MOUSE_WIN_H_

#include "atom/browser/mouse/mouse.h"

#include <windows.h>

namespace atom {

class MouseWin : public Mouse {
 public:
  MouseWin();
  ~MouseWin() override;

  bool Init();

 protected:
  void ProcessMessages() override;

 private:
  static MessageType WinToMouseMessageType(WPARAM wParam);
  static LRESULT CALLBACK LowLevelMouseProc(int nCode,
                                            WPARAM wParam,
                                            LPARAM lParam);

  void HandleMessage(WPARAM wParam, LPARAM lParam);

  DWORD thread_id;

  DISALLOW_COPY_AND_ASSIGN(MouseWin);
};

}  // namespace atom

#endif  // ATOM_BROWSER_MOUSE_WIN_MOUSE_WIN_H_
