#include <stdio.h>
#include <stdlib.h>

#include "devices.h"

#define VIBRATION	2		//Digital, wiringPI

static unsigned char state = 0;
static unsigned char working = 0;
static unsigned char init = 0;

void vib_fuc(){
	if (working)
		state++;
}

static void VIBRATION_start(void *data)
{
	working = 1;
	
	if (!init){
		wiringPiSetup();
		pinMode(VIBRATION, INPUT);
		wiringPiISR(VIBRATION, INT_EDGE_FALLING, &vib_fuc); //Falling function!!!!!!!!!!
		init = 1;
	}

	printf("Vibration sensor is start\n");
}

static void VIBRATION_stop(void *data)
{
	working = 0;
	printf("Vibration sensor is terminated\n");
}

static char* VIBRATION_get(void *data)
{
	static char value_c[20];
	int value;

	if (state != 0){
		state = 0;
		value = 1;
	}
	else
		value = 0;

	sprintf(value_c, "%d\n", value);

	return value_c;
}

static const struct device_ops VIBRATION_device_ops = {
	.name = "VIBRATION",
	.valueType = "INT",
	.valueName = "VIBRATION" ,//If the number of value is 1, no need to write the name of value
	.start = VIBRATION_start,
	.stop = VIBRATION_stop,
	.get = VIBRATION_get
};

DEVICE_OPS_REGISTER(&VIBRATION_device_ops)


