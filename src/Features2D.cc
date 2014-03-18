#include "Features2D.h"
#include "OpenCV.h"
#include "Matrix.h"
#include "KeyPoint.h"

Persistent<FunctionTemplate> SURF::constructor;

void
SURF::Init(Handle<Object> target) {
  HandleScope scope;

  // Constructor
  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(SURF::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("SURF"));

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(constructor, "detect", Detect);

  target->Set(String::NewSymbol("SURF"), constructor->GetFunction());
};

Handle<Value>
SURF::New(const Arguments &args) {
  HandleScope scope;

  SURF *surf;
  surf = new SURF();

  surf->Wrap(args.Holder());
  return scope.Close(args.Holder());
}

SURF::SURF(): ObjectWrap() {
  surf = cv::SURF::SURF();
}

SURF::SURF(double hessianThreshold): ObjectWrap() {
  surf = cv::SURF::SURF(hessianThreshold);
}

SURF::SURF(double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright): ObjectWrap() {
  surf = cv::SURF::SURF(hessianThreshold, nOctaves, nOctaveLayers, extended, upright);
}

// Detects keypoints in an image
// Usage: var keypoints = detector.detect(mat);
Handle<Value>
SURF::Detect(const Arguments &args) {
  HandleScope scope;

  SURF *self = ObjectWrap::Unwrap<SURF>(args.This());
  Matrix *mat = ObjectWrap::Unwrap<Matrix>(args[0]);
  std::vector<cv::KeyPoint> keypoints;

  self->surf.detect(mat->mat, keypoints);

  unsigned int size = keypoints.size();
  Local<Array> arrKeypoints = Array::New(size);
  for (unsigned int i = 0; i < size; i++) {
    Local<Object> kpObject = KeyPoint::constructor->GetFunction()->NewInstance(); // New v8 Keypoint
    KeyPoint *kp = ObjectWrap::Unwrap<KeyPoint>(kpObject);  // ObjectWrap KeyPoint *
    kp->keypoint = keypoints[i];
    arrKeypoints->Set(i, kpObject);
  }

  return scope.Close(arrKeypoints);
}
