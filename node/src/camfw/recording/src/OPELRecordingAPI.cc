#include "OPELRecordingAPI.h"

const char* dev_name = "/dev/video0";

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
	
	if(!(recObj->initDbus()))
	{
		Nan::ThrowError("D-Bus Initialization Failed\n");
		return;
	}
	if(!(recObj->openDevice()))
	{
		//error
		Nan::ThrowError("Open FileDesc /dev/video0 Failed\n");
		return;
	}
	recObj->sendDbusMsg("recInit");
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
		Nan::ThrowTypeError("First parameter should be callback function");
		return;
	}
	if(!info[1]->IsNumber())
	{
		Nan::ThrowTypeError("Second parameter should be callback function");
		return;
	}
	if(!info[2]->IsFunction())
	{
		Nan::ThrowTypeError("Third parameter should be callback function\n");
		return;
	}
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();	
	
	count = Nan::To<int>(info[1]).FromJust();
	
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	recObj->sendDbusMsgCnt("recStart", count);

	for(;;){
		if((recObj->initSharedMemorySpace()))
		{
			break;
		}
	}
	//recObj->sendDbusMsg("recStart");
  
	Nan::Callback *callback = new Nan::Callback(info[2].As<v8::Function>());
	RecordingWorker* recWorker = new RecordingWorker(callback, file_path, count);	
	if(recWorker == NULL)
	{
		Nan::ThrowError("recWorker Error\n");
		return;
	}
	
	recWorker->setFd(recObj->getFd());
	recWorker->setWidth(recObj->getWidth());
	recWorker->setHeight(recObj->getHeight());
	recWorker->setBufferSize(recObj->getBufferSize());
	recWorker->setBufferIndex(recObj->getBufferIndex());
	recWorker->setShmPtr(recObj->getShmPtr());

	if(!(recWorker->openFileCap()))
	{
		recWorker->closeFileCap();
		Nan::ThrowError("Open File Cap Error\n");
		return ;
	}
	if(!(recWorker->initSEM()))
	{
		Nan::ThrowError("init Semaphore Error\n");
		return ;
	}
	eos = true;
//	fprintf(stderr, "recObj->getWidth() : %d\n", recObj->getWidth());
//	fprintf(stderr, "recObj->getHeight() : %d\n", recObj->getHeight());
//	fprintf(stderr, "recObj->getBufferSize() : %d\n", recObj->getBufferSize());
//	recObj->sendDbusMsg("recStart");
	Nan::AsyncQueueWorker(recWorker);
}
NAN_METHOD(OPELRecording::recStop)
{
/*	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	if(eos){
		eos = false;
		recObj->sendDbusMsg("recStop");
	}
	else
		recObj->sendDbusMsg("recStop");
	*/	
		//만약 워커 쓰레드가 작동하고 있다면 ? 
		//eos를 통해 강제 정지 후 리턴하도록 함
		//워커 쓰레드가 작동하지 않는다면? 그냥 디버스로 recStop끝내라라는 메세지만 센딩
		

}
NAN_METHOD(OPELRecording::recClose)
{
	/*OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	if(eos){
		eos = false;
		recObj->sendDbusMsg("recStop");
		recObj->sendDbusMsg("recClose");
	}
	else
		recObj->sendDbusMsg("recClose");
	*/

	
	//워커 쓰레드를 제거
	 // 일단은 dbus로 close를 보내보자
	 // 그리고 dbus 닫고 sharedmemory 닫고 semaphore 제거하고 Done
}

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
bool OPELRecording::sendDbusMsgCnt(const char* msg, int count)
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
	this->fd = open(dev_name, O_RDWR, 0);
//	this->fd = open("tmp/fifo", O_RDONLY);
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
bool OPELRecording::uInitSharedMemorySpace()
{
	if(shmdt(shmPtr) == -1)
		return false;
	return true;
}
NAN_METHOD(OPELRecording::New)
{
	//Nan::NanScope();
	if(info.IsConstructCall())
	{
		OPELRecording *recObj = new OPELRecording();
		recObj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else
	{
		v8::Local<v8::Function> cons = Nan::New(constructor());
		info.GetReturnValue().Set(cons->NewInstance(0, 0));
	}
}
NAN_MODULE_INIT(OPELRecording::Init)
{
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
//	v8::Local<v8::FunctionTemplate> tpl1 = Nan::New<v8::FunctionTemplate>(New);
//	tpl1->SetClassName(Nan::New("RecordingWorker").ToLocalChecked());
	
	tpl->SetClassName(Nan::New("OPELRecording").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
//	tpl1->InstanceTemplate()->SetInternalFieldCount(1);
	
	SetPrototypeMethod(tpl, "init", recInit);
	SetPrototypeMethod(tpl, "start", recStart);
	SetPrototypeMethod(tpl, "stop", recStop);
	SetPrototypeMethod(tpl, "close", recClose);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELRecording").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
//	Nan::Set(target, Nan::New("RecordingWorker").ToLocalChecked(), Nan::GetFunction(tpl1).ToLocalChecked());
}




NODE_MODULE(OPELRecording, OPELRecording::Init)
