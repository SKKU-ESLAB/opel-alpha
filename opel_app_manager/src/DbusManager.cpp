#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <stdbool.h>
#include <pthread.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "DbusManager.h"
#include "appStatusManager.h"
#include "globalData.h"
#include "commManager.h"
#include "jsonString.h"

static DBusConnection *connection;

static char* temp_jsonData;		




			
static DBusHandlerResult rcvNotiPage(DBusConnection *connection, DBusMessage *message, void *iface_user_data){


	int pid;
	char* inputJsonData;

	time_t now;
	struct tm t;
	char dateTime[128]={'\0',};

	char resultJson[MSGBUFSIZE]={'\0',};
	char tmpJson[MSGBUFSIZE]={'\0',};

	int isPageWithNoti;
	dbus_message_get_args(message, NULL, DBUS_TYPE_INT32, &pid, DBUS_TYPE_STRING, &inputJsonData, DBUS_TYPE_INT32, &isPageWithNoti, DBUS_TYPE_INVALID);

	time(&now);
	t = *localtime(&now);

	int i=0;
	while(!appProcessTable::getInstance()->isExistOnRunningTableByPid(pid)){
			i++;
			usleep( 1000 *100 );

			if(i == 100){
				printf("[DbusManager]:rcvOnTermination >> ERROR!!\n");
				return DBUS_HANDLER_RESULT_HANDLED;
			}
	}

	// data : appName, appId, current date/time 
	appProcessInfo appProc;
	if(  ! appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)  ){
		printf("[DbusManager] Cannot find the procInfo >> pid : %d\n", pid);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
		
	
	char* appName = appProc.getAppProcName();
	char* appId = appProc.getAppProcId();

	sprintf(dateTime, "%d/%02d/%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,	t.tm_hour, t.tm_min, t.tm_sec);

	//cut head of json ("{"noti":"noti",")
	strncpy (tmpJson, inputJsonData + 15, strlen(inputJsonData)-15);
	sprintf(resultJson, "{\"type\":\"noti\",\"appID\":\"%s\",\"appTitle\":\"%s\",\"dateTime\":\"%s\",\"isNoti\":\"%d\",%s", appId, appName, dateTime, isPageWithNoti,tmpJson);
	
	if(comManager::getInstance()->response_Dbus_SendNoti(resultJson)){
		printf("[DbusManager] SendNoti Success >> json : %s\n", resultJson);
	}
	else{
		printf("[DbusManager] SendNoti Fail >> json : %s\n", resultJson);
	}
	

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult rcvConfigPage(DBusConnection *connection, DBusMessage *message, void *iface_user_data){

	int pid, rq_num;
	char* inputJsonData;

	char* appId;

	char resultJson[MSGBUFSIZE]={'\0',};
	char tmpJson[MSGBUFSIZE]={'\0',};
	
	dbus_message_get_args(message, NULL, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &rq_num, DBUS_TYPE_STRING, &inputJsonData, DBUS_TYPE_INVALID);

	int i=0;
	while(!appProcessTable::getInstance()->isExistOnRunningTableByPid(pid)){
			i++;
			usleep( 1000 *100 );

			if(i == 100){
				printf("[DbusManager]:rcvSendConfig >> ERROR!!\n");
				break;
			}
	}

	// data : appName, appId, current date/time 

	appProcessInfo appProc;
	if(  ! appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)  ){
		printf("[DbusManager] Cannot find the procInfo >> pid : %d\n", pid);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	
	char* appName = appProc.getAppProcName();		
	appId = appProc.getAppProcId();

	//cut head of json ("{"noti":"noti",")

	strncpy (tmpJson, inputJsonData + 15, strlen(inputJsonData)-15) ;

	sprintf(resultJson, "{\"conf\":\"conf\",\"appID\":\"%s\",\"rqID\":\"%d\",\"pid\":\"%d\",%s", appId, rq_num, pid, tmpJson);


	if(comManager::getInstance()->response_Dbus_SendConfig(resultJson) ){
		printf("[DbusManager] SendConfig Success >> json : %s\n", resultJson);
	}
	else{
		printf("[DbusManager] SendConfig Fail >> json : %s\n", resultJson);
	}


	
	return DBUS_HANDLER_RESULT_HANDLED;
}	


static DBusHandlerResult rcvOnTermination(DBusConnection *connection, DBusMessage *message, void *iface_user_data){

	int pid, rq_num;
	char resultJson[MSGBUFSIZE] = {'\0',};

	char appId[16] = {'\0',};


	dbus_message_get_args(message, NULL, DBUS_TYPE_INT32, &pid, DBUS_TYPE_INT32, &rq_num, DBUS_TYPE_INVALID);
		

	printf("[DbusManager] rcvOntermination >> rqNum : %d pid : %d\n", rq_num, pid);

//	appProcessTable::getInstance()->findProcessByAppPid(pid);

	int i=0;
	while(!appProcessTable::getInstance()->isExistOnRunningTableByPid(pid)){
		i++;
		usleep( 1000 *100 );
//		printf("[DbusManager]:rcvOnTermination >> in loop\n");
		if(i == 10){
			printf("[DbusManager]:rcvOnTermination >> ERROR!!\n");
			break;
		}
	}
	
	// data : appName, appId, current date/time 
	char appName[1024] = {'\0',};


	appProcessInfo appProc;
	if(  ! appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)  ){
		printf("[DbusManager] Cannot find the procInfo >> pid : %d\n", pid);
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	
	strcpy(appName,appProc.getAppProcName());
	strcpy(appId,appProc.getAppProcId());

	sprintf(resultJson, "{\"type\":\"%s\",\"appID\":\"%s\",\"appName\":\"%s\",\"rqID\":\"%d\",\"pid\":\"%d\"}", NIL_TERMINATION, appId, appName, rq_num, pid);

	printf("[DbusManager] send terminationJson data to Android[termination] : %s \n", resultJson);

	comManager* cm;
	cm = comManager::getInstance();
	cm->response_Dbus_RegisterTermination(resultJson);
	
	return DBUS_HANDLER_RESULT_HANDLED;
}	



static DBusHandlerResult rcvSensorInfo(DBusConnection *connection, DBusMessage *message, void *iface_user_data){


	char* inputJsonData;

	dbus_message_get_args(message, NULL,  DBUS_TYPE_STRING, &inputJsonData, DBUS_TYPE_INVALID);
		
	
	printf("[DbusManager] rcv Sensor Data data sensorInfo : %s \n", inputJsonData);


	// [MORE] send inputJsonData to Android!!
	
	return DBUS_HANDLER_RESULT_HANDLED;
}	

static DBusHandlerResult rcvCameraInfo(DBusConnection *connection, DBusMessage *message, void *iface_user_data){


	char* inputJsonData;

	dbus_message_get_args(message, NULL,  DBUS_TYPE_STRING, &inputJsonData, DBUS_TYPE_INVALID);
		
	
	printf("[DbusManager] rcv Camera data cameraInfo : %s \n", inputJsonData);


	// [MORE] send inputJsonData to Android!!
	
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult rcvFaceRecognition(DBusConnection *connection, DBusMessage *message, void *iface_user_data){


	int pid;
	char* inputImgPath;

	char resultJson[MSGBUFSIZE]={'\0',};
	char tmpJson[MSGBUFSIZE]={'\0',};

	dbus_message_get_args(message, NULL, DBUS_TYPE_INT32, &pid, DBUS_TYPE_STRING, &inputImgPath, DBUS_TYPE_INVALID);

	int i=0;
	while(!appProcessTable::getInstance()->isExistOnRunningTableByPid(pid)){
			i++;
			usleep( 1000 *100 );

			if(i == 100){
				printf("[DbusManager]:TimeOver >> ERROR!!\n");
				return DBUS_HANDLER_RESULT_HANDLED;
			}
	}

	// data : appName, appId, current date/time 
	appProcessInfo appProc;
	if(  ! appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)  ){
		printf("[DbusManager] Cannot find the procInfo >> pid : %d\n", pid);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
		
	char* appName = appProc.getAppProcName();
	char* appId = appProc.getAppProcId();


	sprintf(resultJson, "{\"appID\":\"%s\",\"appTitle\":\"%s\",\"imgPath\":\"%s\"}", appId, appName, inputImgPath );
	
	if(comManager::getInstance()->response_Dbus_FaceRecognition(resultJson)){
		printf("[DbusManager] Send FaceRecognition Success >> json : %s\n", resultJson);
	}
	else{
		printf("[DbusManager] Send FaceRecognition Fail >> json : %s\n", resultJson);
	}
	

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_respone(DBusConnection *connection, DBusMessage *message, void *user_data)
{	


	//NIL -> Android
	if (dbus_message_is_signal(message, OPEL_BUS_NAME, "sendNotiPage")){
    	printf("[DbusManager] rcv NotiPage from sendNotiPage function\n");		
		return rcvNotiPage(connection, message, user_data);
	}
		
	else if (dbus_message_is_signal(message, OPEL_BUS_NAME, "sendConfigPage")){
		printf("[DbusManager] rcv Config from sendConfigPage function \n");
		return rcvConfigPage(connection, message, user_data);
	}

	else if (dbus_message_is_signal(message, OPEL_BUS_NAME, "onTermination")){
		printf("[DbusManager] rcv termination from onTermination function \n");
		return rcvOnTermination(connection, message, user_data);
	}

	else if (dbus_message_is_signal(message, OPEL_BUS_NAME, "sendSensorInfo")){
		printf("[DbusManager] rcv sensorData from sensorManager\n");
		return rcvSensorInfo(connection, message, user_data);
	}

	else if (dbus_message_is_signal(message, OPEL_BUS_NAME, "sendCameraInfo")){
		printf("[DbusManager] rcv cameraData from cameraManager\n");
		return rcvCameraInfo(connection, message, user_data);
	}
	else if (dbus_message_is_signal(message, OPEL_BUS_NAME, "face_recognition")){
			printf("[DbusManager] rcv face_recognition from NIL\n");
			return rcvFaceRecognition(connection, message, user_data);
	}

	else{
		printf("[DbusManager] Unexpected message received from %s\n", dbus_message_get_interface(message));
	}
	
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


		

DbusManager::DbusManager(){
	//pthread_t re;
	//pthread_create(&re, NULL, DbusManager::test_event_driven, this);

	//	appProcList = appProcessTable::getInstance();

	pthread_t re;
	pthread_create(&re, NULL, DbusManager::initDbus, this);
//	initDbus();
}



void DbusManager::sendTerminationToCameraManager(){
	char * path_address = "org.opel.camera.daemon";
	char * interface_address = "/org/opel/camera/daemon";
	DBusConnection* conn;
	
	DBusError err;
	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(!conn){
		dbus_error_free(&err);
		return ;
	}
	else{
		DBusMessage* msg;
		msg = dbus_message_new_signal(interface_address, path_address, "recStop");
		dbus_connection_send(conn, msg, NULL);
		dbus_message_unref(msg);

	}
}




//CONFIG JSON
/*{ "conf":"_rq_num",
	"Name1","111",
	"Name2","222/222",
	"Name3","333"

// }
*/


//INPUT : destPID (D-bus Interface), rqNum(registered rq num),
//			jsonMsg, typeOfSignal(what kind of signal)
//Need to get pid(from appId) and rq_num from json string
//remove appId info of json string
void DbusManager::sendMsg(jsonString js, char* typeOfSignal){ //"termination_event_driven"

	char pid_path[100]={'\0',};		
	char pid_interface[100]={'\0',};
	char pid_char[6]={'\0',};
	char * path_address = "org.opel.";
	char * interface_address = "/org/opel/";
	int sensorValue = 10;
	DBusMessage* msg;
	DBusError err;
	dbus_error_init(&err);

	char destPid[128] = {'\0',};
	strcpy(destPid, js.findValue("pid"));


	char rqID[128] = {'\0',};
	strcpy(rqID, js.findValue("rqID"));

	dbus_int32_t _rq_num = atoi(rqID); 

	pid_to_char(atoi(destPid), pid_char); 
	sprintf(pid_path, "%s%s", path_address, pid_char);
	sprintf(pid_interface, "%s%s", interface_address, pid_char);

	dbus_error_init(&err);
	printf("pid Path : %s \n pid interface : %s\n\n", pid_interface, pid_path);

	msg = dbus_message_new_signal(pid_interface, pid_path, typeOfSignal);

	char* json = js.getJsonData();

	dbus_message_append_args(msg, DBUS_TYPE_INT32, &_rq_num, DBUS_TYPE_STRING, 
							&json, DBUS_TYPE_INVALID);


	printf("[DbusManager] send Dbus Signal pid_path : %s, pid_interface : %s, send rq_num :%d\n send json : %s", pid_path, pid_interface, _rq_num, json);	

	if (dbus_error_is_set(&err))
	{
		printf("[DbusManager] Error get data: %s", err.message);
		dbus_error_free(&err);
	}


	dbus_connection_send(connection, msg, NULL);
	dbus_message_unref(msg);

}


bool DbusManager::makeTerminationEvent(jsonString js){

	printf("\n\n DBUG : send Dbus Termination signal : %s\n\n", js.getJsonData());
	sendMsg(js, TERMINATION_SIGNAL);
	return true;
}
bool DbusManager::makeConfigEvent(jsonString js){

	printf("\n\n DBUG : send Dbus Config signal : %s\n\n", js.getJsonData());
	sendMsg(js, CONFIGURATION_SIGNAL);
	return true;
}


void *DbusManager::initDbus(void* data){

//void DbusManager::initDbus(){
	DBusError error;
	GMainLoop *loop;
	int retval;

	printf("[DbusManager] Start dbus initializing \n");


	loop = g_main_loop_new(NULL, FALSE);   

	dbus_error_init(&error);
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error); //DBUS_BUS_SESSION


	if (dbus_error_is_set(&error))
	{
		printf("Error connecting to the daemon bus: %s", error.message);
		dbus_error_free(&error);
	}

	retval = dbus_bus_request_name(connection, OPEL_BUS_NAME, DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &error);
	dbus_connection_flush(connection);

	switch (retval) {
		case -1: {
			g_printerr("Couldn't acquire name %s for our connection: %s\n", OPEL_BUS_NAME, error.message);
			g_printerr("This program may not terminate as a result of this error!\n");
			dbus_error_free(&error);
			break;
		}
		case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
			g_printerr("dbus_bus_request_name(): We now own the name %s !\n", OPEL_BUS_NAME);
			break;
		case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
			g_printerr("dbus_bus_request_name(): We are standing in queue for our name!\n");
			break;
		case DBUS_REQUEST_NAME_REPLY_EXISTS:
			g_printerr("dbus_bus_request_name(): :-( The name we asked for already exists!\n");
			break;
		case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
			g_printerr("dbus_bus_request_name(): Eh? We already own this name!\n");
			break;
		default:
			g_printerr("dbus_bus_request_name(): Unknown result = %d\n", retval);
	}


	dbus_bus_add_match(connection, "type='signal',interface='org.opel.sysAppManager'", NULL);  
	dbus_connection_flush(connection);
	printf("[DbusManager] DBUS Respose ready\n");
	dbus_connection_add_filter(connection, dbus_respone, loop, NULL); 


	dbus_connection_setup_with_g_main(connection, NULL);
	g_main_loop_run(loop);
	
}

void DbusManager::pid_to_char(unsigned int input, char* output){
	unsigned int value = input;
	unsigned int temp;
	int i;
	output[5];

	for (i = 4; i >= 0; i--){
		temp = value % 10; 
		output[i] = ((char)(temp)) + 65;
		value = value / 10;
	}
}


void DbusManager::Sensor_all_request_unregister(int pid){
   DBusMessage* msg;
   DBusError err;
   
   int rq_num;
   
   rq_num = -1;

   dbus_error_init(&err);
   msg = dbus_message_new_signal(SM_INTERFACE, SM_PATH, "unregister");

   dbus_message_append_args(msg,
      DBUS_TYPE_INT32, &pid,
      DBUS_TYPE_INT32, &rq_num,
      DBUS_TYPE_INVALID);

   /* Send the signal */
   dbus_connection_send(connection, msg, NULL);
   dbus_message_unref(msg);

   printf("[NIL] send All_UNREGISTER message to %s | %s\n", SM_PATH, SM_INTERFACE);
}







