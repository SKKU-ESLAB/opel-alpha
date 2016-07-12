#include "OPELStreamingAPI.h"

const char* dev_name = "/dev/video0";

// addon.init(function(err, data){width, height, buffer size});
NAN_METHOD(OPELStreaming::streamInit)
{
	OPELStreaming *streamObj = Nan::ObjectWrap::Unwrap<OPELStreaming>(info.This());
	if(!info[0]->IsFunction())
	{
	 Nan::ThrowError("Invalid Arguments");
	 return;
	}
	v8::Local<v8::Function> cb = info[0].As<v8::Function>();
	const unsigned argc = 3;
	v8::Local<v8::Value> argv[argc] = { Nan::New(streamObj->getWidth()), Nan::New(streamObj->getHeight()), Nan::New(streamObj->getBufferSize()) };
	Nan::Callback callback(cb);
	
	if(!(streamObj->initDbus()))
	{
		Nan::ThrowError("D-Bus Initialization Failed\n");
		return;
	}
	if(!(streamObj->initUVMutex()))
	{
		Nan::ThrowError("Init UV_Mutex Failed\n");
		return; 
	}
	if(!(streamObj->openDevice()))
	{
		//error
		Nan::ThrowError("Open FileDesc /dev/video0 Failed\n");
		return;
	}
	streamObj->sendDbusMsg("recInit");
	//throw callback
	callback.Call(argc, argv);		
	//check property is changed 
}
// Recording Start API (Asynchronous)
// addon.start("video_path", count, function(err, data){   });

NAN_METHOD(OPELStreaming::streamStart)
{	
	int count;
	if(!info[0]->IsNumber())
	{
		Nan::ThrowTypeError("Second parameter should be callback function");
		return;
	}
	if(!info[1]->IsFunction())
	{
		Nan::ThrowTypeError("Third parameter should be callback function\n");
		return;
	}
/*	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();	*/
	
	count = Nan::To<int>(info[0]).FromJust();
	
	if(count == 0)
		count = INFINITE_NUM;
	
	fprintf(stderr ,"Count : %d\n", count);
	OPELStreaming *streamObj = Nan::ObjectWrap::Unwrap<OPELStreaming>(info.This());
	streamObj->sendDbusMsgCnt("recStart", 0);	

	for(;;){
		if((streamObj->initSharedMemorySpace()))
		{
			break;
		}
	}
  
	Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
	StreamingWorker* streamWorker = new StreamingWorker(callback, count);	
	if(streamWorker == NULL)
	{
		Nan::ThrowError("recWorker Error\n");
		return;
	}
	
	streamWorker->setFd(streamObj->getFd());
	streamWorker->setWidth(streamObj->getWidth());
	streamWorker->setHeight(streamObj->getHeight());
	streamWorker->setBufferSize(streamObj->getBufferSize());
	streamWorker->setBufferIndex(streamObj->getBufferIndex());
	streamWorker->setShmPtr(streamObj->getShmPtr());
	//PES
	if(!(streamWorker->initConnection()))
	{
		Nan::ThrowError("Init Network Connection Error\n");
		return ;
	}
	if(!(streamWorker->initSEM()))
	{
		Nan::ThrowError("init Semaphore Error\n");
		return ;
	}
//	eos = true;
	streamObj->setEos(true);
	Nan::AsyncQueueWorker(streamWorker);
}
NAN_METHOD(OPELStreaming::streamStop)
{	
	fprintf(stderr, "Stop Call invoked\n");
//	eos = false;
	OPELStreaming *streamObj = Nan::ObjectWrap::Unwrap<OPELStreaming>(info.This());
	streamObj->setEos(false);
	usleep(10000);
	streamObj->sendDbusMsg("recStop");
	//send D-bus stop message here
}
void OPELStreaming::setEos(bool setEos)
{
		uv_mutex_lock(&uv_mutex);
		eos = setEos;
		uv_mutex_unlock(&uv_mutex);
}
OPELStreaming::OPELStreaming()
{
	this->width = REC_WIDTH;
	this->height = REC_HEIGHT;
	this->buffer_size = REC_BUFFER_SIZE;
	this->buffer_index = REC_BUFFER_INDEX;
}
OPELStreaming::~OPELStreaming()
{
	
}
void OPELStreaming::sendDbusMsg(const char* msg)
{
	DBusMessage* message;
	message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);
}
bool OPELStreaming::sendDbusMsgCnt(const char* msg, int count)
{
	DBusMessageIter args;
	dbus_int32_t cnt = (dbus_int32_t)count;
	DBusMessage* message;
	message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", msg);
	dbus_message_iter_init_append(message, &args);
	if(!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &cnt))
	{
		fprintf(stderr, "[OPELRecording::sendDbusMsgCnt] : dbus append args error\n");
		return false;
	}
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);
	return true;
}
bool OPELStreaming::initDbus()
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

bool OPELStreaming::openDevice()
{
	this->fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if(-1 == fd)
		return false;
	return true;
}

bool OPELStreaming::initSharedMemorySpace()
{
	shmid = shmget((key_t)REC_SHM_KEY, 0, 0);
	if(shmid == -1)
		return false;
	shmPtr = shmat(shmid, (void*)0, 0666|IPC_CREAT);
	if(shmPtr == (void*)-1)
		return false;
	return true;
} 
bool OPELStreaming::uInitSharedMemorySpace()
{
	if(shmdt(shmPtr) == -1)
		return false;
	return true;
}
bool OPELStreaming::initUVMutex(void)
{
	if(uv_mutex_init(&uv_mutex) == 0)
		return true;
	return false;
}
NAN_METHOD(OPELStreaming::New)
{
	//Nan::NanScope();
	if(info.IsConstructCall())
	{
		OPELStreaming *streamObj = new OPELStreaming();
		streamObj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else
	{
		v8::Local<v8::Function> cons = Nan::New(constructor());
		info.GetReturnValue().Set(cons->NewInstance(0, 0));
	}
}
NAN_MODULE_INIT(OPELStreaming::Init)
{
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
//	v8::Local<v8::FunctionTemplate> tpl1 = Nan::New<v8::FunctionTemplate>(New);
//	tpl1->SetClassName(Nan::New("RecordingWorker").ToLocalChecked());
	
	tpl->SetClassName(Nan::New("OPELStreaming").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
//	tpl1->InstanceTemplate()->SetInternalFieldCount(1);
	
	SetPrototypeMethod(tpl, "init", streamInit);
	SetPrototypeMethod(tpl, "start", streamStart);
	SetPrototypeMethod(tpl, "stop", streamStop);
//	SetPrototypeMethod(tpl, "close", recClose);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELStreaming").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
//	Nan::Set(target, Nan::New("RecordingWorker").ToLocalChecked(), Nan::GetFunction(tpl1).ToLocalChecked());
}


NODE_MODULE(OPELStreaming, OPELStreaming::Init)
