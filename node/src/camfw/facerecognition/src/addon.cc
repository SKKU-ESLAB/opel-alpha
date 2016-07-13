#include "OPELFaceDetection.h"
#include <nan.h>

NAN_METHOD(FaceDetection);

NAN_MODULE_INIT(InitAll) {
	Nan::Set(target, Nan::New<v8::String>("faceRecognition").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(FaceRecognition)).ToLocalChecked());
}

NODE_MODULE(OPELFaceRecognition, InitAll)
