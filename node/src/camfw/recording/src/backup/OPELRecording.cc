#include <v8.h>
#include <node.h>
#include <nan.h>
#include <node_buffer.h>
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
#include "OPELRecording.h"
#include <string>
#include <iostream>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

extern "C"{
	#include <stdio.h>
}

using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;
using Nan::GetFunction;
using Nan::New;
using Nan::Set;


NAN_MODULE_INIT(InitAll)
{
	 Set(target, New("init").ToLocalChecked(),
			     GetFunction(New<FunctionTemplate>(init)).ToLocalChecked());
	 Set(target, New("start").ToLocalChecked(),
				   GetFunction(New<FunctionTemplate>(start)).ToLocalChecked());
	 Set(target, New("stop").ToLocalChecked(),
				   GetFunction(New<FunctionTemplate>(stop)).ToLocalChecked());
	 Set(target, New("close").ToLocalChecked(),
					 GetFunction(New<FunctionTemplate>(close)).ToLocalChecked());
}
NODE_MODULE(OPELCamera, InitAll);
/*uv_mutex_t mutex;

void OPELRecording::sendDbusMsg(const char* msg)
{
	DBusMessage* message;
	message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);
}
bool OPELRecording::initDbus()
{
  dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!conn)
	{
		dbus_error_free(&err);
		return false;
	}
	return true;
}
bool OPELRecording::openDevice()
{
	this->fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if(-1 == fd)
		return false;
	return true;
}
bool OPELRecording::initSharedMemorySpace()
{
	shmid = shmget((key_t)REC_SHM_KEY, 0, 0);
	if(shmid == -1)
		return false;
	shmPtr = shmat(shmid, (void*)0, 0666|IPC_CREAT);
	if(shmPtr == (void*)-1)
		return false;

	return true;


}
NAN_METHOD(init)
{
	int err = -1;
	const unsigned argc = 2;
	v8::Local<v8::Value> argv[argc];
	if(info.Length() < 3)
	{
		Nan::ThrowTypeError("Wrong Number of Arguments");
		return;
	}
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("Is Not String Format");
		return;
	}
	v8::Local<v8::Function> cb = info[1].As<v8::FunctionTemplate>();
	if(!recObj->initDbus()){
  	argv[ERR_INDEX] = { Nan::New(err).ToLocalChecked() };
		argv[DATA_INDEX] = { Nan::New("D-Bus Init Failed").ToLocalChecked() }; 
		Nan::MakeCallback(Nan::GetCurrentContext()->global(), cb, argv, argc); //function(err, data);
		return;
	}
	if(!recObj->openDevice())
	{	
		argv[ERR_INDEX] =  { Nan::New(err).ToLocalChecked() };
		argv[DATA_INDEX] = { Nan::New("Open Device Failed").ToLocalChecked() };
		Nan::MakeCallback(Nan::GetCurrentContext()->global(), cb, argv, argc); //function(err, data);
		return;
	} 
	if(!initSharedMemorySpace())
	{
		argv[ERR_INDEX] =  { Nan::New(err).ToLocalChecked() };
		argv[DATA_INDEX] = { Nan::New("initSharedMemorySpace").ToLocalChecked() };
		Nan::MakeCallback(Nan::GetCurrentContext()->global(), cb, argv, argc); //function(err, data);
		return;
	}
	//argv[ERR_INDEX] = { Nan::New(errMsg).ToLocalChecked() };	

	sendDbusMsg("recInit");
	

	//Recording Service Initialization
	v8::Local<v8::Function> cb = info[1].As<v8::FunctionTemplate>();
	Nan::MakeCallback(Nan::GetCurrentContext()->global(), cb, argv, argc); //function(err, data);

}

Nan::Persistent<v8::Function> OPELRecording::constructor;

OPELRecording::OPELRecording()
{
	// Dbus Initialization 
	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!connection)
	{
		fprintf(stderr, "[OPELRecording:OPELRecording] : dbus connection error\n");
		dbus_error_free(&error);
	}
}

OPELRecording::~OPELRecording()
{ }

NAN_MODULE_INIT(OPELRecording::Init)
{
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("OPELRecording").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELRecording"), Nan::GetFunction(tpl).ToLocalChecked());
}

static NAN_METHOD(OPELRecording::New)
{
	Nan::NanScope();
	if(info.IsConstructCall())
	{
		OPELRecording *recObj = new OPELRecording();
		recObj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else
	{
//		v8::Local<v8::Function> cons = Nan::New(constructor);
//		info.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}*/
