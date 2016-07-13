#include <OPELFaceDetection.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>

#include <iostream>
#include <v8.h>
#include <node.h>
#include <uv.h> 
#include <nan.h>

using namespace std;
using namespace cv;

string face_cascade = "/home/pi/OPEL-CameraFramework/npm/facedetection/haarcascade_frontalface_alt.xml";

class OPELFaceDetection : public Nan::AsyncWorker
{
	public:
		OPELFaceDetection(Nan::Callback *callback, int counts) : Nan::AsyncWorker(callback), counts(counts), numOfFace(0) {}
		~OPELFaceDetection() {}
		
		void Execute() 
		{
			numOfFace = 0;
			VideoCapture cap(-1);
			while(counts--)
			{
				if(!face.load(face_cascade))
				{
					fprintf(stderr, "cascade open failed\n");
					break;
				}
				Mat frame;
				Mat gray;
				bool bSuccess = cap.read(frame);
				if(!bSuccess)
				{
					fprintf(stderr, "cannot read frame\n");
					break;
				}
				cvtColor(frame, gray, CV_RGB2GRAY);
				vector<Rect> face_pos; 
				face.detectMultiScale(gray, face_pos, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
				numOfFace = (int)face_pos.size();
				if(numOfFace != 0)
					break;
			}
		}
		
		void HandleOKCallback(){
			Nan::HandleScope scope;
			v8::Local<v8::Value> argv[] = { Nan::Null(), Nan::New<v8::Number>(numOfFace)};
			callback->Call(2, argv);
		}
	
	private:
			int counts;
			int numOfFace;
			CascadeClassifier face;
};

NAN_METHOD(FaceDetection){

	int counts = Nan::To<int>(info[0]).FromJust();
	Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
	OPELFaceDetection* fd = new OPELFaceDetection(callback, counts);
	Nan::AsyncQueueWorker(fd);
}

