#ifndef _REMOTEFILEMANAGER
#define _REMOTEFILEMANAGER

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "jsonString.h"

using namespace std;

#define PATH_LENGTH 	256
#define FILEINFO 	struct file_info
#define FILENAME_SIZE	256

struct file_info{
	char filename[FILENAME_SIZE+1];
	unsigned long inode;
	unsigned long mode;
	unsigned long nlink;
	unsigned long uid;
	unsigned long gid;
	unsigned long size;
	//마지막 수정시간 출력
	time_t atime;  
	//이 파일이 차지하는 block수
	unsigned long blocks;
	struct file_info *next;
};




class remoteFileManager{

	private:
		char prevPath[PATH_LENGTH];
	public:
		remoteFileManager();
		
		int seekDir(char *dir, jsonString* json);
		int parseToJson(FILEINFO *list_head, jsonString* json);	
		void saveAllFileInfo(FILEINFO **list_head,struct stat *cur_stat,struct dirent *cur_dir);	
		
		void freeList(FILEINFO *list_head);	
		void sortList(FILEINFO *list_head);	//sorted by ascending order of name
		
	
};

#endif

