#include <stdio.h>
#include <stdlib.h>

#include "devices.h"
#include "driver_common.h"

#define LIGHT	10

static void LIGHT_start(void *data)
{
	printf("Light sensor is started\n");
}

static void LIGHT_stop(void *data)
{
	printf("Light sensor is terminated\n");
}

static char* LIGHT_get(void *data)
{
	static char value_c[20];
	int value = ADC_Read(LIGHT);

	value = value / 100;
	
	sprintf(value_c, "%d\n", value);

	return value_c;
}

static const struct device_ops LIGHT_device_ops = {
	.name = "LIGHT",
	.valueType = "INT",
	.valueName = "LIGHT" ,//If the number of value is 1, no need to write the name of value
	.start = LIGHT_start,
	.stop = LIGHT_stop,
	.get = LIGHT_get
};

DEVICE_OPS_REGISTER(&LIGHT_device_ops)


