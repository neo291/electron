// Copyright (c) 2018 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/api/atom_api_mouse.h"

#include "native_mate/dictionary.h"

#include "atom/common/node_includes.h"

namespace mate {
template <>
struct Converter<atom::MouseEvent> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate, atom::MouseEvent val) {
    mate::Dictionary dict = mate::Dictionary::CreateEmpty(isolate);
    dict.Set("button", static_cast<int>(val.button));
    dict.Set("buttons", static_cast<int>(val.buttons));
    dict.Set("movementX", val.movement_x);
    dict.Set("movementY", val.movement_y);
    dict.Set("screenX", val.screen_x);
    dict.Set("screenY", val.screen_y);
    dict.Set("which", static_cast<int>(val.which));
    return dict.GetHandle();
  }
};
}  // namespace mate

namespace atom {

namespace api {

Mouse::Mouse(v8::Isolate* isolate) {
  mouse_ = atom::Mouse::GetMouse();

  if (mouse_)
    mouse_->set_delegate(this);

  Init(isolate);
}

Mouse::~Mouse() {
  if (mouse_)
    mouse_->set_delegate(nullptr);
}

void Mouse::EmitMouseEvent(atom::MouseEvent mouse_event) {
  int type = static_cast<int>(mouse_event.type);

  if (type & static_cast<int>(MouseEvent::Type::Click))
    Emit("click", mouse_event);

  if (type & static_cast<int>(MouseEvent::Type::DblClick))
    Emit("dblclick", mouse_event);

  if (type & static_cast<int>(MouseEvent::Type::Down))
    Emit("down", mouse_event);

  if (type & static_cast<int>(MouseEvent::Type::Move))
    Emit("move", mouse_event);

  if (type & static_cast<int>(MouseEvent::Type::Up))
    Emit("up", mouse_event);
}

bool Mouse::IsSupported() {
  return !!mouse_;
}

bool Mouse::HasFullMoveResolution() {
  if (mouse_)
    return mouse_->HasFullMoveResolution();

  return false;
}

void Mouse::SetFullMoveResolution(bool value) {
  if (mouse_)
    mouse_->SetFullMoveResolution(value);
}

// static
mate::Handle<Mouse> Mouse::Create(v8::Isolate* isolate) {
  return mate::CreateHandle(isolate, new Mouse(isolate));
}

// static
void Mouse::BuildPrototype(v8::Isolate* isolate,
                           v8::Local<v8::FunctionTemplate> prototype) {
  prototype->SetClassName(mate::StringToV8(isolate, "Mouse"));
  mate::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
      .SetMethod("isSupported", &Mouse::IsSupported)
      .SetMethod("hasFullMoveResolution", &Mouse::HasFullMoveResolution)
      .SetMethod("setFullMoveResolution", &Mouse::SetFullMoveResolution);
}

}  // namespace api

}  // namespace atom

namespace {

using atom::api::Mouse;

void Initialize(v8::Local<v8::Object> exports,
                v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context,
                void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  mate::Dictionary dict(isolate, exports);
  dict.Set("mouse", Mouse::Create(isolate));
  dict.Set(
      "Mouse",
      Mouse::GetConstructor(isolate)->GetFunction(context).ToLocalChecked());
}

}  // namespace

NODE_LINKED_MODULE_CONTEXT_AWARE(atom_common_mouse, Initialize)
