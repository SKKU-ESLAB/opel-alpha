#include <OPELFaceDetection.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>
#include <v8.h>
#include <node.h>
#include <uv.h> 
#include <nan.h>
#include <string>
//using namespace std;
//using namespace cv;
const char* path = "/home/pi/faceRecognition/";
std::string face_cascade = "/home/pi/OPEL-CameraFramework/npm/facedetection/haarcascade_frontalface_alt.xml";

class OPELFaceDetection : public Nan::AsyncWorker
{
	public:
		OPELFaceDetection(Nan::Callback *callback, int counts) : Nan::AsyncWorker(callback), counts(counts), numOfFace(0) {}
		~OPELFaceDetection() {}
		
		void Execute() 
		{
			char timeBuf[100];
			imgPath = (char*)malloc(sizeof(char)*100);
			numOfFace = 0;
			cv::VideoCapture cap(-1);
			memset(imgPath, 0x0, 100);
			while(counts--)
			{
				if(!face.load(face_cascade))
				{
					fprintf(stderr, "cascade open failed\n");
					break;
				}
				cv::Mat frame;
				cv::Mat gray;
				bool bSuccess = cap.read(frame);
				if(!bSuccess)
				{
					fprintf(stderr, "cannot read frame\n");
					break;
				}
				cv::cvtColor(frame, gray, CV_RGB2GRAY);
				std::vector<cv::Rect> face_pos; 
				face.detectMultiScale(gray, face_pos, 1.1, 3, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(10, 10));
				numOfFace = (int)face_pos.size();
				if(numOfFace != 0){
					gettimeofday(&(this->curTime), NULL);
					sprintf(timeBuf, "%d", curTime.tv_sec);
					strcpy(imgPath, path); 
					strcat(imgPath, timeBuf);	
					strcat(imgPath, ".jpg");
					fprintf(stderr, "Image Path : %s\n", imgPath);
					imwrite(imgPath, gray);
					//send over selective connection to grayscale Image Data
						
					break;
				}
			}
		}
		
		void HandleOKCallback(){	
			
			Nan::HandleScope scope;
			fprintf(stderr, "imgPath : %s\n", imgPath);
			v8::Local<v8::Value> argv[] = {Nan::Null(), Nan::New<v8::Number>(numOfFace), Nan::New<v8::String>(imgPath).ToLocalChecked()};
			callback->Call(3, argv);
		}
	
	private:
			int counts;
			int numOfFace;
			cv::CascadeClassifier face;
			struct timeval curTime;
			char* imgPath;
};

NAN_METHOD(FaceRecognition){

	int counts = Nan::To<int>(info[0]).FromJust();
	Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
	OPELFaceDetection* fd = new OPELFaceDetection(callback, counts);
	Nan::AsyncQueueWorker(fd);
}

