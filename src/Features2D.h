#include "OpenCV.h"

#include "opencv2/nonfree/features2d.hpp"

using namespace v8;

class SURF: public node::ObjectWrap {
  public:
    cv::SURF surf;

    static Persistent<FunctionTemplate> constructor;
    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);

    static Handle<Value> Detect(const Arguments &args);
};