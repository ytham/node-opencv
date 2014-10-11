#include "BoundaryFinder.h"
#include "OpenCV.h"

v8::Persistent<FunctionTemplate> BoundaryFinder::constructor;

void
BoundaryFinder::Init(Handle<Object> target) {
  HandleScope scope;

  // Constructor
  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(BoundaryFinder::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("BoundaryFinder"));

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(constructor, "findBoundary", FindBoundary);
  target->Set(String::NewSymbol("BoundaryFinder"), constructor->GetFunction());
};

Handle<Value>
BoundaryFinder::New(const Arguments &args) {
  HandleScope scope;

  int camNumber = args[0]->IntegerValue();
  int lowerBound = args[1]->IntegerValue();
  int upperBound = args[2]->IntegerValue();

  BoundaryFinder *bf = new BoundaryFinder(camNumber, lowerBound, upperBound);

  bf->Wrap(args.This());

  return args.This();
}

BoundaryFinder::BoundaryFinder(int camNumber, int lowerBound, int upperBound) {
  camNumber = camNumber;
  lowerBound = lowerBound;
  upperBound = upperBound;
}

/**
 * Finds the screen boundary of a device that is between the lower and upper bounds
 *
 * Input: None
 * Output: 4 points
 */
Handle<Value>
BoundaryFinder::FindBoundary() {
  // Open the camera
  camera.open(camNumber);
  if (!camera.isOpened()) {
    std::cout << "Unable to access camera" << std::endl;
    exit(1);
  }

  cv::Mat cameraFrame;
  camera >> cameraFrame;

  cv::Mat compositeImage = cv::Mat::zeros(cameraFrame.rows, cameraFrame.cols, CV_8UC1);

  for (int i = 0; i < 100; i++) {
        
    camera >> cameraFrame;
    
    cv::Mat cameraGray;
    cvtColor(cameraFrame, cameraGray, CV_BGR2GRAY);
    
    cv::Mat corners;
    cornerHarris(cameraGray, corners, 2, 3, 0.06, BORDER_DEFAULT);
    
    cv::Mat corners_norm;
    normalize(corners, corners_norm, 0, 255, NORM_MINMAX, CV_8UC1, Mat());
    
    cv::Mat thresh;
    adaptiveThreshold(corners_norm, thresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 5, 1);
    
    bitwise_or(compositeImage, thresh, compositeImage);
  }

  std::vector<vector<Point> > contours;
  std::vector<Vec4i> hierarchy;
  findContours(compositeImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0,0) );
  cv::Mat contourDrawing = Mat::zeros(compositeImage.size(), CV_8UC1);
  
  
  std::vector<vector<Point> > contours_poly(contours.size());
  for (int i = 0; i < contours.size(); i++) {
    approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
  }
  
  for (int i = 0; i < contours_poly.size(); i++) {
    int contourArea = contourArea(contours_poly[i]);
    if (contourArea > lowerBound && contourArea < upperBound) {   // device screen size
      drawContours(contourDrawing, contours_poly, i, Scalar(255,0,0), 2, 8, hierarchy, 0, Point());
    }
  }

  return scope.Close();
}