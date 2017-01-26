#include <stdio.h>
#include <stdlib.h>

#include "devices.h"
#include "jetsonGPIO.h"

//#define TOUCH	25			//Digital, wiringPI
static int button_flag = 0;
enum jetsonTX1GPIONumber pushButton = gpio38;

void doNothing()
{
//	printf("Hello!\n");
}


static void BUTTON_start(void *data)
{
  
  gpioExport(pushButton);
  gpioSetDirection(pushButton, inputPin);
  printf("Button sensor is start\n");
}

static void BUTTON_stop(void *data)
{
	printf("Button sensor is terminated\n");
	gpioUnexport(pushButton);
}

static char* BUTTON_get(void *data)
{
	unsigned int value;
	static char value_c[20];

  	gpioGetValue(pushButton, &value);

	
	sprintf(value_c, "%d\n", value);
	printf("[SM] Button Value :%d \n", value);

	
	return value_c;
}

static const struct device_ops BUTTON_device_ops = {
	.name = "BUTTON",
	.valueType = "INT",
	.valueName = "BUTTON",//If the number of value is 1, no need to write the name of value
	.start = BUTTON_start,
	.stop = BUTTON_stop,
	.get = BUTTON_get
};

DEVICE_OPS_REGISTER(&BUTTON_device_ops)


