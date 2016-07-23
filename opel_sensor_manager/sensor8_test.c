#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#define TEST	125			//Digital, wiringPI
static int test_flag = 0;


static void TEST_start(void *data)
{
	if (!test_flag){
		wiringPiSetup();
		pinMode(TEST, INPUT);
		test_flag = 1;
		printf("Touch init\n");
	}
	printf("Touch sensor is start\n");
}

static void TEST_stop(void *data)
{
	printf("Touch sensor is terminated\n");
}

static char* TEST_get(void *data)
{
	int value;
	static char value_c[20];

	value = digitalRead(TEST);

	sprintf(value_c, "%d\n", value);
	printf("[SM] Touch Value :%d \n", value);

	
	return value_c;
}

static const struct device_ops TEST_device_ops = {
	.name = "TEST",
	.valueType = "INT",
	.valueName = "TEST",//If the number of value is 1, no need to write the name of value
	.start = TEST_start,
	.stop = TEST_stop,
	.get = TEST_get
};

DEVICE_OPS_REGISTER(&TEST_device_ops)


