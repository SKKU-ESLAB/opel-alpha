#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "deviceRepo.h"

#include "deviceManager.h"





using namespace std;


deviceManager::deviceManager(){
		Init();
		devRepo.selectAllDevice(devList);
		devList.printAllDevList();
}

void deviceManager::Init(){
		//update LIst
}

	
