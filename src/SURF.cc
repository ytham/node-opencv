#include "SURF.h"
#include "OpenCV.h"

Persistent<FunctionTemplate> SURF::constructor;

void
SURF::Init(Handle<Object> target) {
  HandleScope scope;

  // Constructor
  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(SURF::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("SURF"));

  // Prototype
  Local<ObjectTemplate> proto = constructor->PrototypeTemplate();

  target->Set(String::NewSymbol("SURF"), constructor->GetFunction());
}

Handle<Value>
SURF::New(const Arguments &args) {
  HandleScope scope;

  if (args.This()->InternalFieldCount() == 0) {
    return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot instantiate without new")));
  }

  SURF *kp = new SURF();
}