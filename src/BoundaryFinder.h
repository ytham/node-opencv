#include "OpenCV.h"

class BoundaryFinder: public node::ObjectWrap {
  public: 
    cv::VideoCapture camera;
    int camNumber;
    int lowerBound;
    int upperBound;

    static Persistent<FunctionTemplate> constructor;
    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);

    BoundaryFinder(int camNumber, int lowerBound, int upperBound);

    static Handle<Value> FindBoundary();
}