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

using namespace v8;

#ifndef __NATIVE_INTERFACE_LAYER_H__
#define __NATIVE_INTERFACE_LAYER_H__

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

typedef struct _RequestList{
	int rq_num;						//Request number
	int type;						//Request Type, SENSOR_MANAGER//CAMERA_MANAGER
	Persistent<Function> callback;	//callback function
	void *data;						//User data
	struct _RequestList* next;				//Linked list
}RequestList;

static char nilPath[100];
static char nilInterface[100];
static int num_of_list = 3;
//static RequestList RequestList;
static Persistent<Function> callback;

static const char* SUPPORT_LIST[] = {
	"ACC",
	"MOTION",
	"TEMP"
};

#endif  

gboolean NIL_DbusConnection(DBusConnection *bus_cnxn);
void pid_to_char(char* output);
int check_sensor_name(const char* name);
DBusConnection *DbusInit(void);

void initRequestList(RequestList *rList);
void printRequset(RequestList *rList);
int countRequest(RequestList *rList);
int deleteRequset(RequestList *rList, int rq_num);
RequestList * newRequest(RequestList *rList);
RequestList * getRequest(RequestList *rList, int rq_num);

DBusHandlerResult configEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data);
DBusHandlerResult terminationEventDriven(DBusConnection *connection, DBusMessage *message, void *iface_user_data);
