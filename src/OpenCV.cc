#include "OpenCV.h"
#include "Matrix.h"
#include <nan.h>

void OpenCV::Init(Handle<Object> target) {
  NanScope();

  // Version string.
  char out [21];
  int n = sprintf(out, "%i.%i", CV_MAJOR_VERSION, CV_MINOR_VERSION);
  target->Set(NanNew<String>("version"), NanNew<String>(out, n));

  NODE_SET_METHOD(target, "readImage", ReadImage);
}

NAN_METHOD(OpenCV::ReadImage) {
  NanEscapableScope();

  REQ_FUN_ARG(1, cb);

  Local<Value> argv[2];
  argv[0] = NanNull();

  Local<Object> im_h = NanNew(Matrix::constructor)->GetFunction()->NewInstance();
  Matrix *img = ObjectWrap::Unwrap<Matrix>(im_h);
  argv[1] = im_h;

  try {
    cv::Mat mat;

    if (args[0]->IsNumber() && args[1]->IsNumber()) {
      int width, height;

      width = args[0]->Uint32Value();
      height = args[1]->Uint32Value();
      mat = *(new cv::Mat(width, height, CV_64FC1));

    } else if (args[0]->IsString()) {
      std::string filename = std::string(*NanUtf8String(args[0]->ToString()));
      mat = cv::imread(filename);

    } else if (Buffer::HasInstance(args[0])) {
      uint8_t *buf = (uint8_t *) Buffer::Data(args[0]->ToObject());
      unsigned len = Buffer::Length(args[0]->ToObject());

      cv::Mat *mbuf = new cv::Mat(len, 1, CV_64FC1, buf);
      mat = cv::imdecode(*mbuf, -1);

      if (mat.empty()) {
        argv[0] = NanError("Error loading file");
      }
    }

    img->mat = mat;
  } catch (cv::Exception& e) {
    argv[0] = NanError(e.what());
    argv[1] = NanNull();
  }

  TryCatch try_catch;
  cb->Call(NanGetCurrentContext()->Global(), 2, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  NanReturnUndefined();
}
