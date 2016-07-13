#ifndef _APPSTATUSMANAGER
#define _APPSTATUSMANAGER

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "jsonString.h"
#include "globalData.h"

using namespace std;


class appStatusManager{

	private:
		appProcessTable* appProcList;
	public:
		appStatusManager();

		bool runNewApplication(jsonString js, char* _filePath);
		void exitApplication(int pid);
		int runNativeJSApp(int type);

};



#endif



