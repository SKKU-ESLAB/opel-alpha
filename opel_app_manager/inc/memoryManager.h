#ifndef _MEMORYMANAGER
#define _MEMORYMANAGER

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;


class memoryManager{

	private:
	public:
		//check memory usage of each user process(app) [thread]
		void runToCheckProcessMemory();
	
};

#endif
