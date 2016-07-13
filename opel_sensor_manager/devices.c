#include <stdio.h>
#include "devices.h"

static sensorHead *sensor_head = NULL;

#define	Clock	27
#define	Address	28
#define	DataOut	29

static int ADC_INIT = 0;
static int ADC_LOCK = 0;

int ADC_init(){
	if (wiringPiSetup() < 0){
		printf("Error : Fail to init WiringPi\n");
		return 1;
	}

	pinMode(DataOut, INPUT);
	pullUpDnControl(DataOut, PUD_UP);

	pinMode(Clock, OUTPUT);
	pinMode(Address, OUTPUT);
	ADC_INIT = 1;

}

unsigned int ADC_Read(unsigned char channel)
{
	unsigned int value;
	unsigned char i;
	unsigned char LSB = 0, MSB = 0;

	if (!ADC_INIT)
		ADC_init();

	if (ADC_LOCK){
		while (ADC_LOCK){}
	}

	ADC_LOCK = 1;

	channel = channel << 4;
	for (i = 0; i < 4; i++)
	{
		if (channel & 0x80)
			digitalWrite(Address, 1);
		else
			digitalWrite(Address, 0);
		digitalWrite(Clock, 1);
		digitalWrite(Clock, 0);
		channel = channel << 1;
	}
	for (i = 0; i < 6; i++)
	{
		digitalWrite(Clock, 1);
		digitalWrite(Clock, 0);
	}

	delayMicroseconds(15);
	for (i = 0; i < 2; i++)
	{
		digitalWrite(Clock, 1);
		MSB <<= 1;
		if (digitalRead(DataOut))
			MSB |= 0x1;
		digitalWrite(Clock, 0);
	}
	for (i = 0; i < 8; i++)
	{
		digitalWrite(Clock, 1);
		LSB <<= 1;
		if (digitalRead(DataOut))
			LSB |= 0x1;
		digitalWrite(Clock, 0);
	}
	value = MSB;
	value <<= 8;
	value |= LSB;

	ADC_LOCK = 0;

	return value;
}


sensorList* getSensorByName(sensorHead* sh, char* sensor_name){
	sensorList* temp;
	temp = sh->start;

	while (temp != NULL){
		if (!strcmp(temp->dev->name, sensor_name))
			break;

		temp = temp->next;
	}
	if (temp == NULL){
		printf("Error : %s is not supported sensor! \n", sensor_name);
	}
	return temp;
}

sensorHead* getSensorHead(void){
	return sensor_head;
}
sensorHead* initSensors(void){
	sensor_head = (sensorHead*)malloc(sizeof(sensorHead));
	sensor_head->start = NULL;
	sensor_head->count = 0;

	return sensor_head;
}

void addSensor(const struct device_ops *dev)
{
	sensorList* sl;
	sensorList* new_sl;

	if (sensor_head == NULL){
		printf("Sensor head is not initilized! Perform initilizing\n");
		initSensors();
	}

	sl = sensor_head->start;
	if (sl == NULL){
		sensor_head->start = (sensorList*)malloc(sizeof(sensorList));
		new_sl = sensor_head->start;
	}
	else{
		while (1){
			if (sl->next == NULL)
				break;
			sl = sl->next;
		}

		sl->next = (sensorList*)malloc(sizeof(sensorList));
		new_sl = sl->next;
	}
	new_sl->dev = dev;
	new_sl->next = NULL;

	//new_sl->num_of_request = 0;
	new_sl->rh = initRequestHeader();
	new_sl->status = SENSOR_STOP;

	sensor_head->count++;

	

	printf("[SM] %s is added [Total number of sensor :%d ]\n", dev->name, sensor_head->count);
}

void deleteSensor(const struct device_ops *dev){
}

int countSensor(void);
/*
	Sensor On/Off 등의 컨트롤에 대한 정보도 추가.
*/




