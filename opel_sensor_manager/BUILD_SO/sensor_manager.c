#include "sensor_manager.h"
#include "devices.h"
#include <pthread.h>
#include <unistd.h>

#define OPEL_INTERFACE "org.opel.sensorManager"

#define VALUE_CHANGED	1
#define VALUE_UNCHANGED	2

#define SENSING_INTERVAL		1	
#define SENSING_EVENT_DRIVEN	2
#define SENSING_ONESHOT			3

#define SEND_SENSOR_DATA		"sensorData"
#define SEND_SENSOR_NOTIFY		"sensorNotify"

DBusConnection *connection;
static sensorHead *sensor_head_main;

void pid_to_char(unsigned int input, char* output){
	unsigned int value = input;
	unsigned int temp;
	int i;
	output[5] = '\0'; //NULL

	for (i = 4; i >= 0; i--){
		temp = value % 10; //1???�리
		output[i] = ((char)(temp)) + 65;
		value = value / 10;
	}
	//printf("Convert : from %u to %s \n", (unsigned int)input, output);
}
char* getInterface(sensorList* sl){
	return sl->rh->start->target_interface;
}
char* getPath(sensorList* sl){
	return sl->rh->start->target_path;
}
void makeDbusAddress(requestData* rd){
	unsigned int pid = rd->pid;
	char pid_char[6];

	char * ople_path = "/org/opel/";
	char * ople_interface = "org.opel.";

	pid_to_char(pid, pid_char);

	sprintf(rd->target_path, "%s%s", ople_path, pid_char);
	sprintf(rd->target_interface, "%s%s", ople_interface, pid_char);

	//printf("make adress %s / %s \n", rd->target_path, rd->target_interface);
}

struct timespec getSleepTime(requestHeader* rh){
	struct timeval cur_time;
	struct timeval sleep_time;
	struct timespec next_time;
	int one_second = 1000000;
	int one_second_nano = 1000000000;

	gettimeofday(&cur_time, NULL);

	sleep_time.tv_sec = (rh->start->next_run_time.tv_sec - cur_time.tv_sec);
	sleep_time.tv_usec = (rh->start->next_run_time.tv_usec - cur_time.tv_usec);

	if (sleep_time.tv_sec > 0 &&
		sleep_time.tv_usec < 0){
		sleep_time.tv_usec += 1000000;
		sleep_time.tv_sec--;
	}
	else if (sleep_time.tv_sec <= 0 &&
		sleep_time.tv_usec < 0){
		sleep_time.tv_usec = 0;
		sleep_time.tv_sec = 0;

		printf("Sensing interval is very small, need to adjust\n");
	}	
	//printf("Current time : %d.%6d", cur_time.tv_sec, cur_time.tv_usec);
	//printf("Target  Time : %d.%6d\n", rh->start->next_run_time.tv_sec, rh->start->next_run_time.tv_usec);
	//printf("  Sleep time : %2d.%6d\n", sleep_time.tv_sec, sleep_time.tv_usec);

	next_time.tv_nsec = (cur_time.tv_usec + sleep_time.tv_usec) * 1000; //Change from msec to usec
	next_time.tv_sec = cur_time.tv_sec + sleep_time.tv_sec;

	if (next_time.tv_nsec >= one_second_nano){
		next_time.tv_sec++;
		next_time.tv_nsec -= one_second_nano;
	}
	
	return next_time;
}
int sensorDataParsing(sensorList* sl, char* input, char* type){
	int event_check = 0;
	requestData* rd = sl->rh->start;

//	printf("[SM_DEBUG] notify prev value : %s, cur : %s\n", rd->prev_sensor_data, rd->sensor_data);
	
	if (rd->prev_sensor_data != NULL){
		if (strcmp(rd->prev_sensor_data, rd->sensor_data))
			event_check = 1;


		if (!strcmp("0\n", rd->sensor_data))
			event_check = 0;
		//For On/Off type sensor 
		//���� ���� 0/1 ������ On/off ������ ���
		//���� ���� On ���� ���� ���� event �߻����� ó����.

		//printf("cur_[%s] \n", rd->prev_sensor_data, rd->sensor_data);
	}

	strcpy(rd->prev_sensor_data, rd->sensor_data);

	if (event_check)
		return VALUE_CHANGED;
	else
		return VALUE_UNCHANGED;
}
void sendSensorData(sensorList* sl){
	int rq_num = sl->rh->start->rq_num;
	char* sensor_value = sl->sensor_data_ori;
	DBusMessage *message;

	message = dbus_message_new_signal(getPath(sl), getInterface(sl), SEND_SENSOR_DATA);
  if(message == NULL){
			printf("making new signal failed\n");
	}	
	message = dbus_message_new_signal(getPath(sl), getInterface(sl), SEND_SENSOR_DATA);


	dbus_message_append_args(message,
		DBUS_TYPE_INT32, &(rq_num),
		DBUS_TYPE_STRING, &(sensor_value),
		DBUS_TYPE_STRING, &(sl->dev->valueType),
		DBUS_TYPE_STRING, &(sl->dev->valueName),
		DBUS_TYPE_INVALID);

<<<<<<< HEAD
	printf("Send the signal\n");
=======
>>>>>>> raspberry-pi2_3
	/* Send the signal */
	dbus_connection_send(connection, message, NULL);   
	dbus_message_unref(message);

<<<<<<< HEAD
	printf("Function end\n");
=======
>>>>>>> raspberry-pi2_3
	//printf("[SM] Send Sensor data to %s, rq_num[%d]\n", getInterface(sl), rq_num);
}
void sendSensorNotify(sensorList* sl){
	int rq_num = sl->rh->start->rq_num;
	char* sensor_value = sl->sensor_data_ori;
	DBusMessage *message;

	printf("[SM DEBUG] send sensor notify\n");

	message = dbus_message_new_signal(getPath(sl), getInterface(sl), SEND_SENSOR_NOTIFY);

	dbus_message_append_args(message,
		DBUS_TYPE_INT32, &(rq_num),
		DBUS_TYPE_INVALID);

	/* Send the signal */
	dbus_connection_send(connection, message, NULL);    //�� �κп��� �����͸� ������
	dbus_message_unref(message);

	//printf("[SM] Send Sensor notify to %s, rq_num[%d]\n", getInterface(sl), rq_num);
}
void* sensorThread(void* args){
	struct timespec sleep_time; //This sturct include "nano sec" not "usec" 
	int event_status;
	requestData* rd;
	char* sensor_value;
	sensorList *sl = (sensorList*)args;

	printf("[SM] %s Thread created \n", sl->dev->name);

	pthread_cond_init(&(sl->cond), NULL);
	pthread_mutex_init(&(sl->mutex), NULL);

	while(1){
		if (sl->rh->num_of_request == 0){
			sensorStop(sl, NULL);
			pthread_cond_wait(&(sl->cond), &(sl->mutex));
			continue;
		}

		if (sl->status == SENSOR_STOP){
			sensorStart(sl, NULL);
		}

		//printRequest(sl->rh);

		rd = sl->rh->start;

		//------------------ 1. Request ó�� ------------------// 
		// 
<<<<<<< HEAD
		sl->sensor_data_ori = sensorGet(sl, NULL);
=======
		//sl->sensor_data_ori = sensorGet(sl, NULL);
>>>>>>> raspberry-pi2_3
		sensor_value = sensorGet(sl, NULL);
		strcpy(rd->sensor_data, sensor_value);
		//rd->sensor_data = sensorGet(sl, NULL);

		//////////////////////////////////////////////////
		//printf("sensord data : %s \n", sl->sensor_data_ori);
		
		//Sensor Data  ȹ��
		//printf("[SM] Handle Rq[%d] from PID[%d] ", sl->rh->start->rq_num, sl->rh->start->pid);
		if (rd->handle_type == SENSING_INTERVAL){
<<<<<<< HEAD
		  printf("[SM] Handle Type is SENSING_INTERVAL\n");
			sendSensorData(sl);
		}
		else if (rd->handle_type == SENSING_EVENT_DRIVEN){
		  printf("[SM] Handle Type is SENSING_EVENT_DRIVEN\n");
=======
			sendSensorData(sl);
		}
		else if (rd->handle_type == SENSING_EVENT_DRIVEN){
>>>>>>> raspberry-pi2_3
			event_status = sensorDataParsing(sl, rd->sensor_data, sl->dev->valueType);

			if (event_status == VALUE_CHANGED){
				sendSensorNotify(sl);
			}
		}
		else{
			printf("Not supported sensor handle type \n");
		}
		
		// 2. update
		// ���� �ð� �������� ���� ���� �ð� ������Ʈ + queue ����
		scheduleRequest(sl->rh);

		// 3. sleep
		// ���� ������Ʈ ���� �ð����� ����
		sleep_time = getSleepTime(sl->rh);	
		
		pthread_cond_timedwait(&(sl->cond), &(sl->mutex), &(sleep_time));
		
	}

}
void sensorThreadInit(){
	int i, number_of_sensor;
	sensor_head_main = getSensorHead();
	sensorList* sl;
	int status;

	number_of_sensor = sensor_head_main->count;
	sl = sensor_head_main->start;

	printf("Sensor Thread Initilizing - %d of Sensor  \n", number_of_sensor);
	
	for (i = 0; i < number_of_sensor; i++){
		sl->sensor_data_prev = (char *)malloc(100);
		pthread_create(&(sl->senseor_thread), NULL, &sensorThread, (void*)sl);
		sl = sl->next;
	}
}

static DBusHandlerResult introspect(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	DBusMessage *reply;
	char *instrospect = (char *)malloc(2048);
	char *s_data = (char *)malloc(512);
	
	FILE *fp = fopen("instrospect", "r");
	fgets(instrospect, 1024, fp);
	
	while (fgets(s_data, 512, fp) != NULL) {
		strcat(instrospect, s_data);
	}
	printf("Instropect file read = %d bytes\n", strlen(instrospect));

	reply = dbus_message_new_method_return(message);
	if (reply == NULL)
		printf("Reply Null Error!\n");

	dbus_message_append_args(reply, DBUS_TYPE_STRING, &instrospect, DBUS_TYPE_INVALID);
	dbus_connection_send(connection, reply, NULL);
	fclose(fp);
	free(instrospect);
	free(s_data);

	return DBUS_HANDLER_RESULT_HANDLED;
}
static sensorManagerEventRegister(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	pthread_t tid;
	unsigned int pid;
	int rq_num;
	requestData *rd;
	sensorList* sl;

	printf("[SM] Start Register Operation\n");
	rd = (requestData *)malloc(sizeof(requestData));

	dbus_message_get_args(message, NULL,
		DBUS_TYPE_INT32, &(rd->pid),
		DBUS_TYPE_INT32, &(rd->rq_num),
		DBUS_TYPE_STRING, &(rd->sensor_name),
		DBUS_TYPE_INT32, &(rd->handle_type),
		DBUS_TYPE_INT32, &(rd->sensing_interval),
		DBUS_TYPE_INT32, &(rd->sensing_level),
		DBUS_TYPE_INVALID);

	rd->next = NULL;
	makeDbusAddress(rd);

	printf("[DEBUG] Received data check : [%u / %d / %s / %d / %d / %d ] \n", rd->pid, rd->rq_num, rd->sensor_name, rd->handle_type, rd->sensing_interval, rd->sensing_level);

	//0. Target sensor list ��������

	sl = getSensorByName(sensor_head_main, rd->sensor_name);

	if (sl == NULL){
		printf("[SM] %s is not supported sensor! \n", rd->sensor_name);
		free(rd);

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	//1. request header ��������, 2 .�ش� rh�� rq�߰�
	addRequest(sl->rh, rd);

	printf("Wake up thread, %s\n", rd->sensor_name);

	pthread_cond_signal(&(sl->cond));

	return DBUS_HANDLER_RESULT_HANDLED;
}
static sensorManagerEventUpdate(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	pthread_t tid;
	unsigned int pid;
	int rq_num;
	requestData *rd;
	sensorList* sl;
	int handle_type, sensing_interval, sensing_level;

	printf("[SM] Start Update Operation\n");

	dbus_message_get_args(message, NULL,
		DBUS_TYPE_INT32, &(pid),
		DBUS_TYPE_INT32, &(rq_num),
		DBUS_TYPE_INT32, &(handle_type),
		DBUS_TYPE_INT32, &(sensing_interval),
		DBUS_TYPE_INT32, &(sensing_level),
		DBUS_TYPE_INVALID);

	printf("[DEBUG] Received data check : [%u / %d / %d / %d / %d ] \n", pid, rq_num, handle_type, sensing_interval, sensing_level);

	//0. Target request ��������

	rd = getRequestByPidRqnum(sensor_head_main, pid, rq_num);

	if (rd == NULL){
		printf("[SM] [PID:%d/RQNUM:%d] is not exist request! \n", pid, rq_num);
		free(rd);

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	//Value update
	rd->handle_type = handle_type;
	rd->sensing_interval = sensing_interval;

	return DBUS_HANDLER_RESULT_HANDLED;
}
static sensorManagerEventUnregister(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	pthread_t tid;
	unsigned int pid;
	int rq_num;
	requestData *rd;
	sensorList* sl;

	printf("[SM] Start Unregister Operation\n");

	dbus_message_get_args(message, NULL,
		DBUS_TYPE_INT32, &(pid),
		DBUS_TYPE_INT32, &(rq_num),
		DBUS_TYPE_INVALID);

	printf("[DEBUG] Received data check : [%u / %d ] \n", pid, rq_num);

	//0. Target request ��������

	if (rq_num == -1){
		int i;
		sl = sensor_head_main->start;
		printf("[SM] Start unregister operation\n");

		for (i = 0; i < sensor_head_main->count; i++){
			deleteRequestByPid(sl, pid);
			printf("[SM] Delete request from sensor %d\n", i);
			sl = sl->next;
		}
		printf("[SM]All request is deleted\n");
	}
	else{
		sl = getSensorListByPidRqnum(sensor_head_main, pid, rq_num);

		if (sl == NULL){
			printf("[SM] [PID:%d/RQNUM:%d] is not exist request! \n", pid, rq_num);
			free(rd);

			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		}

		deleteRequestByPidRqnum(sl, pid, rq_num);

		if (sl->rh->num_of_request == 0){
			sensorStop(sl, NULL);
			pthread_cond_wait(&(sl->cond), &(sl->mutex));
		}
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}
static sensorManagerEventGet(DBusConnection *connection, DBusMessage *message, void *iface_user_data){
	pthread_t tid;
	unsigned int pid;
	int rq_num;
	requestData *rd;
	sensorList* sl;

	char* sensor_name;
	char* sensor_value;
	DBusMessage *reply;

	dbus_message_get_args(message, NULL,
		DBUS_TYPE_STRING, &(sensor_name),
		DBUS_TYPE_INVALID);

	sl = getSensorByName(sensor_head_main, sensor_name);

	if (sl->status == SENSOR_STOP){
		sensorStart(sl, NULL);
		sensor_value = sensorGet(sl, NULL);
		sensorStop(sl, NULL);
	}
	else
		sensor_value = sensorGet(sl, NULL);

	//////////////////////////////////////////////////
	//printf("sensord data : %s \n", sl->sensor_data_ori);

	//printf("test3\n");
	reply = dbus_message_new_method_return(message);
	if (reply == NULL)
		printf("Reply Null Error!\n");

	dbus_message_append_args(reply,
		DBUS_TYPE_STRING, &sensor_value,
		DBUS_TYPE_STRING, &(sl->dev->valueType),
		DBUS_TYPE_STRING, &(sl->dev->valueName),
		DBUS_TYPE_INVALID);
<<<<<<< HEAD

=======

>>>>>>> raspberry-pi2_3
	dbus_connection_send(connection, reply, NULL);

	return DBUS_HANDLER_RESULT_HANDLED;
}
//Respone function for d-bus message
static DBusHandlerResult dbus_respone(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	pthread_t tid;
	unsigned int pid;
	int rq_num;
	requestData *rd;
	sensorList* sl;

	/*
	Current support list
	- Register
	- Unregister
	- Update
	- Terminate
	- Get
	Sync data get.

	*/

	//printf("[SM] Message received\n");
	if (dbus_message_is_signal(message, OPEL_INTERFACE, "register"))
	{
		return sensorManagerEventRegister(connection, message, user_data);
	}
	else if (dbus_message_is_signal(message, OPEL_INTERFACE, "update"))
	{
		return sensorManagerEventUpdate(connection, message, user_data);
	}
	else if (dbus_message_is_signal(message, OPEL_INTERFACE, "unregister")){
		return sensorManagerEventUnregister(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, OPEL_BUS_NAME, "Get"))  // Senseor data
	{
		return sensorManagerEventGet(connection, message, user_data);
	}
	else if (dbus_message_is_method_call(message, "org.freedesktop.DBus.Introspectable", "Introspect")){
		return introspect(connection, message, user_data);
	}
}
void initDbus(){
	int retval;

	printf("Start D-Bus Initlizing \n");
	//--------------------------------------------------------------------------------//
	//                D-Bus Initilize (Path : /opel.sensor)                    //
	DBusError error;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);   // main loop ?�정
	dbus_error_init(&error);

	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

	if (dbus_error_is_set(&error))
	{
		printf("Error connecting to the daemon bus: %s", error.message);
		dbus_error_free(&error);
	}

	retval = dbus_bus_request_name(connection, OPEL_BUS_NAME, DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &error);
	dbus_connection_flush(connection);

	switch (retval) {
		case -1: {
			printf("Couldn't acquire name %s for connection: %s\n", OPEL_BUS_NAME, error.message);
			dbus_error_free(&error);
			break;
		}
		case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
			printf("Success to get bus name :  %s\n", OPEL_BUS_NAME);
			break;
		case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
			printf("In queue :  %s\n", OPEL_BUS_NAME);
			break;
		case DBUS_REQUEST_NAME_REPLY_EXISTS:
			printf("Already exists :  %s\n", OPEL_BUS_NAME);
			break;
		case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
			printf("Already own this name :  %s\n", OPEL_BUS_NAME);
			break;
		default:
			printf("Unknown result = %d\n", retval);
	}
	
	dbus_bus_add_match(connection, "type='signal',interface='org.opel.sensorManager'", NULL);  //Dbus ?�치 ?�정
	dbus_connection_add_filter(connection, dbus_respone, loop, NULL); //signal filter?�정

	dbus_connection_setup_with_g_main(connection, NULL);
	g_main_loop_run(loop); // loop ?�작
	printf("dbus loop end... \n");
	//                                                                                                                                                                //
	//--------------------------------------------------------------------------------//

}

int main(void)
{	
	sensorThreadInit();

	initDbus();

	return 0;
}



