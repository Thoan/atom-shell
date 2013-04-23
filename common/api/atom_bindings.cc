// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/api/atom_bindings.h"

#include "base/logging.h"
#include "vendor/node/src/node.h"

namespace atom {

// Defined in atom_extensions.cc.
node::node_module_struct* GetBuiltinModule(const char *name, bool is_browser);

AtomBindings::AtomBindings() {
}

AtomBindings::~AtomBindings() {
}

void AtomBindings::BindTo(v8::Handle<v8::Object> process) {
  v8::HandleScope scope;

  node::SetMethod(process, "atom_binding", Binding);
}

// static
v8::Handle<v8::Value> AtomBindings::Binding(const v8::Arguments& args) {
  v8::HandleScope scope;

  v8::Local<v8::String> module = args[0]->ToString();
  v8::String::Utf8Value module_v(module);
  node::node_module_struct* modp;

  v8::Local<v8::Object> process = v8::Context::GetCurrent()->Global()->
      Get(v8::String::New("process"))->ToObject();
  DCHECK(!process.IsEmpty());

  // is_browser = process.__atom_type == 'browser'.
  bool is_browser = std::string("browser") == *v8::String::Utf8Value(
      process->Get(v8::String::New("__atom_type")));

  // Cached in process.__atom_binding_cache.
  v8::Local<v8::Object> binding_cache;
  v8::Local<v8::String> bc_name = v8::String::New("__atom_binding_cache");
  if (process->Has(bc_name)) {
    binding_cache = process->Get(bc_name)->ToObject();
    DCHECK(!binding_cache.IsEmpty());
  } else {
    binding_cache = v8::Object::New();
    process->Set(bc_name, binding_cache);
  }

  v8::Local<v8::Object> exports;

  if (binding_cache->Has(module)) {
    exports = binding_cache->Get(module)->ToObject();
    return scope.Close(exports);
  }

  if ((modp = GetBuiltinModule(*module_v, is_browser)) != NULL) {
    exports = v8::Object::New();
    // Internal bindings don't have a "module" object,
    // only exports.
    modp->register_func(exports, v8::Undefined());
    binding_cache->Set(module, exports);
    return scope.Close(exports);
  }

  return v8::ThrowException(v8::Exception::Error(
      v8::String::New("No such module")));
}

}  // namespace atom