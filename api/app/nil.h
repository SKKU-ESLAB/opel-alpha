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


//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/msg.h>
//#include <unistd.h>
//#include <signal.h>
//#include <iostream>

using namespace v8;
//using namespace node;

#ifndef __NATIVE_INTERFACE_LAYER_H__
#define __NATIVE_INTERFACE_LAYER_H__

//------------------------------------------------------------------------------------//
//								 Sensor Manager Structure                            //
#define MAX_REQUEST 20

typedef struct request {
	char sensor_name[20];
	int sensor_type;
	int id;
	int handle_type;
	int sensing_interval;
	int sensing_level;
	Persistent<Function> callback;

	int data; //Data
} RequestData;


typedef struct _requestList{
	int rq_num;						//Request number
	int type;						//Request Type, SENSOR_MANAGER//CAMERA_MANAGER
	Persistent<Function> callback;	//callback function
	void *data;						//User data
	struct _requestList* next;				//Linked list
}requestList;

static char nilPath[100];
static char nilInterface[100];
static int num_of_list = 3;
//static RequestList requestList;
static Persistent<Function> callback;

static const char* SUPPORT_LIST[] = {
	"ACC",
	"MOTION",
	"TEMP"
};
//
//------------------------------------------------------------------------------------//


#endif  





gboolean NIL_DbusConnection(DBusConnection *bus_cnxn);
void pid_to_char(char* output);
int check_sensor_name(const char* name);
DBusConnection *DbusInit(void);

void initRequestList(requestList *rList);
void printRequset(requestList *rList);
int countRequest(requestList *rList);
int deleteRequset(requestList *rList, int rq_num);
requestList * newRequest(requestList *rList);
requestList * getRequest(requestList *rList, int rq_num);

DBusHandlerResult configEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data);
DBusHandlerResult terminationEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data);