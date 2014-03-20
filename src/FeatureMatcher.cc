#include "FeatureMatcher.h"
#include "OpenCV.h"
#include "Matrix.h"

v8::Persistent<FunctionTemplate> FeatureMatcher::constructor;

void
FeatureMatcher::Init(Handle<Object> target) {
  HandleScope scope;
  
  // Constructor
  constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FeatureMatcher::New));
  constructor->InstanceTemplate()->SetInternalFieldCount(1);
  constructor->SetClassName(String::NewSymbol("FeatureMatcher"));

  // Prototype
  //Local<ObjectTemplate> proto = constructor->PrototypeTemplate();
  NODE_SET_PROTOTYPE_METHOD(constructor, "trainSVM", TrainSVM);
  NODE_SET_PROTOTYPE_METHOD(constructor, "classify", Classify);
  target->Set(String::NewSymbol("FeatureMatcher"), constructor->GetFunction());
};

Handle<Value>
FeatureMatcher::New(const Arguments &args) {
  HandleScope scope;

  String::AsciiValue args0(args[0]->ToString());
  std::string vocab = *args0;
  String::AsciiValue args1(args[1]->ToString());
  std::string classifiers = std::string(*args1);  

  std::vector<std::string> v_folders;
  Local<Array> folders = Local<Array>::Cast(args[2]);
  int flength = folders->Length();
  for (int i = 0; i < flength; i++) {
    String::Utf8Value u_folder(folders->Get(i)->ToString());
    std::string folder = std::string(*u_folder);
    v_folders.push_back(folder);
  }

  FeatureMatcher *fm = new FeatureMatcher(vocab, classifiers, v_folders);

  fm->Wrap(args.This());

  return args.This();
}

FeatureMatcher::FeatureMatcher(const std::string vocab, const std::string classifiers, const std::vector<std::string> inputfolders) {
  vocabularyFile = vocab;
  classifiersPath = classifiers;
  folders = inputfolders;

  detector = cv::SiftFeatureDetector(500);
  matcher = cv::DescriptorMatcher::create("FlannBased");
  extractor = new cv::SiftDescriptorExtractor();
}

// Input: Array of file path strings, vocabulary file path string, classifier folder string
// Output: Vocabulary Matrix
Handle<Value>
FeatureMatcher::TrainSVM(const Arguments &args) {
  HandleScope scope;

  FeatureMatcher *self = ObjectWrap::Unwrap<FeatureMatcher>(args.This());

  // Define variables to be used
  cv::Mat img, descriptors, training_descriptors;
  std::vector<cv::KeyPoint> keypoints;

  // Turn JS array into C++ array
  // Local<Array> folders = Local<Array>::Cast(args[0]);
  // int flength = folders->Length();

  // Second argument is object of files
  Local<Object> files = args[0]->ToObject();

  for (std::vector<std::string>::iterator it = self->folders.begin(); it != self->folders.end(); ++it) {
    // Get the string of foldernames
    std::string folder = (*it);

    // Get the array 
    Local<Array> thisFolder = Local<Array>::Cast(files->Get(String::New(folder.c_str())));
    int alength = thisFolder->Length();

    for (int j = 0; j < alength; j++) {
      String::Utf8Value u_filePath(thisFolder->Get(j)->ToString());
      std::string filePath = std::string(*u_filePath);
      std::cout << "Input: [" << folder << "] " << filePath << std::endl;

      // Convert to Mat
      img = cv::imread(filePath);
      self->detector.detect(img, keypoints);
      self->extractor->compute(img, keypoints, descriptors);

      training_descriptors.push_back(descriptors);
    }
  }
  std::cout << "Total Descriptors: " << training_descriptors.rows << std::endl;

  // Use BOW for K-means clustering
  cv::TermCriteria bowtc = cv::TermCriteria(3, 10, 0.001);
  cv::BOWKMeansTrainer bow(100, bowtc, 1, cv::KMEANS_PP_CENTERS);
  bow.add(training_descriptors);
  cv::Mat vocabulary = bow.cluster();
  
  // Write the vocabulary to file
  std::cout << "Writing Vocabulary: " << self->vocabularyFile << std::endl;
  //self->vocabularyFile = "/Users/ytham/cell-script-testbox/vision/svm/vocabulary/vocabulary.yml";
  self->filestore.open(std::string(self->vocabularyFile), cv::FileStorage::WRITE);
  self->filestore << "Vocabulary" << vocabulary;
  self->filestore.release();
  // Local<Object> o_vocabulary = Matrix::constructor->GetFunction()->NewInstance();
  // Matrix *js_vocabulary = ObjectWrap::Unwrap<Matrix>(o_vocabulary);
  // js_vocabulary->mat = vocabulary;

  // Get Bag-Of-Words extractor
  cv::BOWImgDescriptorExtractor bowExtractor(self->extractor, self->matcher);
  bowExtractor.setVocabulary(vocabulary);

  // Build training data for classifiers
  cv::Mat response_histogram;
  std::map<std::string,cv::Mat> classes_training;
  classes_training.clear();

  for (std::vector<std::string>::iterator it = self->folders.begin(); it != self->folders.end(); ++it) {
    // Get the string of foldernames
    std::string folder = (*it);

    // Get the array
    Local<Array> thisFolder = Local<Array>::Cast(files->Get(String::New(folder.c_str())));
    int alength = thisFolder->Length();

    for (int j = 0; j < alength; j++) {
      // Get the file path
      String::Utf8Value u_filePath(thisFolder->Get(j)->ToString());
      std::string filePath = std::string(*u_filePath);

      // Compute response histogram from image
      img = cv::imread(filePath);
      self->detector.detect(img, keypoints);
      bowExtractor.compute(img, keypoints, response_histogram);

      // Push the response_histogram onto the classes_training map, key is folder name
      if (classes_training.count(folder) == 0) {
        classes_training[folder].create(0, response_histogram.cols, response_histogram.type());
      }
      classes_training[folder].push_back(response_histogram);
    }
  }

  // Train SVM 1-vs-all
  std::map<std::string,CvSVM::CvSVM> classes_classifiers;
  for (std::map<std::string,cv::Mat>::iterator it = classes_training.begin(); it != classes_training.end(); ++it) {
    std::string thisClass = (*it).first;
    std::cout << "Training SVM for class " << thisClass << std::endl;

    // Create Mat that will eventually hold all of the samples and labels
    cv::Mat samples(0, response_histogram.cols, response_histogram.type());
    cv::Mat labels(0, 1, CV_32FC1);

    // Push back the current training 1D Mat w/ a ones class label;
    samples.push_back(classes_training[thisClass]);
    cv::Mat class_label = cv::Mat::ones(classes_training[thisClass].rows, 1, CV_32FC1);
    labels.push_back(class_label);

    // Push back all other sets with a zeroes class label;
    for (std::map<std::string,cv::Mat>::iterator jt = classes_training.begin(); jt != classes_training.end(); ++jt) {
      std::string negativeClass = (*jt).first;
      if (thisClass == negativeClass) continue;

      samples.push_back(classes_training[negativeClass]);
      cv::Mat negative_label = cv::Mat::zeros(classes_training[negativeClass].rows, 1, CV_32FC1);
      labels.push_back(negative_label);
    }

    // Convert samples to CV_32F
    cv::Mat samples_32f;
    samples.convertTo(samples_32f, CV_32F);

    // Set the termination criteria and params
    cv::TermCriteria svmtc = cv::TermCriteria(2, 1000, 0.00001);  // CV_TERMCRIT_EPS = 2
    CvSVMParams::CvSVMParams params = CvSVMParams::CvSVMParams(CvSVM::C_SVC, CvSVM::RBF, 10.0, 8.0, 1.0, 10.0, 0.5, 0.1, NULL, svmtc);

    // Train and save the classifier
    std::string saveClassifier = self->classifiersPath + thisClass + ".xml";
    classes_classifiers[thisClass].train(samples_32f, labels, cv::Mat(), cv::Mat(), params);
    classes_classifiers[thisClass].save(saveClassifier.c_str());
  }

  return scope.Close(v8::Null());
}

// Input: A JS Matrix object
// Output: classification string
Handle<Value>
FeatureMatcher::Classify(const Arguments &args) {
  HandleScope scope;

  FeatureMatcher *self = ObjectWrap::Unwrap<FeatureMatcher>(args.This());

  // Return string
  // Local<String> return_string = String->constructor->GetFunction()->NewInstance();
  // String *o_return_string = ObjectWrap::Unwrap<String>(return_string);

  // Define variables to be used
  std::vector<cv::KeyPoint> keypoints;
  cv::Mat response_histogram;
  std::map<std::string,CvSVM::CvSVM> classes_classifiers;

  // Load the Vocabulary
  cv::Mat vocabulary;
  self->filestore.open(self->vocabularyFile, cv::FileStorage::READ);
  self->filestore["Vocabulary"] >> vocabulary;
  self->filestore.release();

  // Load the classifiers
  for (std::vector<std::string>::iterator it = self->folders.begin(); it != self->folders.end(); ++it) {
    // Get the string of foldernames
    std::string folder = (*it);
    std::string classifierFile = self->classifiersPath + folder + ".xml";
    classes_classifiers[folder].load(classifierFile.c_str());
  }

  // Set up the bag of words descriptor extractor
  cv::BOWImgDescriptorExtractor bowExtractor(self->extractor, self->matcher);
  bowExtractor.setVocabulary(vocabulary);

  // Get keypoints and response_histogram for the input Mat
  Matrix *img = ObjectWrap::Unwrap<Matrix>(args[0]->ToObject());
  self->detector.detect(img->mat, keypoints);
  bowExtractor.compute(img->mat, keypoints, response_histogram);

  if (response_histogram.rows == 0) {
    return scope.Close(String::New("Invalid matrix"));
  }

  float bestMatchValue = 1.0;
  std::string bestMatch;

  // Run all classifiers over the image and take the highest score
  for (std::map<std::string,CvSVM::CvSVM>::iterator it = classes_classifiers.begin(); it != classes_classifiers.end(); ++it) {
    float prediction = (*it).second.predict(response_histogram, true);
    std::string thisClass = (*it).first;
    std::cout << "class: " << thisClass << "\tprediction: " << prediction << std::endl;
    if (prediction < bestMatchValue) {
      bestMatchValue = prediction;
      bestMatch = thisClass;
    }
  }

  if (bestMatchValue > -0.2) {
    bestMatch = "No best match";
  }

  return scope.Close(String::New(bestMatch.c_str()));
}