#include "OPELRecordingAPI.h"

static unsigned getPid()
{
	return getpid();
}
void OPELRecording::sendDbusMsg(const char* msg, 
		dbusRequest *dbus_request)
{
 	assert(msg != NULL && dbus_request != NULL);
	DBusMessage* message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, msg);
	dbus_message_append_args(message,
			DBUS_TYPE_STRING, &(dbus_request->file_path),
			DBUS_TYPE_UINT64, &(dbus_request->pid),
			DBUS_TYPE_UINT64, &(dbus_request->fps),
			DBUS_TYPE_UINT64, &(dbus_request->width),
			DBUS_TYPE_UINT64, &(dbus_request->height),
			DBUS_TYPE_UINT64, &(dbus_request->play_seconds),
			DBUS_TYPE_INVALID);
	dbus_connection_send (conn, message, NULL);
	dbus_message_unref(message);
}

NAN_METHOD(OPELRecording::recStart)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	
	int seconds;
	const char* file_path;
	dbusRequest *dbus_request = NULL;	
	
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
	
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();	

	seconds = Nan::To<int>(info[1]).FromJust();

	if(seconds == 0)
	{
		Nan::ThrowTypeError("Seconds cannot be 0");
		return;
	}
	if(!recObj->conn){
		if(!(recObj->initDbus()))
		{
			Nan::ThrowError("D-Bus Initialization Failed\n");
			return;
		}
	}

	dbus_request = new dbusRequest();
	dbus_request->file_path = path;
	dbus_request->pid = getPid();
	dbus_request->fps = 30;
	dbus_request->width = 1920;
	dbus_request->height = 1080;
	dbus_request->play_seconds = seconds;

	recObj->sendDbusMsg(rec_init_request, dbus_request);
		
}

NAN_METHOD(OPELRecording::jpegStart)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	dbusRequest *dbus_request = NULL;	
	const char* file_path;
	
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("First parameter should be callback function");
		return;
	}
	if(!recObj->conn){
		if(!(recObj->initDbus()))
		{
			Nan::ThrowError("D-Bus Initialization Failed\n");
			return;
		}
	}
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);
	file_path = path.c_str();	
	
	dbus_request = (dbusRequest*)malloc(sizeof(dbusRequest));
	dbus_request->file_path = path;
	dbus_request->pid = getPid();
	dbus_request->fps = 1;
	dbus_request->width = 1920;
	dbus_request->height = 1080;

	recObj->sendDbusMsg(snap_start_request, dbus_request);

}

NAN_METHOD(OPELRecording::recStop)
{
}

bool OPELRecording::initDbus()
{
  dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!conn)
	{
//		dbus_error_free(&err);
		return false;
	}
	return true;
}

NAN_METHOD(OPELRecording::New)
{
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
	
	tpl->SetClassName(Nan::New("OPELRecording").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	SetPrototypeMethod(tpl, "RecordingStart", recStart);
	SetPrototypeMethod(tpl, "RecordingStop", recStop);
	SetPrototypeMethod(tpl, "SnapshotStart", jpegStart);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELRecording").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}


NODE_MODULE(OPELRecording, OPELRecording::Init)
