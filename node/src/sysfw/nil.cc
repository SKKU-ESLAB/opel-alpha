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
#include "nil.h"


#define SENSOR_REQUEST	1
#define CAMERA_REQUEST	2
#define APP_REQUEST	3


#define SM_INTERFACE "/org/opel/sensorManager"
#define SM_PATH	"org.opel.sensorManager"

#define SAM_INTERFACE "/org/opel/sysAppManager"
#define SAM_PATH	"org.opel.sysAppManager"

#define MAXNUMBEROFITEMS 20
#define MSGBUFSIZE 1024


pid_t pid;
static DBusConnection *opelCon = NULL; //Only 1 connection 
requestList *rList;

void nativeInterfaceLayerInit(){
	pid = getpid();
	rList = (requestList*)malloc(sizeof(requestList));
	initRequestList(rList);
}

DBusHandlerResult configEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	int rq_num;
	char* jsonData;
	
	requestList* rl;
	TryCatch try_catch;
	DBusError err;
	dbus_error_init(&err);

	printf("[NIL] Response function\n");

//	printRequset(rList);

	dbus_message_get_args(message, &err,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_STRING, &jsonData,
		DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&err))
	{
		printf("Error get data: %s", err.message);
		dbus_error_free(&err);
	}

	printf("[NIL] Receive Config rq_num : %d / value :%s\n\n\n\n\n", rq_num, jsonData);
	
	rl = getRequest(rList, rq_num);

	Handle<Value> argv[] = {
		String::New(jsonData)
		//String::New(data)
	};


	rl->callback->Call(Context::GetCurrent()->Global(), 1, argv);


	if (try_catch.HasCaught()) {
		node::FatalException(try_catch);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}


DBusHandlerResult terminationEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	int rq_num;

	
	requestList* rl;
	TryCatch try_catch;
	DBusError err;
	dbus_error_init(&err);

//	printRequset(rList);

	dbus_message_get_args(message, &err,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_INVALID);

	if (dbus_error_is_set(&err))
	{
		printf("Error get data: %s", err.message);
		dbus_error_free(&err);
	}
	
	rl = getRequest(rList, rq_num);

	Handle<Value> argv[] = {
//		String::New(jsonData)
		//String::New(data)
	};

	printf("[NIL] termination Event >> pid : %d, Receive rq_num : %d \n", getpid(), rq_num);
	rl->callback->Call(Context::GetCurrent()->Global(), 0, argv); // [CHECK]


	if (try_catch.HasCaught()) {
		node::FatalException(try_catch);
	}

	exit(1);
	return DBUS_HANDLER_RESULT_HANDLED;
}




// 0 = Invalid
// 1 = Noti page
// 2 = Config page
// 3 = Config event driven page
int check_page_name(const char* name){

	char tempString[15];
	
	strncpy(tempString, name, 14);
	tempString[14] = '\0';
	

	if(!strcmp(tempString, "{\"noti\":\"noti\"")){
		return 1;
	}
	
	if(!strcmp(tempString, "{\"conf\":\"conf\"")){
		return 2;
	}

	
	if(!strcmp(tempString, "{\"conf_eventd\"")){
			return 3;
	}
	
	return 0;
}


bool check_key_overlap(const char* json, const char* name){
	int position = 0;

	char key[512];
	memset(key, '\0', 512);
	
		

	for(unsigned int i=0; i< strlen(json); i++){

		if ( json[i] == '{' )
			continue;

		else if ( json[i] == '}' ){
			if(!strcmp(key, name)){

				return false;
			}
		}
		
		else if ( json[i] == ':' ){
			if( json[i+1] == '"')
				position++;
		}

		else if ( json[i] == ',' ){
			if( json[i+1] == '"'){
				position++;

				if(!strcmp(key, name)){

					return false;
				}
				else{

					memset(key, '\0', 512);

				}
			}
		}

		else if ( json[i] == '"' ){
			continue;
		}

		else{		
			
			char tmp[2];
			tmp[1] = '\0';
			tmp[0] = json[i];			
			
			if(! (position % 2)) // even num -> key
				strcat(key, tmp);
			else{}				// odd num -> value
				
		}
	}
	
	return true;
}

Handle<Value> makeEventPage(const Arguments& args) {

		HandleScope scope;
		const char* description;
				
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string inputJsonData = std::string(*param1);
		description = inputJsonData.c_str();

		if (args.Length() == 0) {
				return scope.Close(String::New("{\"noti\":\"noti\",\"description\":\"\"}"));
		}
		
		if (args.Length() == 1) {

			char newJsonData[sizeof(char) * strlen(description) + 32];
			sprintf(newJsonData, "{\"noti\":\"noti\",\"description\":\"%s\"}" , description);
			return scope.Close(String::New(newJsonData));
		}

		ThrowException(Exception::TypeError(String::New("Invalid Use : arguments expected [Null or Description(string)]")));
		return scope.Close(Undefined());
		
		//Refer this : http://luismreis.github.io/node-bindings-guide/docs/returning.html
		//About return value from Native to Java	
}


Handle<Value> addEventText(const Arguments& args) {

		int newJsonLength;
	//	char* newJsonData;
		
		const char* oldJsonData;
		const char* textView;
		
		HandleScope scope;

		if (args.Length() != 2) {
				ThrowException(Exception::TypeError(String::New("Invalid Use : 2 arguments expected [Page Name]")));
				return scope.Close(Undefined());
		}
		
		if (!args[0]->IsString()) { //IsInteger, IsFunction, etc...
				ThrowException(Exception::TypeError(String::New("Wrong arguments")));
				return scope.Close(Undefined());
		}
				
		if (!args[1]->IsString()) { //IsInteger, IsFunction, etc...
				ThrowException(Exception::TypeError(String::New("Wrong arguments")));
				return scope.Close(Undefined());
		}
		
			//----------------------------------------------------------------//
			
			
			//--------------------- Page Name Check -------------------------//
			
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string inputJsonData = std::string(*param1);
		oldJsonData = inputJsonData.c_str();

		v8::String::Utf8Value param2(args[1]->ToString());
		std::string inputText = std::string(*param2);
		textView = inputText.c_str();
		
		if ( check_page_name(oldJsonData) != 1){
			ThrowException(Exception::TypeError(String::New("addEventItem argument error : first arg is not the object of event page")));
			return scope.Close(Undefined());				
		}


		newJsonLength = strlen(oldJsonData) + strlen(textView) + 20;
//		newJsonData = (char*)malloc(sizeof(char) * newJsonLength);
		char newJsonData[sizeof(char) * newJsonLength];
		memset(newJsonData, '\0', newJsonLength);
		

		char tmpStr[strlen(oldJsonData)];
		strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
		tmpStr[strlen(oldJsonData)-1] = '\0';

	
		sprintf(newJsonData, "%s,\"text\":\"%s\"}" , tmpStr, textView);


		return scope.Close(String::New(newJsonData));
				//Refer this : http://luismreis.github.io/node-bindings-guide/docs/returning.html
				//About return value from Native to Java	

		
}

// [MORE] addDescription

Handle<Value> addEventImg(const Arguments& args) {
		
		int newJsonLength;
//		char* newJsonData;
		
		const char* oldJsonData;
		const char* imgPath;
		
		HandleScope scope;

		if (args.Length() != 2) {
				ThrowException(Exception::TypeError(String::New("Invalid Use : 2 arguments expected [Page Name]")));
				return scope.Close(Undefined());
		}

		if (!args[0]->IsString()) { //IsInteger, IsFunction, etc...
				ThrowException(Exception::TypeError(String::New("Wrong arguments")));
				return scope.Close(Undefined());
		}

		if (!args[1]->IsString()) { //IsInteger, IsFunction, etc...
				ThrowException(Exception::TypeError(String::New("Wrong arguments")));
				return scope.Close(Undefined());
		}

		//----------------------------------------------------------------//

			
		//--------------------- Page Name Check -------------------------//
			
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string inputJsonData = std::string(*param1);
	oldJsonData = inputJsonData.c_str();

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string inputText = std::string(*param2);
	imgPath = inputText.c_str();

	if ( check_page_name(oldJsonData) != 1){
		ThrowException(Exception::TypeError(String::New("addEventItem argument error : first arg is not the object of event page")));
		return scope.Close(Undefined());				
	}
	
	newJsonLength = strlen(oldJsonData) + strlen(imgPath)+256 ;
	//newJsonData = (char*)malloc(sizeof(char) * newJsonLength);
	char newJsonData[sizeof(char) * newJsonLength];
	memset(newJsonData, '\0', newJsonLength);
	
	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"img\":\"%s\"}" , tmpStr, imgPath);
	
	return scope.Close(String::New(newJsonData));

		
}

Handle<Value> sendEventPage(const Arguments& args) {

	HandleScope scope;

	const char* jsonData;
	
	DBusMessage* msg;
	DBusError err;

	dbus_int32_t pid;
	dbus_int32_t noti = 0;
	//----------------------------------------------------------------//
	//						1. Argument Check
	if ((args.Length() != 1) ||	!args[0]->IsString() ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [Page obj]")));
		return scope.Close(Undefined());
	}

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string name_c = std::string(*param1);
	jsonData = name_c.c_str();

	if ( check_page_name(jsonData) != 1){
		ThrowException(Exception::TypeError(String::New("sendEventItem argument error : first arg is not the object of event page")));
		return scope.Close(Undefined());				
	}

	
	//
	//----------------------------------------------------------------//


	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)


	pid = (unsigned int)getpid();

	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);
	msg = dbus_message_new_signal(SAM_INTERFACE, SAM_PATH, "sendNotiPage");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_STRING, &jsonData,
		DBUS_TYPE_INT32, &noti,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] SendEventPage to Manager>>  %s\n", jsonData);
	//
	//----------------------------------------------------------------//

	return scope.Close(Undefined());
}


Handle<Value> sendEventPageWithNoti(const Arguments& args) {

	HandleScope scope;

	const char* jsonData;
	
	DBusMessage* msg;
	DBusError err;

	dbus_int32_t pid;
	dbus_int32_t noti = 1;
	//----------------------------------------------------------------//
	//						1. Argument Check
	if ((args.Length() != 1) ||	!args[0]->IsString() ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [Page obj]")));
		return scope.Close(Undefined());
	}

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string name_c = std::string(*param1);
	jsonData = name_c.c_str();

	if ( check_page_name(jsonData) != 1){
		ThrowException(Exception::TypeError(String::New("sendEventItem argument error : first arg is not the object of event page")));
		return scope.Close(Undefined());				
	}

	
	//
	//----------------------------------------------------------------//


	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)


	pid = (unsigned int)getpid();

	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);
	msg = dbus_message_new_signal(SAM_INTERFACE, SAM_PATH, "sendNotiPage");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_STRING, &jsonData,
		DBUS_TYPE_INT32, &noti,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] SendEventPage to Manager>>  %s\n", jsonData);
	//
	//----------------------------------------------------------------//

	return scope.Close(Undefined());
}





//JSON Warnning : any string shouln't has '"' char
//OUTPUT Json format : 
/*
	{
	"conf":"conf",
	"strTB":"Name[descript/10]",
	"numTB":"Name[descript/1/10]",
	"sDialog":"Name[descript/a/b/c/d/e]",
	"mDialog":"Name[descript/a[a]/b/c/d/e]",
	"dateDialog":Name[descript/flag]",
	"timeDialog":Name[descript/flag]"
	}
	

*/

//----------------------   config page -------------------------//
Handle<Value> makeConfigPage(const Arguments& args) {

		HandleScope scope;
		
		return scope.Close(String::New("{\"conf\":\"conf\"}"));
		//Refer this : http://luismreis.github.io/node-bindings-guide/docs/returning.html
		//About return value from Native to Java	
}



// addStrTextbox(page, name, description, length);
// Return new Json string
Handle<Value> addStrTextbox(const Arguments& args) {


		int newJsonLength;

		const char* oldJsonData;

		const char* description;

		const char* name;
		const char* length;
		HandleScope scope;
		
		

		if (args.Length() != 4) {
				ThrowException(Exception::TypeError(String::New("Invalid Use : 1st arguments expected [Page, Name(str), Description(str), length(int)]")));
				return scope.Close(Undefined());
			}
		if (!args[0]->IsString()||!args[1]->IsString() ||!args[2]->IsString()|| !args[3]->NumberValue() > 0  ) { //IsInteger, IsFunction, etc...
				ThrowException(Exception::TypeError(String::New("Wrong arguments")));
				return scope.Close(Undefined());
			}
		//----------------------------------------------------------------//

			
		//--------------------- Page Name Check -------------------------//
			
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string json = std::string(*param1);
		oldJsonData = json.c_str();

		v8::String::Utf8Value param2(args[1]->ToString());
		std::string name_c = std::string(*param2);
		name = name_c.c_str();
		
		v8::String::Utf8Value param11(args[2]->ToString());
		std::string des = std::string(*param11);
		description= des.c_str();

		v8::String::Utf8Value param3(args[3]->ToString());
		std::string length_c = std::string(*param3);
		length = length_c.c_str();

		if ( check_page_name(oldJsonData) != 2){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : first arg is not the object of config page")));
				return scope.Close(Undefined());				
		}
		
		if (check_key_overlap(oldJsonData, name) == false){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
		}

		newJsonLength = strlen(oldJsonData) + strlen(name) + strlen(length) + 32;
//		newJsonData = (char*)malloc(sizeof(char) * newJsonLength);

		char newJsonData [sizeof(char) * newJsonLength];
		memset(newJsonData, '\0', newJsonLength);


		char tmpStr[strlen(oldJsonData)];
		strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
		tmpStr[strlen(oldJsonData)-1] = '\0';

	
		sprintf(newJsonData, "%s,\"strTB\":\"%s[%s/%s]\"}" , tmpStr, name, description,length);
				
		return scope.Close(String::New(newJsonData));
}



// addNumberTextbox(page, name, description, range1, range2);
// Return new Json string
Handle<Value> addNumberTextbox(const Arguments& args) {

	const char* oldJsonData;

//	char* newJsonData;
	int newJsonLength;
			
	const char* name;
	const char* description;
	const char* range1;
	const char* range2;

	HandleScope scope;
	

	if (args.Length() != 5) {
			ThrowException(Exception::TypeError(String::New("addNumberTextbox::Invalid Use : 5 arguments expected [page,name,description,range1,range2]")));
			return scope.Close(Undefined());
		}
	if (!args[0]->IsString()) { 
			ThrowException(Exception::TypeError(String::New("addNumberTextbox::1st argument is wrong [page]")));
			return scope.Close(Undefined());
	}


	if (!args[1]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("addNumberTextbox::2nd argument is wrong [string:Name]")));
			return scope.Close(Undefined());
	}

	if (!args[2]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("addNumberTextbox::3rd argument is wrong [string:description]")));
			return scope.Close(Undefined());
	}
	
	if (!args[3]->IsNumber()|| !args[4]->IsNumber() ) { 
			ThrowException(Exception::TypeError(String::New("addNumberTextbox::4,5th argument is wrong [int:range1, int:range2]")));
			return scope.Close(Undefined());
	}
	//----------------------------------------------------------------//


	
			
	//--------------------- Page Name Check -------------------------//
			
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json = std::string(*param1);
	oldJsonData = json.c_str();

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string name_c = std::string(*param2);
	name = name_c.c_str();

	v8::String::Utf8Value param11(args[2]->ToString());
	std::string des = std::string(*param11);
	description= des.c_str();
	
	v8::String::Utf8Value param3(args[3]->ToString());
	std::string range_1 = std::string(*param3);
	

	v8::String::Utf8Value param4(args[4]->ToString());
	std::string range_2 = std::string(*param4);


	if ( atof(range_1.c_str()) < atof(range_2.c_str())){
		range1 = range_1.c_str();
		range2 = range_2.c_str();
	}

	else{
		range1 = range_2.c_str();
		range2 = range_1.c_str();	
	}


	if ( check_page_name(oldJsonData) != 2){
			ThrowException(Exception::TypeError(String::New("addConfigItem argument error : first arg is not the object of config page")));
			return scope.Close(Undefined());				
	}

	if ( check_key_overlap(oldJsonData, name) == false ){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
	}
	newJsonLength = strlen(oldJsonData) + strlen(name) + strlen(range1) + strlen(range2)+ 32;
//	newJsonData = (char*)malloc(sizeof(char) * newJsonLength);
	char newJsonData [sizeof(char) * newJsonLength];
	memset(newJsonData, '\0', newJsonLength);

	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"numTB\":\"%s[%s/%s/%s]\"}" , tmpStr, name, description,range1, range2);
				
	return scope.Close(String::New(newJsonData));
}


// addSingleDialog(page, name, description, item1,2,3...);
// Return new Json string (+"sDialog":"Name[description/a/b/c/d/e]"),
Handle<Value> addSingleDialog(const Arguments& args) {

	const char* oldJsonData;

//	char* newJsonData;
	int newJsonLength;
			
	const char* name;

	char** items;

	HandleScope scope;

	int i;
	
	if (!args[0]->IsString()) { 
			ThrowException(Exception::TypeError(String::New("addSingleDialog::1st argument is wrong [page]")));
			return scope.Close(Undefined());
	}


	if (!args[1]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("addSingleDialog::2nd argument is wrong [string:Name]")));
			return scope.Close(Undefined());
	}

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json = std::string(*param1);
	oldJsonData = json.c_str();
	
	v8::String::Utf8Value param2(args[1]->ToString());
	std::string name_c = std::string(*param2);
	name = name_c.c_str();

	if ( check_page_name(oldJsonData) != 2){
		ThrowException(Exception::TypeError(String::New("addSingleDialog argument error : first arg is not the object of config page")));
		return scope.Close(Undefined());				
	}

	if (check_key_overlap(oldJsonData, name) == false){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
	}

//------------------------------------- get Items -----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------
	items = (char**) malloc(sizeof(char*) * MAXNUMBEROFITEMS);

	int itemsTotalLength=0;
	
	for( i=2; args[i]->IsString(); i++){
		v8::String::Utf8Value param3(args[i]->ToString());
		std::string item = std::string(*param3);
		items[i-2] = (char*) malloc(sizeof(char) * strlen(item.c_str()) );
		strcpy(items[i-2], item.c_str());
		itemsTotalLength += strlen(item.c_str());


		if( i > MAXNUMBEROFITEMS+1 ){
			
			ThrowException(Exception::TypeError(String::New("addSingleDialog argument error : over the maximum number of items")));
			return scope.Close(Undefined());
		}
	}

	char* itemsString = (char*) malloc( (sizeof(char) * itemsTotalLength) + 4*i );
	memset(itemsString, '\0', (sizeof(char) * itemsTotalLength) + 4*i );
	
	for( int j = 0; j < i-2; j++){
		if( strchr(items[j], '/') != NULL){ // some items contain '/' character -> cannot use that cuz it's token char

			ThrowException(Exception::TypeError(String::New("addSingleDialog::item cannot contain '/' character")));
			free(items);
			free(itemsString);
			return scope.Close(Undefined());
		}
		strcat(itemsString, items[j]);
		if(j == i-3){
			strcat(itemsString, "\0");
		}
		else{
			strcat(itemsString, "/");
		}


	}

	
	newJsonLength = strlen(oldJsonData) + strlen(name) + strlen(itemsString)+ 32;
//	newJsonData = (char*)malloc(sizeof(char) * newJsonLength);
	char newJsonData [sizeof(char) * newJsonLength];
	memset(newJsonData, '\0', newJsonLength);


	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"sDialog\":\"%s[%s]\"}" , tmpStr, name, itemsString);
	free(items);
	free(itemsString);
	return scope.Close(String::New(newJsonData));


}




// addMultipleDialog(page, name, description, item1,2,3...);
// Return new Json string (+"mDialog":"Name[description/a/b/c/d/e]")

Handle<Value> addMultipleDialog(const Arguments& args) {


		const char* oldJsonData;
	
//		char* newJsonData;
		int newJsonLength;
				
		const char* name;
		
		char** items;
	
		HandleScope scope;
	
		int i;
		
		if (!args[0]->IsString()) { 
				ThrowException(Exception::TypeError(String::New("addMultipleDialog::1st argument is wrong [page]")));
				return scope.Close(Undefined());
		}
	
	
		if (!args[1]->IsString() ) { 
				ThrowException(Exception::TypeError(String::New("addMultipleDialog::2nd argument is wrong [string:Name]")));
				return scope.Close(Undefined());
		}
	
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string json = std::string(*param1);
		oldJsonData = json.c_str();
		
		v8::String::Utf8Value param2(args[1]->ToString());
		std::string name_c = std::string(*param2);
		name = name_c.c_str();
	
		if ( check_page_name(oldJsonData) != 2){
			ThrowException(Exception::TypeError(String::New("addMultipleDialog argument error : first arg is not the object of config page")));
			return scope.Close(Undefined());				
		}
		if (check_key_overlap(oldJsonData, name) == false){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
		}
	
	
//------------------------------------- get Items -----------------------------------------------------//
//-----------------------------------------------------------------------------------------------------
	items = (char**) malloc(sizeof(char*) * MAXNUMBEROFITEMS);

	int itemsTotalLength=0;
	
	for( i=2; args[i]->IsString(); i++){
		v8::String::Utf8Value param3(args[i]->ToString());
		std::string item = std::string(*param3);
		items[i-2] = (char*) malloc(sizeof(char) * strlen(item.c_str()) );
		strcpy(items[i-2], item.c_str());
		itemsTotalLength += strlen(item.c_str());


		if( i > MAXNUMBEROFITEMS+1 ){
			
			ThrowException(Exception::TypeError(String::New("addSingleDialog argument error : over the maximum number of items")));
			return scope.Close(Undefined());

		}
			
	}

	char* itemsString = (char*) malloc( (sizeof(char) * itemsTotalLength) + 4*i );
	memset(itemsString, '\0', (sizeof(char) * itemsTotalLength) + 4*i );
	
	for( int j = 0; j < i-2; j++){
		if( strchr(items[j], '/') != NULL){ // some items contain '/' character -> cannot use that cuz it's token char
			ThrowException(Exception::TypeError(String::New("addSingleDialog::item cannot contain '/' character")));
			free(items);
			free(itemsString);
			return scope.Close(Undefined());
		}
		strcat(itemsString, items[j]);
		if(j == i-3){
			strcat(itemsString, "\0");
		}
		else{
			strcat(itemsString, "/");
		}


	}
	
	newJsonLength = strlen(oldJsonData) + strlen(name) + strlen(itemsString)+ 32;
//	newJsonData = (char*)malloc(sizeof(char) * newJsonLength);
	char newJsonData [sizeof(char) * newJsonLength];
	memset(newJsonData, '\0', newJsonLength);

	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"mDialog\":\"%s[%s]\"}" , tmpStr, name, itemsString);
	free(items);
	free(itemsString);
	return scope.Close(String::New(newJsonData));
}



// addMultipleDialog(page, name, description, flag(0||1));
// Return new Json string (+"dateDialog":"Name[flag]")
// flag 0 = no constraint , 1 = allow only after current date

Handle<Value> addDateDialog(const Arguments& args) {


	const char* oldJsonData;

	int newJsonLength;
			
	const char* name;
	const char* description;
	const char* flag;

	HandleScope scope;
	

	if (args.Length() != 4) {
			ThrowException(Exception::TypeError(String::New("addDateDialog::Invalid Use : 4 arguments expected [page,name,description, flag]")));
			return scope.Close(Undefined());
		}
	if (!args[0]->IsString()) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::1st argument is wrong [page]")));
			return scope.Close(Undefined());
	}

	if (!args[1]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::2nd argument is wrong [string:Name]")));
			return scope.Close(Undefined());
	}

	if (!args[2]->IsString() ) { 
				ThrowException(Exception::TypeError(String::New("addDateDialog::3rd argument is wrong [string:description]")));
				return scope.Close(Undefined());
	}

	if (!args[3]->IsNumber() ) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::4th argument is wrong [int 0 or 1]")));
			return scope.Close(Undefined());
	}
	//----------------------------------------------------------------//


	
			
	//--------------------- Page Name Check -------------------------//
			
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json = std::string(*param1);
	oldJsonData = json.c_str();

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string name_c = std::string(*param2);
	name = name_c.c_str();
	
	v8::String::Utf8Value param11(args[2]->ToString());
	std::string des = std::string(*param11);
	description= des.c_str();

	v8::String::Utf8Value param3(args[3]->ToString());
	std::string flag_c = std::string(*param3);
	flag = flag_c.c_str();

	if ( check_page_name(oldJsonData) != 2){
			ThrowException(Exception::TypeError(String::New("addDateDialog argument error : first arg is not the object of config page")));
			return scope.Close(Undefined());				
	}
	if (check_key_overlap(oldJsonData, name) == false){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
	}
	
	if ( atoi(flag)!=0 && atoi(flag)!=1){
		ThrowException(Exception::TypeError(String::New("addDateDialog::3rd argument is wrong [int 0 or 1]")));
		return scope.Close(Undefined());
	}

	char newJsonData [MSGBUFSIZE];
	memset(newJsonData, '\0', MSGBUFSIZE);

	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"dateDialog\":\"%s[%s/%s]\"}" , tmpStr, name, description, flag);
				
	return scope.Close(String::New(newJsonData));
}






// addTimeDialog(page, name, flag(0||1));
// Return new Json string (+"timeDialog":"Name[flag]")
// flag 0 = no constraint , 1 = allow only after current time
Handle<Value> addTimeDialog(const Arguments& args) {

	const char* oldJsonData;

	int newJsonLength;
			
	const char* name;
	const char* description;
	const char* flag;

	HandleScope scope;

	if (args.Length() != 4) {
			ThrowException(Exception::TypeError(String::New("addDateDialog::Invalid Use : 4 arguments expected [page,name,description, flag]")));
			return scope.Close(Undefined());
	}
	
	if (!args[0]->IsString()) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::1st argument is wrong [page]")));
			return scope.Close(Undefined());
	}


	if (!args[1]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::2nd argument is wrong [string:Name]")));
			return scope.Close(Undefined());
	}

	if (!args[2]->IsString() ) { 
				ThrowException(Exception::TypeError(String::New("addDateDialog::3rd argument is wrong [string:description]")));
				return scope.Close(Undefined());
	}

	if (!args[3]->IsNumber() ) { 
			ThrowException(Exception::TypeError(String::New("addDateDialog::4th argument is wrong [int 0 or 1]")));
			return scope.Close(Undefined());
	}

	//----------------------------------------------------------------//


	
			
		//--------------------- Page Name Check -------------------------//
			
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json = std::string(*param1);
	oldJsonData = json.c_str();

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string name_c = std::string(*param2);
	name = name_c.c_str();
	
	v8::String::Utf8Value param11(args[2]->ToString());
	std::string des = std::string(*param11);
	description= des.c_str();

	v8::String::Utf8Value param3(args[3]->ToString());
	std::string flag_c = std::string(*param3);
	flag = flag_c.c_str();

	if ( check_page_name(oldJsonData) != 2){
			ThrowException(Exception::TypeError(String::New("addDateDialog argument error : first arg is not the object of config page")));
			return scope.Close(Undefined());				
	}
	if (check_key_overlap(oldJsonData, name) == false){
				ThrowException(Exception::TypeError(String::New("addConfigItem argument error : the name(key) is already used in this page")));
				return scope.Close(Undefined());	
	}
	
	if ( atoi(flag)!=0 && atoi(flag)!=1){
		ThrowException(Exception::TypeError(String::New("addDateDialog::3rd argument is wrong [int 0 or 1]")));
		return scope.Close(Undefined());
	}

	char newJsonData [MSGBUFSIZE];
	memset(newJsonData, '\0', MSGBUFSIZE);

	char tmpStr[strlen(oldJsonData)];
	strncpy(tmpStr, oldJsonData, strlen(oldJsonData)-1 );
	tmpStr[strlen(oldJsonData)-1] = '\0';

	sprintf(newJsonData, "%s,\"timeDialog\":\"%s[%s/%s]\"}" , tmpStr, name, description, flag);
				
	return scope.Close(String::New(newJsonData));
}






/*//SEND Format to Android
	{
	"conf":"Pid",
	"rqID":"_rqID",
	"strTB":"Name[StrTextBoxDialog/10]",
	"numTB":"Age[NumberTextBoxDialog/0/100]",
	"sDialog":"Gender[SingleDialog/Male/Female]",
	"mDialog":"Hobby[MultiDialog/Book/Movie/Tennis/Jogging]",
	"dateDialog":"Birthday[DateDialog/0]",
	"timeDialog":"time[TimeDialog/0]"	
	}

*/
Handle<Value> sendConfigPage(const Arguments& args) {
	HandleScope scope;
	requestList* rl;
	DBusMessage* msg;
	DBusError err;

	dbus_int32_t pid;
	dbus_int32_t rq_num; 

	const char* jsonData;

	//----------------------------------------------------------------//
	//						1. Argument Check
	if ((args.Length() != 2) ||	!args[1]->IsFunction() || !args[0]->IsString() ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 2 arguments expected [ConfigPage, Function]")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//


	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json_c = std::string(*param1);
	jsonData = json_c.c_str();
	
			
	if ( check_page_name(jsonData) != 2){
			ThrowException(Exception::TypeError(String::New("sendConfig argument error : first arg is not the object of config page")));
			return scope.Close(Undefined());				
	}

	

	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)

	rl = newRequest(rList);
	rl->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
	rl->type = APP_REQUEST;
	
	pid = (unsigned int)getpid();
	rq_num = rl->rq_num;

	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);

	msg = dbus_message_new_signal(SAM_INTERFACE, SAM_PATH, "sendConfigPage");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_STRING, &jsonData,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] SendConfigPage >>  %s \n", jsonData);
	//
	//----------------------------------------------------------------//

	return scope.Close(Undefined());
}








// getData(page[return page], name);
// Return name(key)->value [string]
Handle<Value> getData(const Arguments& args) {

	const char* rcvJson;
	const char* name;

	HandleScope scope;
	

	if (args.Length() != 2) {
			ThrowException(Exception::TypeError(String::New("getData::Invalid Use : 2 arguments expected [configurableData,Key]")));
			return scope.Close(Undefined());
		}
	if (!args[0]->IsString()) { 
			ThrowException(Exception::TypeError(String::New("getData::1st argument is wrong [page]")));
			return scope.Close(Undefined());
	}

	if (!args[1]->IsString() ) { 
			ThrowException(Exception::TypeError(String::New("getData::2nd argument is wrong [string:Name]")));
			return scope.Close(Undefined());
	}

	//----------------------------------------------------------------//


	
			
	//--------------------- Page Name Check -------------------------//
			
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string json = std::string(*param1);
	rcvJson = json.c_str();

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string name_c = std::string(*param2);
	name = name_c.c_str();
	
	int position = 0;
	char key[512];
	char value[512];
	memset(key, '\0', 512);
	memset(value, '\0', 512);
	
	for(unsigned int i=0; i<strlen(rcvJson); i++){

		if ( rcvJson[i] == '{' )
			continue;

		else if ( rcvJson[i] == '}' ){
			if(!strcmp(key, name)){

				return scope.Close(String::New(value));
			}
		}
		
		else if ( rcvJson[i] == ':' ){
			if( rcvJson[i+1] == '"'){
				position++;
			}
			else{
				char tmp[2];
				tmp[1] = '\0';
				tmp[0] = rcvJson[i];			
			
				if(! (position % 2)) // even num -> key
					strcat(key, tmp);
				else				// odd num -> value
					strcat(value, tmp);
			}
		}

		else if ( rcvJson[i] == ',' ){
			if( rcvJson[i+1] == '"'){
				position++;

				if(!strcmp(key, name)){
					
					return scope.Close(String::New(value));
				}
				else{
					
					memset(key, '\0', 512);
					memset(value, '\0', 512);	
				}
			}
		}

		else if ( rcvJson[i] == '"' ){
			continue;
		}

		else{

			char tmp[2];
			tmp[1] = '\0';
			tmp[0] = rcvJson[i];			
			
			if(! (position % 2)) // even num -> key
				strcat(key, tmp);
			else				// odd num -> value
				strcat(value, tmp);
		}
	}
	
	
	printf("[NIL] There is no value with the key[%s]\n", name);			
	return scope.Close(String::New("N/A"));
}
// [MORE] return multiple choice -> array








//onTermination(function)
Handle<Value> onTermination(const Arguments& args) {
	HandleScope scope;
	requestList* rl;
	DBusMessage* msg;
	DBusError err;

	dbus_int32_t pid;
	dbus_int32_t rq_num; 

	//----------------------------------------------------------------//
	//						1. Argument Check
	if ((args.Length() != 1) ||	!args[0]->IsFunction()) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [Callback Function]")));
		return scope.Close(Undefined());
	}
	//
	//----------------------------------------------------------------//



	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)

	rl = newRequest(rList);
	rl->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));
	rl->type = APP_REQUEST;
	
	pid = (unsigned int)getpid();
	rq_num = rl->rq_num;

	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);

	msg = dbus_message_new_signal(SAM_INTERFACE, SAM_PATH, "onTermination");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_INT32, &rq_num,
		DBUS_TYPE_INVALID);


	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);

	printf("[NIL] send message to %s, pid : %d, rq_num : %d\n", SAM_PATH, pid, rq_num);
	//
	//----------------------------------------------------------------//

	return scope.Close(Undefined());
}



Handle<Value> faceRecognitionWithNoti(const Arguments& args) {

	HandleScope scope;

	const char* imgInputPath;
	
	DBusMessage* msg;
	DBusError err;

	dbus_int32_t pid;

	
	//----------------------------------------------------------------//
	//						1. Argument Check
	if ((args.Length() != 1) ||	!args[0]->IsString() ) {
		ThrowException(Exception::TypeError(String::New("Invalid Use : 1 arguments expected [image]")));
		return scope.Close(Undefined());
	}

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string imageInputPath = std::string(*param1);
	imgInputPath = imageInputPath.c_str();

	
	//
	//----------------------------------------------------------------//


	//----------------------------------------------------------------//
	//			2. Request Creation (For function callback)


	pid = (unsigned int)getpid();

	//
	//----------------------------------------------------------------//
	

	//----------------------------------------------------------------//
	//				3. Send Message (Request struct) 
	//				Send message with reply or not
	dbus_error_init(&err);
	msg = dbus_message_new_signal(SAM_INTERFACE, SAM_PATH, "face_recognition");

	dbus_message_append_args(msg,
		DBUS_TYPE_INT32, &pid,
		DBUS_TYPE_STRING, &imgInputPath,
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(opelCon, msg, NULL);
	dbus_message_unref(msg);


	//
	//----------------------------------------------------------------//

	return scope.Close(Undefined());
}



void init(Handle<Object> exports) {
	nativeInterfaceLayerInit();
	opelCon = DbusInit(); //Init Dbus message receiver (by PID)


//-----------------------EventPage-------------------------------------------
	exports->Set(String::NewSymbol("makeEventPage"),
			FunctionTemplate::New(makeEventPage)->GetFunction());
	exports->Set(String::NewSymbol("addEventText"),
				FunctionTemplate::New(addEventText)->GetFunction());
	exports->Set(String::NewSymbol("addEventImg"),
				FunctionTemplate::New(addEventImg)->GetFunction());
	exports->Set(String::NewSymbol("sendEventPageWithNoti"),
					FunctionTemplate::New(sendEventPageWithNoti)->GetFunction());
	exports->Set(String::NewSymbol("sendEventPage"),
					FunctionTemplate::New(sendEventPage)->GetFunction());


//-----------------------ConfigPage-------------------------------------------

	exports->Set(String::NewSymbol("makeConfigPage"),
			FunctionTemplate::New(makeConfigPage)->GetFunction());
	exports->Set(String::NewSymbol("addStrTextbox"),
		FunctionTemplate::New(addStrTextbox)->GetFunction());
	exports->Set(String::NewSymbol("addNumberTextbox"),
		FunctionTemplate::New(addNumberTextbox)->GetFunction());
	exports->Set(String::NewSymbol("addSingleDialog"),
			FunctionTemplate::New(addSingleDialog)->GetFunction());
	exports->Set(String::NewSymbol("addMultipleDialog"),
			FunctionTemplate::New(addMultipleDialog)->GetFunction());
	exports->Set(String::NewSymbol("addDateDialog"),
			FunctionTemplate::New(addDateDialog)->GetFunction());
	exports->Set(String::NewSymbol("addTimeDialog"),
			FunctionTemplate::New(addTimeDialog)->GetFunction());
	exports->Set(String::NewSymbol("sendConfigPage"),
			FunctionTemplate::New(sendConfigPage)->GetFunction());

	exports->Set(String::NewSymbol("getData"),
			FunctionTemplate::New(getData)->GetFunction());

	
//------------------------termination-----------------------------------------
	exports->Set(String::NewSymbol("onTermination"),
			FunctionTemplate::New(onTermination)->GetFunction());


//------------------------cloud service-----------------------------------------
exports->Set(String::NewSymbol("faceRecognitionWithNoti"),
		FunctionTemplate::New(faceRecognitionWithNoti)->GetFunction());


}



NODE_MODULE(nil, init)
