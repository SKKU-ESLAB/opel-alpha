#ifndef _OPEL_RECORDING_H_
#define _OPEL_RECORDING_H_
extern "C"{
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <assert.h>
#include <dbus/dbus.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <poll.h>
}
#include <v8.h>
#include <node.h>
#include <uv.h>
#include <nan.h>
#include "OPELdbusInterface.h"

class OPELRecording : public Nan::ObjectWrap{

	public:
		static NAN_MODULE_INIT(Init);
		void sendDbusMsg(const char* msg, dbusRequest *dbus_request);
		bool initDbus();

		DBusConnection* conn;
		DBusError err;
	private:
		explicit OPELRecording() {};
		~OPELRecording(){};

		static NAN_METHOD(New);

		static NAN_METHOD(recStart);
		static NAN_METHOD(recStop);
		
		static NAN_METHOD(jpegStart);
	  
	

		static inline Nan::Persistent<v8::Function>& constructor()
		{
			static Nan::Persistent<v8::Function> my_constructor;
			return my_constructor;
		}
};
//NODE_MODULE(OPELRecording, OPELRecording::Init)

#endif /* _OPEL_RECORDING_H_ */

