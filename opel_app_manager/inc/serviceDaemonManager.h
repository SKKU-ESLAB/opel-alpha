
#ifndef _SERVICEDAEMONMANAMER
#define _SERVICEDAEMONMANAMER

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "serviceDaemonRepo.h"


using namespace std;



class serviceDaemonManager{

	private:
		serviceDaemonRepo smRepo;
		serviceDaemonList smList;
		
	public:
		serviceDaemonManager();
		
		//check every service process is alive (Thread)
		//and resume the process if any process has problem to run
		void start();
		
};
#endif

