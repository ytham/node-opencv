#include "OpenCV.h"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/ml/ml.hpp"

class FeatureMatcher: public node::ObjectWrap {
  public:
    std::string vocabularyFile;
    std::string classifiersPath;
    std::vector<std::string> folders;
    cv::FileStorage filestore;

    cv::SiftFeatureDetector detector;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    cv::Ptr<cv::DescriptorExtractor> extractor;

    static Persistent<FunctionTemplate> constructor;
    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments &args);

    FeatureMatcher(const std::string vocab, const std::string classifiers, const std::vector<std::string> inputfolders);
    static Handle<Value> TrainSVM(const Arguments &args);
    static Handle<Value> Classify(const Arguments &args);
};