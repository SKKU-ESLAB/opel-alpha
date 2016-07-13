#include "OPELRecording.h"

// addon.init(function(err, data){width, height, buffer size});
NAN_METHOD(OPELRecording::recInit)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	if(!info[0]->IsFunction())
	{
	 Nan::ThrowError("Invalid Arguments");
	 return;
	}
	v8::Local<v8::Function> cb = info[0].As<v8::Function>();
	const unsigned argc = 3;
	v8::Local<v8::Value> argv[argc] = { Nan::New(recObj->getWidth()), Nan::New(recObj->getHeight()), Nan::New(recObj->getBufferSize()) };
	Nan::Callback callback(cb);
	
	if(!(recObj->openDevice()))
	{
		//error
		Nan::ThrowError("Open FileDesc /dev/video0 Failed\n");
		return;
	}
	if(!(recObj->initSharedMemorySpace()))
	{
		//error
		Nan::ThrowError("Initialize Shared Memory Space Failed\n");
		return;
	}
//	recObj->sendDbusMsg("recInit");
  
	//throw callback
	callback.Call(argc, argv);		
	//check property is changed 

}
// Recording Start API (Asynchronous)
// addon.start("video_path", count, function(err, data){   });
NAN_METHOD(OPELRecording::recStart)
{	
	int count;
	const char* file_path;
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("Put Recording File Path");
		return;
	}
	if(!info[1]->IsNumber())
	{
		Nan::ThrowTypeError("Put Recording Count");
		return;
	}
//std::string file_path = Nan::To<std::string>(info[0]).FromJust();
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();
	
	count = Nan::To<int>(info[1]).FromJust();
	
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());


	recObj->sendDbusMsg("recStart");
  Nan::Callback *callback = new Nan::Callback();
	RecordingWorker* recWorker = new RecordingWorker(callback, file_path, count);	
	
	recWorker->setFd(recObj->getFd());
	recWorker->setWidth(recObj->getWidth());
	recWorker->setHeight(recObj->getHeight());
	recWorker->setBufferSize(recObj->getBufferSize());
	recWorker->setBufferIndex(recObj->getBufferIndex());
	recWorker->setShmPtr(recObj->getShmPtr());

	Nan::AsyncQueueWorker(recWorker);
	
}
NAN_METHOD(OPELRecording::recStop)
{


}
NAN_METHOD(OPELRecording::recClose)
{

}

extern "C"{
OPELRecording::OPELRecording()
{
	this->width = REC_WIDTH;
	this->height = REC_HEIGHT;
	this->buffer_size = REC_BUFFER_SIZE;
	this->buffer_index = REC_BUFFER_INDEX;
}
OPELRecording::~OPELRecording()
{

}
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
