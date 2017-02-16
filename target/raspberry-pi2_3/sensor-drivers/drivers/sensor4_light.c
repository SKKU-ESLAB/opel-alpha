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

