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

#include "nil.h"

typedef DBusMessage * (*OpelDbusFunction) (DBusConnection *connection, DBusMessage *message, void *user_data);

#define GDBUS_ARGS(args...) (const GDBusArgInfo[]) { args, { } }
#define DBUS_NEED_REPLY		1
#define DBUS_NEED_NO_REPLY	0


/*
#define DBUS_METHOD(_name, _reply, _function, _in_args, _out_args) \
	.name = _name, \
	.reply = _reply, \
	.function = _function,
	.in_args = _in_args, \
	.out_args = _out_args \
	
struct GDBusArgInfo {
	const char *name;
	const char *signature;
};

struct DbusTable {
	const char			*name;
	bool				reply;
	DbusFunction		function;
	const GDBusArgInfo	*in_args;
	const GDBusArgInfo	*out_args;
};

static const DbusTable properties_methods[] = {
	{ DBUS_METHOD("SensorEventNotify",
					DBUS_NEED_NO_REPLY,
					SensorEventNotify,
					GDBUS_ARGS({ "value1", "s" }, { "value2", "s" }),
					NULL) 
	},					
	{ DBUS_METHOD("SensorData",
					DBUS_NEED_REPLY,
					SensorEventNotify,
					GDBUS_ARGS({ "value1", "s" }, { "value2", "s" }),
					GDBUS_ARGS({ "value", "v" }) ) 
	},	
	{ DBUS_METHOD("Exit",
					DBUS_NEED_NO_REPLY,
					SensorEventNotify,
					GDBUS_ARGS({ "value1", "s" }, { "value2", "s" }),
					GDBUS_ARGS({ "value", "v" })) 
	}
};
*/

/*
	introspect는 따로 처리.
	xml 만드는 함수 추가
	nil에서 dbus bus 어떻게 하는지 보기 
		기본 : org.opel
		프로세스 : org.opel.PID
		매니저 org.opel.sensorManager
*/




static DBusHandlerResult dbus_respone(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	int i;

	printf("[NIL] Message received [Interface:%s]\n", nilInterface);
	/*
	for (i = 0; properties_methods[i] != null; i++){
	printf("test, %d\n", i);

	if (dbus_message_is_signal(message, module_interface, properties_methods[i].name){
	properties_methods[i].function(); //add args
	return DBUS_HANDLER_RESULT_HANDLED;
	}
	}
	*/
	//callJavaFunc

	if (dbus_message_is_signal(message, nilInterface, "config_event_driven")){
			printf("[NIL] config_event_driven Call");
	
			return configEventDriven(connection, message, user_data);
	}


	if (dbus_message_is_signal(message, nilInterface, "termination_event_driven")){
			printf("[NIL] termination_event_driven Call");
	
			return terminationEventDriven(connection, message, user_data);
	}
	

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void pid_to_char(char* output){
	pid_t pid = getpid();

	unsigned int value = (unsigned int)pid;
	unsigned int temp;

	output[5] = NULL;

	for (int i = 4; i >= 0; i--){
		temp = value % 10; //1의 자리
		output[i] = ((char)(temp)) + 65;
		value = value / 10;
	}


}

DBusConnection *DbusInit() {
	DBusConnection *connection;
	DBusError error;
	dbus_error_init(&error);

	char total_address[100];
	const char * address1 = "type='signal', interface='org.opel.";
	char address2[6];
	const char * address3 = "'";

	printf("[Module] Start dbus initializing \n");
	pid_to_char(address2);

	sprintf(total_address, "%s%s%s", address1, address2, address3);
	printf("Total address : %s \n", total_address);

	sprintf(nilPath, "/org/opel/%s", address2);
	sprintf(nilInterface, "org.opel.%s", address2);

	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

	if (dbus_error_is_set(&error))
	{
		printf("Error connecting to the daemon bus: %s", error.message);
		dbus_error_free(&error);
	}

	dbus_connection_set_exit_on_disconnect(connection, FALSE);

	if (!NIL_DbusConnection(connection)){
		printf("Failt to initized connection \n");

	}
	dbus_bus_add_match(connection, total_address, NULL);  //Dbus 위치 설정	
	dbus_connection_flush(connection);

	dbus_connection_add_filter(connection, dbus_respone, NULL, NULL); //Register dbus message handling fuction "dbus_respone"

	

	printf("Dbus initializing complete \n");
	
	return connection;
}	

