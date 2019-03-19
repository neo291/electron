// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_MOUSE_MOUSE_DELEGATE_H_
#define ATOM_BROWSER_MOUSE_MOUSE_DELEGATE_H_

#include "atom/browser/mouse/mouse.h"

namespace atom {

class MouseDelegate {
 public:
  virtual void EmitMouseEvent(MouseEvent mouse_event) {}

 protected:
  MouseDelegate() = default;
  ~MouseDelegate() = default;
};

}  // namespace atom

#endif  // ATOM_BROWSER_MOUSE_MOUSE_DELEGATE_H_
