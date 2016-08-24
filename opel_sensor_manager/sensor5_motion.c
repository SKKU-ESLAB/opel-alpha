#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#define MOTION_PIN	1			//Digital, wiringPI

static int flag = 0;

static void MOTION_start(void *data)
{
	if(!flag){
		setupWiringPi();
		pinMode(MOTION_PIN, INPUT);
		flag = 1;
	}
	printf("Motion sensor is start\n");
}

static void MOTION_stop(void *data)
{
	printf("Motion sensor is terminated\n");
}

static char* MOTION_get(void *data)
{	
	static char value_c[20];
	int value = digitalRead(MOTION_PIN);
	
	sprintf(value_c, "%d\n", value);

	return value_c;
	
}

static const struct device_ops MOTION_device_ops = {
	.name = "MOTION",
	.valueType = "INT",
	.valueName = "MOTION" ,//If the number of value is 1, no need to write the name of value
	.start = MOTION_start,
	.stop = MOTION_stop,
	.get = MOTION_get
};

DEVICE_OPS_REGISTER(&MOTION_device_ops)


