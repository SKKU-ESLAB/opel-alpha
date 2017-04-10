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

