#include "OPELRecordingAPI.h"

static unsigned getPid()
{
	return getpid();
}

DBusMessage* OPELRecording::sendDbusMsg(const char* msg, 
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
	return message;
}

DBusMessage* OPELRecording::sendStreamingDbusMsg(const char* msg,
		dbusStreamingRequest *dbus_request)
{
 	assert(msg != NULL && dbus_request != NULL);
	DBusMessage* message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, msg);
	dbus_message_append_args(message,
			DBUS_TYPE_STRING, &(dbus_request->ip_address),
			DBUS_TYPE_UINT64, &(dbus_request->port),
			DBUS_TYPE_INVALID);
	dbus_connection_send(conn, message, NULL);
	return message;
}

void OPELrecordingAsync::Execute()
{
	assert(this->conn != NULL && this->msg != NULL);
//	unsigned success;
//	DBusError *err;
/*	this->reply = dbus_connection_send_with_reply_and_block(this->conn,
		this->msg, this->seconds*1000 + 2000, err);
	*/
	sleep(seconds+1);
/*	if(reply == NULL)
	{
		this->is_success = true;
//		dbus_error_free(err);
		dbus_message_unref(msg);
		return;
	}
	else{
//		dbus_error_free(err);
		dbus_message_unref(msg);
		dbus_message_get_args(reply, NULL,
			DBUS_TYPE_UINT64, &success,
			DBUS_TYPE_INVALID);
		if(success == 0)
		{
			this->is_success = 1;
		}
		this->is_success = 1;
	}*/
	this->is_success = 1;
}

void OPELrecordingAsync::HandleOKCallback()
{
	Nan::HandleScope scope;
	v8::Local<v8::Value> argv[] = { Nan::New<v8::Number>(this->is_success) };
	callback->Call(1, argv);
}

NAN_METHOD(OPELRecording::recStart)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	
	int seconds;
	dbusRequest *dbus_request = NULL;
	
	OPELrecordingAsync *recordingAsync;
	
	DBusMessage* message;
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("First parameter should be File Path");
		return;
	}
	if(!info[1]->IsNumber())
	{
		Nan::ThrowTypeError("Second Parameter Should be Number for Seconds");
		return;
	}
	if(!info[2]->IsFunction())
	{
		Nan::ThrowTypeError("Third Parameter Should be a Callback Function");
		return;
	}
	Nan::Callback *callback = new Nan::Callback(info[2].As<v8::Function>());
	v8::String::Utf8Value param1(info[0]->ToString());
	std::string path = std::string(*param1);

	seconds = Nan::To<int>(info[1]).FromJust();

	if(seconds == 0)
	{
		Nan::ThrowTypeError("Seconds cannot be 0");
		return;
	}
	if(!(recObj->initDbus()))
	{
			Nan::ThrowError("D-Bus Initialization Failed\n");
			return;
	}

	dbus_request = new dbusRequest();
	dbus_request->file_path = path;
	dbus_request->pid = getPid();
	dbus_request->fps = 30;
	dbus_request->width = 1920;
	dbus_request->height = 1080;
	dbus_request->play_seconds = seconds;
	message = recObj->sendDbusMsg(rec_init_request, dbus_request);
	
	recordingAsync = new OPELrecordingAsync(callback, recObj->conn, 
			message, seconds);
	
	Nan::AsyncQueueWorker(recordingAsync);
}

NAN_METHOD(OPELRecording::jpegStart)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	DBusMessage *reply;
	dbusRequest *dbus_request = NULL;	
	DBusMessage *message;
	DBusError err;
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
	
	dbus_request = new dbusRequest();
	dbus_request->file_path = path;
	dbus_request->pid = getPid();
	dbus_request->fps = 1;
	dbus_request->width = 1920;
	dbus_request->height = 1080;
	dbus_request->play_seconds = 1;

	message = recObj->sendDbusMsg(snap_start_request, dbus_request);
	
//	reply = dbus_connection_send_with_reply_and_block(recObj->conn,
//			message, 500, &err);
	sleep(1);
	dbus_message_unref(message);
//	dbus_message_unref(reply);
}

NAN_METHOD(OPELRecording::streamingStart)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	DBusMessage* message;
	std::string ip_address;
	dbusStreamingRequest *dbus_request;
	unsigned port;
	if(!info[0]->IsString())
	{
		Nan::ThrowTypeError("First Parameter Should be IP Address");
		return;
	}
	if(!info[1]->IsNumber())
	{
		Nan::ThrowTypeError("Second Parameter should be a Port Number");
		return;
	}
	v8::String::Utf8Value param1(info[0]->ToString());
	ip_address = std::string(*param1);
	port = Nan::To<int>(info[1]).FromJust();	
	
	if(!(recObj->initDbus()))
	{
		Nan::ThrowError("D-Bus Initiailization Failed\n");
		return;
	}
	dbus_request = new dbusStreamingRequest();
	dbus_request->ip_address = ip_address;
	dbus_request->port = port;
	message = recObj->sendStreamingDbusMsg(streaming_start_request, dbus_request);
	
}

NAN_METHOD(OPELRecording::streamingStop)
{
	OPELRecording *recObj = Nan::ObjectWrap::Unwrap<OPELRecording>(info.This());
	if(!(recObj->initDbus()))
	{
		Nan::ThrowError("D-Bus Initiailization Failed\n");
		return;
	}
	DBusMessage* message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, streaming_stop_request);
	dbus_connection_send(recObj->conn, message, NULL);
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
		dbus_error_free(&err);
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

	SetPrototypeMethod(tpl, "streamingStart", streamingStart);
	SetPrototypeMethod(tpl, "streamingStart", streamingStart);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("OPELRecording").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NODE_MODULE(OPELRecording, OPELRecording::Init)
