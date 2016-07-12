#include <node.h>
#include <stdlib.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <uv.h>
#include <signal.h>
#include <sys/time.h>

#include "nil.h"

#define MAX_STRING_LENTGH	256
#define MAX_DATA_LENGTH		20
#define SENSOR_REQUEST	1
#define SM_INTERFACE "/org/opel/sensorManager"
#define SM_PATH	"org.opel.sensorManager"
#define REQUEST_DELAY	50 //ms

static DBusConnection *opelCon = NULL; //Only 1 connection 
static struct timeval time_to_delay; //임시 변수, event register시 약간의 시간 차를 두기 위해 사용.
requestList *rList;
pid_t pid;

int check_sensor_name(const char* name){
	int i;

	for (i = 0; ; i++){
		if (!strcmp(name, SUPPORT_LIST[i]))
			return i;
		else if (!strcmp("END", SUPPORT_LIST[i]))
			break;
	}

	return -1;
}
int parsingString(char* string, char* value){
	//devide string into word.
	int i, j;
	int length = strlen(string);
	
	//printf("input : %s\n", string);

	for (i = 0; i < MAX_STRING_LENTGH; i++){
		if (string[i] != ' ')
			break;
	}
	
	if (i  >= length)
		return 1;

	for (j = i; j <= length; j++){
		if (string[j] == ' '){
			j--;
			break;
		}
	}
	
	strncpy(value, &(string[i]), j - i + 1);
	value[j - i + 1] = '\0'; // NULL;

	for (i = 0; i <= j; i++)
		string[i] = ' ';

	string[length] = '\0'; // NULL;

	return 0;
}
int parsingToArgv(requestList* rl, char* in_value, char* in_value_type, char* in_value_name){
	//Make callback arguments
	v8::Local<Object> obj = Object::New();
	HandleScope scope;
	TryCatch try_catch;

	char value[MAX_STRING_LENTGH];
	char type[MAX_STRING_LENTGH];
	char name[MAX_STRING_LENTGH];

	char value_temp[MAX_DATA_LENGTH];
	char type_temp[MAX_DATA_LENGTH];
	char name_temp[MAX_DATA_LENGTH];

	strcpy(value, in_value);
	strcpy(type, in_value_type);
	strcpy(name, in_value_name);

	//obj->Set(String::NewSymbol(name), Integer::New(value));

	while (1){
		if (parsingString(value, value_temp))
			break;

		parsingString(type, type_temp);
		parsingString(name, name_temp);

		if (!strcmp(type_temp, "FLOAT")){
			obj->Set(String::NewSymbol(name_temp), Number::New(atof(value_temp)));
		}
		else if (!strcmp(type_temp, "INT")){
			obj->Set(String::NewSymbol(name_temp), Integer::New(atoi(value_temp)));
		}
		else if (!strcmp(type_temp, "STRING")){
			obj->Set(String::NewSymbol(name_temp), String::New(value_temp));
		}
		else{
			printf("Error : not supported data type %s\n", type_temp);
		}
	}


	Handle<Value> argv[] = {
		obj
	};

	rl->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	if (try_catch.HasCaught()) {
		node::FatalException(try_catch);
	}

	return 0;
}
Handle<Value> parsingToReturn(char* in_value, char* in_value_type, char* in_value_name){
	//Make return value
	v8::Local<Object> obj = Object::New();
	HandleScope scope;
	TryCatch try_catch;

	char value[MAX_STRING_LENTGH];
	char type[MAX_STRING_LENTGH];
	char name[MAX_STRING_LENTGH];

	char value_temp[MAX_DATA_LENGTH];
	char type_temp[MAX_DATA_LENGTH];
	char name_temp[MAX_DATA_LENGTH];

	strcpy(value, in_value);
	strcpy(type, in_value_type);
	strcpy(name, in_value_name);

	//obj->Set(String::NewSymbol(name), Integer::New(value));

	while (1){
		if (parsingString(value, value_temp))
			break;

		parsingString(type, type_temp);
		parsingString(name, name_temp);

		if (!strcmp(type_temp, "FLOAT")){
			obj->Set(String::NewSymbol(name_temp), Number::New(atof(value_temp)));
		}
		else if (!strcmp(type_temp, "INT")){
			obj->Set(String::NewSymbol(name_temp), Integer::New(atoi(value_temp)));
		}
		else if (!strcmp(type_temp, "STRING")){
			obj->Set(String::NewSymbol(name_temp), String::New(value_temp));
		}
		else{
			printf("Error : not supported data type %s\n", type_temp);
		}
	}


	return scope.Close(obj);
}

// Handler function for dbus message
// 1. On (Periodic)
// 2. On (Notify)
DBusHandlerResult sensorEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	int rq_num;
	char* sensorValue;
	char* valueType;
	char* valueName;
	requestList* rl;
	TryCatch try_catch;
	DBusError err;
	dbus_error_init(&err);

	//printRequset(rList);

	dbus_message_get_args(message, &err,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_STRING, &sensorValue,
		DBUS_TYPE_STRING, &valueType,
		DBUS_TYPE_STRING, &valueName,
		DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&err))
	{
		printf("Error get data: %s \n", err.message);
		dbus_error_free(&err);
	}
	//printf("[NIL] Receive rq_num : %d / value :%d\n", rq_num, sensorValue);
	
	rl = getRequest(rList, rq_num);
	
	//printf("Received data : value[%s] type[%s] name[%s]\n", sensorValue, valueType, valueName);
	parsingToArgv(rl, sensorValue, valueType, valueName);
	
	return DBUS_HANDLER_RESULT_HANDLED;
}
DBusHandlerResult sensorEventNotify(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	int rq_num;
	requestList* rl;
	TryCatch try_catch;
	DBusError err;

	dbus_error_init(&err);
	
	//printRequset(rList);

	dbus_message_get_args(message, &err,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&err))
	{
		printf("Error get data: %s", err.message);
		dbus_error_free(&err);
	}

	printf("[NIL] Event Notify rq_num : %d", rq_num);

	rl = getRequest(rList, rq_num);


	rl->callback->Call(Context::GetCurrent()->Global(), 0, NULL);
	if (try_catch.HasCaught()) {
		node::FatalException(try_catch);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

int wait_delay(){
	//Node.js Multi thread 환경에서 sleep으로 인한 문제가 발생하기 때문에
	//어쩔 수 없이 busy wait로 구현
	struct timeval temp;
	int i;
	
	while (1){
		gettimeofday(&temp, NULL);

		if (temp.tv_sec > time_to_delay.tv_sec)
			break;
		else if (temp.tv_usec >= (time_to_delay.tv_usec + (REQUEST_DELAY * 1000)))
			break;
		else{
			for (i = 0; i < 10000; i++);
		}
	}

	gettimeofday(&time_to_delay, NULL);

	return 0;
}

// Sensor manager API
// 1. On
// 2. Get
Handle<Value> On(const Arguments& args) {
	HandleScope scope;
	requestList* rl;
	DBusMessage* msg;
	DBusError err;
	int pid;
	int rq_num;
	const char* sensor_name;
	int sensing_interval;
	int handle_type = 0;
	const char* handle_type_s;
	int sensing_level = 0; //현재는 사용하지 않음.

	
	wait_delay(); //Perform wait.

	/*
	Receive Args
	1. sensor name
	2. handle type
	3. sensing interval
	4. Function
	*/

	//----------------------------------------------------------------//
	//						1. Argument Check
	printf("Length : %d\n", args.Length());
	if ((args.Length() < 4) ||	
		!args[0]->IsString() || 
		!args[1]->IsString() ||
		!args[2]->IsInt32() ||
		!args[3]->IsFunction() ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 4 arguments expected [Sensor Name, Handling Type, Interval, Function]")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//


	//--------------------- 1.1 Sensor Name Check --------------------//
	// 서포트 리스트와 비교해서, 지원하는 센서인지 체크
	//
	v8::String::Utf8Value param0(args[0]->ToString());
	std::string name_c = std::string(*param0);
	sensor_name = name_c.c_str();

	if (check_sensor_name(sensor_name) == -1){
		ThrowException(Exception::TypeError(String::New("This sensor is not supported sensor\n")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//

	//--------------------- 1.2 Handle Type Check --------------------//
	//
	v8::String::Utf8Value param1(args[1]->ToString());
	std::string type_c = std::string(*param1);
	handle_type_s = type_c.c_str();

	if (!(strcmp(handle_type_s, "PERIODIC"))){
		handle_type = SENSING_INTERVAL;
	}
	else if (!(strcmp(handle_type_s, "NOTIFY"))){
		handle_type = SENSING_EVENT_DRIVEN;
	}

	if (!handle_type){
		ThrowException(Exception::TypeError(String::New("Not supported handling type\n")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//

	//--------------------- 1.3 interval check --------------------//
	//
	sensing_interval = args[2]->IntegerValue();
	
	if (sensing_interval < MIN_INTERVAL_TIME){
		ThrowException(Exception::TypeError(String::New("Too small interval time\n")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//



	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)
	rl = newRequest(rList);
	rl->callback = Persistent<Function>::New(Local<Function>::Cast(args[3]));
	rl->type = SENSOR_REQUEST;
	
	pid = (unsigned int)getpid();
	rq_num = rl->rq_num;
	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);
	msg = dbus_message_new_signal(SM_INTERFACE, SM_PATH, "register");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_STRING, &(sensor_name),
		DBUS_TYPE_INT32, &(handle_type),
		DBUS_TYPE_INT32, &(sensing_interval),
		DBUS_TYPE_INT32, &sensing_level,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	//printf("[NIL] Send message (Event Register) to %s | %s\n", SM_PATH, SM_INTERFACE);
	//
	//----------------------------------------------------------------//

	//return scope.Close(Undefined());
	return scope.Close(Integer::New(rq_num));
}
Handle<Value> Get(const Arguments& args) {
	const char* sensorName;
	char* sensorValue;
	char* valueType;
	char* valueName;

	HandleScope scope;
	DBusMessage* msg;
	DBusMessage* reply;
	DBusError error;	

	//------------------- Argument check-----------------------------//
	if (args.Length() != 1) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [Sensor Name]")));
		return scope.Close(Undefined());
	}
	if (!args[0]->IsString()) { //IsInteger, IsFunction, etc...
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	//----------------------------------------------------------------//
	
	
	//--------------------- Sensor Name Check -------------------------//
	// 서포트 리스트와 비교해서, 지원하는 센서인지 체크
	//
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string name_c = std::string(*param1);
	sensorName = name_c.c_str();

	if ( check_sensor_name(sensorName) == -1){
		ThrowException(Exception::TypeError(String::New("This sensor is not supported!")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//
	

	//-------------------- DBus Message Initilizing -----------------// 
	//
	msg = dbus_message_new_method_call("org.opel.sensorManager", // target for the method call
		"/", // object to call on
		"org.opel.sensorManager", // interface to call on
		"Get"); // method name
	if (NULL == msg) {
		printf("Message Null\n");
		ThrowException(Exception::TypeError(String::New("Fail to create message")));
		return scope.Close(Undefined());
	}

	dbus_message_append_args(msg,
		DBUS_TYPE_STRING, &sensorName,
		DBUS_TYPE_INVALID);
	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	// send message and get a reply
	//
	dbus_error_init(&error);

	if (opelCon == NULL){
		printf("Why null? \n");
		opelCon = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	}
	
	//printf("Send Message \n");
	reply = dbus_connection_send_with_reply_and_block(opelCon, msg, 500, &error); // Timeout 500 milli seconds

	if (reply == NULL){
		printf("Get Null Reply \n");
		printf("Error : %s\n", error.message);
	}
	dbus_error_free(&error);
	//printf("Got  Reply Message \n");

	dbus_message_unref(msg);
	//
	//----------------------------------------------------------------//


	//----------------------------------------------------------------//
	//
	dbus_message_get_args(reply, NULL,
		DBUS_TYPE_STRING, &sensorValue,
		DBUS_TYPE_STRING, &valueType,
		DBUS_TYPE_STRING, &valueName,
		DBUS_TYPE_INVALID);

	dbus_message_unref(reply);
	printf("receive : %d \n", sensorValue);
	//
	//----------------------------------------------------------------//

	return parsingToReturn(sensorValue, valueType, valueName);
	//Refer this : http://luismreis.github.io/node-bindings-guide/docs/returning.html
	//About return value from Native to Java	
}
Handle<Value> Update(const Arguments& args){
	HandleScope scope;
	requestList* rl;
	DBusMessage* msg;
	DBusError err;
	int pid;
	int rq_num;
	int sensing_interval = 0;
	int handle_type = 0;
	const char* handle_type_s;
	int sensing_level = 0; //현재는 사용하지 않음.
	int request_change_flag = 0;


	//----------------------------------------------------------------//
	//						1. Argument Check
	printf("Length : %d\n", args.Length());
	if ((args.Length() < 4) ||
		!args[0]->IsInt32() ||
		!(args[1]->IsString() || args[1]->IsNull()) ||
		!(args[2]->IsInt32() || args[2]->IsNull()) ||
		!(args[3]->IsFunction() || args[3]->IsNull()) ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 4 arguments expected [Request ID, Handling Type[or NULL], Interval[or NULL], Function[or NULL]]")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//
	
	rq_num = args[0]->IntegerValue();
	pid = (unsigned int)getpid();

	//--------------------- 1.2 Handle Type Check --------------------//
	// 서포트 리스트와 비교해서, 지원하는 센서인지 체크
	//
	if (args[1]->IsString()){
		v8::String::Utf8Value param1(args[1]->ToString());
		std::string type_c = std::string(*param1);
		handle_type_s = type_c.c_str();

		if (!(strcmp(handle_type_s, "PERIODIC"))){
			handle_type = SENSING_INTERVAL;
		}
		else if (!(strcmp(handle_type_s, "NOTIFY"))){
			handle_type = SENSING_EVENT_DRIVEN;
		}//Need to oneshot?

		if (!handle_type){
			ThrowException(Exception::TypeError(String::New("Not supported handling type\n")));
			return scope.Close(Undefined());
		}

		request_change_flag = 1;
	}
	//
	//----------------------------------------------------------------//

	//--------------------- 1.3 interval check --------------------//
	// 서포트 리스트와 비교해서, 지원하는 센서인지 체크
	//
	if (args[2]->IsInt32()){
		sensing_interval = args[2]->IntegerValue();

		if (sensing_interval < MIN_INTERVAL_TIME){
			ThrowException(Exception::TypeError(String::New("Too small interval time\n")));
			return scope.Close(Undefined());
		}
		request_change_flag = 1;
	}
	//
	//----------------------------------------------------------------//


	//-----------------------------------------------------------------//
	if (args[3]->IsFunction()){
		rl = getRequest(rList, rq_num);
		rl->callback = Persistent<Function>::New(Local<Function>::Cast(args[3]));	
	}

	if (request_change_flag){
		dbus_error_init(&err);
		msg = dbus_message_new_signal(SM_INTERFACE, SM_PATH, "update");

		dbus_message_append_args(msg,
			DBUS_TYPE_INT32, &pid,
			DBUS_TYPE_INT32, &rq_num,
			DBUS_TYPE_INT32, &(handle_type),
			DBUS_TYPE_INT32, &(sensing_interval),
			DBUS_TYPE_INT32, &sensing_level,
			DBUS_TYPE_INVALID);

		/* Send the signal */
		dbus_connection_send(opelCon, msg, NULL);
		dbus_message_unref(msg);

		printf("[NIL] send message to %s | %s\n", SM_PATH, SM_INTERFACE);

	}
	//-----------------------------------------------------------------//
	
	return scope.Close(Undefined());
}
Handle<Value> Unregister(const Arguments& args){
	HandleScope scope;
	DBusMessage* msg;
	DBusError err;
	int pid;
	int rq_num;

	
	//----------------------------------------------------------------//
	//						1. Argument Check
	printf("Length : %d\n", args.Length());
	if ((args.Length() < 1) ||
		!args[0]->IsInt32() ){
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [Request ID]")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//
	
	rq_num = args[0]->IntegerValue();
	pid = (unsigned int)getpid();

	//1.UNREGISTER DBUS MESSAGE
	dbus_error_init(&err);
	msg = dbus_message_new_signal(SM_INTERFACE, SM_PATH, "unregister");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] send message to %s | %s\n", SM_PATH, SM_INTERFACE);
	//
	//----------------------------------------------------------------//


	//2.DELETE REQEUST FROM NIL

	deleteRequest(rList, rq_num);

	return scope.Close(Undefined());
}


//----------------------- exit handler--------------------------//
//
void all_request_unregister(){
	DBusMessage* msg;
	DBusError err;
	int pid;
	int rq_num;

	pid = (unsigned int)getpid();
	rq_num = -1;

	dbus_error_init(&err);
	msg = dbus_message_new_signal(SM_INTERFACE, SM_PATH, "unregister");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] send All_UNREGISTER message to %s | %s\n", SM_PATH, SM_INTERFACE);
}
void exit_handler(void){
	all_request_unregister();
	//Perform unregister
}
void sigint_handler(int signo)
{
	//Perform unregister
	all_request_unregister();
	signal(SIGINT, SIG_DFL);
	//Sig call
	kill(getpid(), SIGINT);
}
//
//-------------------------------------------------------------//

void nativeInterfaceLayerInit(){
	pid = getpid();
	rList = (requestList*)malloc(sizeof(requestList));
	initRequestList(rList);
	gettimeofday(&time_to_delay, NULL);
}

void init(Handle<Object> exports) {
	nativeInterfaceLayerInit();
	opelCon = DbusInit(); //Init Dbus message receiver (by PID)

	if (atexit(exit_handler)) printf("Failed to register exit_handler1\n");
	signal(SIGINT, sigint_handler);

	exports->Set(String::NewSymbol("Get"),
		FunctionTemplate::New(Get)->GetFunction());
	exports->Set(String::NewSymbol("On"),
		FunctionTemplate::New(On)->GetFunction());
	exports->Set(String::NewSymbol("EventRegister"),
		FunctionTemplate::New(On)->GetFunction());
	exports->Set(String::NewSymbol("EventUpdate"),
		FunctionTemplate::New(Update)->GetFunction());
	exports->Set(String::NewSymbol("EventUnregister"),
		FunctionTemplate::New(Unregister)->GetFunction());





}

NODE_MODULE(nil, init)


