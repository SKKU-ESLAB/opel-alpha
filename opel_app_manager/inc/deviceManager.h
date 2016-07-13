#ifndef _DEVMANAMER
#define _DEVMANAMER

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "deviceRepo.h"

using namespace std;

class deviceManager{


	private:
	public:
		deviceRepo devRepo;
		deviceList devList;

		deviceManager();

 		void Init();

	
};

#endif

