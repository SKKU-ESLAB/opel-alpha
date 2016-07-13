#ifndef _DBUSMANAGER
#define _DBUSMANAGER

#include <stdio.h>

#include "jsonString.h"
#include "globalData.h"

#define OPEL_BUS_NAME	"org.opel.sysAppManager"
#define OPEL_Camera_BUS_NAME	"org.opel.camera.daemon"


#define TERMINATION_SIGNAL "termination_event_driven"
#define CONFIGURATION_SIGNAL "config_event_driven"

#define SM_INTERFACE "/org/opel/sensorManager"
#define SM_PATH	"org.opel.sensorManager"



class DbusManager{

	private:
		static void pid_to_char(unsigned int input, char* output);
		static void *initDbus(void* data);
		

	public:
		
		DbusManager();
		void sendMsg(jsonString js, char* typeOfSignal);

		bool makeTerminationEvent(jsonString js);
		bool makeConfigEvent(jsonString js);		
		void Sensor_all_request_unregister(int pid);
		void sendTerminationToCameraManager();


};

#endif

