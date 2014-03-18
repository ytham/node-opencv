#include "KeyPoint.h"
#include "OpenCV.h"

Persistent<FunctionTemplate> KeyPoint::constructor;

void
KeyPoint::Init(Handle<Object> target) {
  HandleScope scope;

  // Constructor
  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(KeyPoint::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("KeyPoint"));

  // Prototype
  Local<ObjectTemplate> proto = constructor->PrototypeTemplate();

  target->Set(String::NewSymbol("KeyPoint"), constructor->GetFunction());
}

Handle<Value>
KeyPoint::New(const Arguments &args) {
  HandleScope scope;

  if (args.This()->InternalFieldCount() == 0) {
    return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot instantiate without new")));
  }

  KeyPoint *kp;
  cv::Point2f _pt;
  float x, y, _size, _angle;
  int _response, _octave, _class_id;

  if (args[0]->IsNull()) {
    cout << "KeyPoint created with no arguments" << endl;
    kp = new KeyPoint();
  } else if (args[0]->IsNumber()) {
    cout << "KeyPoint created with float x,y" << endl;
    if (args[0]->IsNumber()) x = args[0]->NumberValue();
    if (args[1]->IsNumber()) y = args[1]->NumberValue();
    if (args[2]->IsNumber()) _size = args[2]->NumberValue();
    if (args[3]->IsNumber()) _angle = args[3]->NumberValue();
    if (args[4]->IsNumber()) _response = args[4]->NumberValue();
    if (args[5]->IsNumber()) _octave = args[5]->NumberValue();
    if (args[6]->IsNumber()) _class_id = args[6]->NumberValue();
    kp = new KeyPoint()
  }


  kp->Wrap(args.Holder());
  return scope.Close(args.Holder());
}

KeyPoint::KeyPoint(): ObjectWrap() {
  keypoint = cv::KeyPoint();
}

KeyPoint::KeyPoint(cv::Point2f _pt, float _size, float _angle, float _response, int _octave, int _class_id): ObjectWrap() {
  keypoint = cv::KeyPoint(_pt, _size, _angle, _response, _octave, _class_id);
}

KeyPoint::KeyPoint(float x, float y, float _size, float _angle, float _response, int _octave, int _class_id): ObjectWrap() {
  keypoint = cv::KeyPoint(x, y, _size, _angle, _response, _octave, _class_id);
}

