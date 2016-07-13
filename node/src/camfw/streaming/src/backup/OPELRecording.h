#ifndef _OPEL_RECORDING_H_
#define _OPEL_RECORDING_H_

#include <node.h>
#include <uv.h>
#include <nan.h>
#include <dbus/dbus.h>
#define REC_SHM_KEY 9447
#define ERR_INDEX 0
#define DATA_INDEX 1

const char* dev_name = "/dev/video0";


class OPELRecording : public Nan::ObjectWrap{

	public:
		static NAN_MODULE_INIT(Init);

		static NAN_METHOD(init);
		static NAN_METHOD(start);
		static NAN_METHOD(stop);
		static NAN_METHOD(close);
  	void sendDbusMsg(const char* msg); 
		bool initDbus();
		//void init(const Nan::FunctionCallbackInfo<v8::value>& info);
		bool openDevice();
		bool initSharedMemorySpace();
	private:
		explicit OPELRecording();
		~OPELRecording();
	  
		static NAN_METHOD(New);
		static Nan::Persistent<v8::Function> constructor;
		
		DBusConnection* conn;
		DBusError err;
		void* shmPtr;		
		int shmid;
		int fd;

};


#endif /* _OPEL_RECORDING_H_ */

