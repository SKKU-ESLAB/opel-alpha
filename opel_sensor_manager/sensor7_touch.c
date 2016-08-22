#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#define TOUCH	25			//Digital, wiringPI
static int touch_flag = 0;


static void TOUCH_start(void *data)
{
	if (!touch_flag){
		setupWiringPi();
		pinMode(TOUCH, INPUT);
		touch_flag = 1;
		printf("Touch init\n");
	}
	printf("Touch sensor is start\n");
}

static void TOUCH_stop(void *data)
{
	printf("Touch sensor is terminated\n");
}

static char* TOUCH_get(void *data)
{
	int value;
	static char value_c[20];

	value = digitalRead(TOUCH);

	sprintf(value_c, "%d\n", value);
	printf("[SM] Touch Value :%d \n", value);

	
	return value_c;
}

static const struct device_ops TOUCH_device_ops = {
	.name = "TOUCH",
	.valueType = "INT",
	.valueName = "TOUCH",//If the number of value is 1, no need to write the name of value
	.start = TOUCH_start,
	.stop = TOUCH_stop,
	.get = TOUCH_get
};

DEVICE_OPS_REGISTER(&TOUCH_device_ops)


