// Template class for KeyPoints

#include "OpenCV.h"

class KeyPoint: public node::ObjectWrap {
  public:
    cv::KeyPoint keypoint;

    static Persistent<FunctionTemplate> constructor;
    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);
    
    KeyPoint();   
    KeyPoint(cv::Point2f _pt, float _size, float _angle, float _response, int _octave, int _class_id);
    KeyPoint(float x, float y, float _size, float _angle, float _response, int _octave, int _class_id);
    

};