// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_API_ATOM_API_MOUSE_H_
#define ATOM_BROWSER_API_ATOM_API_MOUSE_H_

#include "atom/browser/api/event_emitter.h"
#include "atom/browser/mouse/mouse.h"
#include "atom/browser/mouse/mouse_delegate.h"
#include "native_mate/handle.h"

namespace atom {

namespace api {

class Mouse : public mate::EventEmitter<Mouse>, public MouseDelegate {
 public:
  static mate::Handle<Mouse> Create(v8::Isolate* isolate);

  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype);

  // MouseDelegate:
  void EmitMouseEvent(atom::MouseEvent mouseEvent) override;

 protected:
  Mouse(v8::Isolate* isolate);
  ~Mouse() override;

  bool IsSupported();

  bool HasFullMoveResolution();
  void SetFullMoveResolution(bool value);

 private:
  atom::Mouse* mouse_;

  DISALLOW_COPY_AND_ASSIGN(Mouse);
};

}  // namespace api

}  // namespace atom

#endif  // ATOM_BROWSER_API_ATOM_API_MOUSE_H_
