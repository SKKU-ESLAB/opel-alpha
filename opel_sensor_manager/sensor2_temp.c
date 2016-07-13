#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#define TEMP	6

static void TEMP_start(void *data)
{
	printf("Temperature sensor is start\n");
}

static void TEMP_stop(void *data)
{
	printf("Temperature sensor is terminated\n");
}

static char* TEMP_get(void *data)
{
	static char value_c[20];
	float value = (float)ADC_Read(TEMP);

	value = (500 * value) / 1024;

	sprintf(value_c, "%.2f\n", value);

	return value_c;

	//sensor1Data sData;
	//sData.value = 10;

	//data->data = (void *)malloc(sizeof(sensor1Data));
	//memcpy(data->data, &sData, sizeof(sensor1Data));
	//char* test = "77";

	//printf("Get data from sensor 1\n");

	
}

static const struct device_ops TEMP_device_ops = {
	.name = "TEMP",
	.valueType = "FLOAT",
	.valueName = "TEMP" ,//If the number of value is 1, no need to write the name of value
	.start = TEMP_start,
	.stop = TEMP_stop,
	.get = TEMP_get
};

DEVICE_OPS_REGISTER(&TEMP_device_ops)


