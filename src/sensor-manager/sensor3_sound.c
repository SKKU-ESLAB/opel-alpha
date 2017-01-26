#include <stdio.h>
#include <stdlib.h>

#include "devices.h"
#include "driver_common.h"

#define SOUND	9

static void SOUND_start(void *data)
{
	printf("Sound sensor is started\n");
}

static void SOUND_stop(void *data)
{
	printf("Sound sensor is terminated\n");
}

static char* SOUND_get(void *data)
{
	static char value_c[20];
	int value = ADC_Read(SOUND);
	//printf("Sound value : %d\n", value);
	sprintf(value_c, "%d\n", value);

	return value_c;

}

static const struct device_ops SOUND_device_ops = {
	.name = "SOUND",
	.valueType = "INT",
	.valueName = "SOUND" ,//If the number of value is 1, no need to write the name of value
	.start = SOUND_start,
	.stop = SOUND_stop,
	.get = SOUND_get
};

DEVICE_OPS_REGISTER(&SOUND_device_ops)


