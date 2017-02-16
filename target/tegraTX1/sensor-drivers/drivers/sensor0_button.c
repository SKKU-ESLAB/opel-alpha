#include "jetsonGPIO.h"

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


