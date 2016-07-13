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

//#define GDBUS_ARGS(args...) (const GDBusArgInfo[]) { args, { } }
#define DBUS_NEED_REPLY		1
#define DBUS_NEED_NO_REPLY	0

static DBusHandlerResult dbus_respone(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	if (dbus_message_is_signal(message, nilInterface, SEND_SENSOR_DATA)){
		//printf("[NIL] sensor_event_driven sensor data");

		return sensorEventDriven(connection, message, user_data);
	}
	if (dbus_message_is_signal(message, nilInterface, SEND_SENSOR_NOTIFY)){
		//printf("[NIL] sensor_event_driven notify");

		return sensorEventNotify(connection, message, user_data);
	}
	
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
void pid_to_char(char* output){
	pid_t pid = getpid();

	unsigned int value = (unsigned int)pid;
	unsigned int temp;

	output[5] = NULL;

	for (int i = 4; i >= 0; i--){
		temp = value % 10; //1�� �ڸ�
		output[i] = ((char)(temp)) + 65;
		value = value / 10;
	}

	printf("Convert : from %u to %s \n", (unsigned int)pid, output);
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
	dbus_bus_add_match(connection, total_address, NULL);  //Dbus ��ġ ����	
	dbus_connection_flush(connection);

	dbus_connection_add_filter(connection, dbus_respone, NULL, NULL); //Register dbus message handling fuction "dbus_respone"

	

	printf("Dbus initializing complete \n");
	
	return connection;
}	

