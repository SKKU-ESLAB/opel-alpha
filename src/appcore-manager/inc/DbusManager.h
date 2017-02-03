/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Dongig Sin<dongig@skku.edu>, 
 *              Gyeonghwan Hong<redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DBUSMANAGER
#define _DBUSMANAGER

#include <stdio.h>

#include "JsonString.h"
#include "GlobalData.h"

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
		void sendMsg(JsonString js, char* typeOfSignal);

		bool makeTerminationEvent(JsonString js);
		bool makeConfigEvent(JsonString js);		
		void Sensor_all_request_unregister(int pid);
		void sendTerminationToCameraManager();


};

#endif

