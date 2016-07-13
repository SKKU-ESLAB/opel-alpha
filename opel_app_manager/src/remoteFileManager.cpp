#include "remoteFileManager.h"

remoteFileManager::remoteFileManager(){

}

//Or need fork process and handle that cuz changing path is affected of whole of the system when some job related to cur path is executed
//save cur dir and recover the path****
int remoteFileManager::seekDir(char *dir, jsonString* jp){
	DIR *dp;
	struct dirent *entry;
	struct stat tmp_stat;
	FILEINFO *tmp_list;
	FILEINFO *list_head;
	
	list_head = NULL;
		
	if((dp = opendir(dir)) == NULL){	//디렉토리 스트림을 Open함
		fprintf(stderr,"directory open error: %s\n",dir);
		return 0;
	}
	
	chdir(dir);
	
	while((entry = readdir(dp)) != NULL){	//디렉토리정보를 한개씩 읽어옴
		lstat(entry->d_name, &tmp_stat);

		saveAllFileInfo(&list_head,&tmp_stat,entry);
	}
	
	
	sortList(list_head);	//읽어온 자료를 정렬함
		
	parseToJson(list_head, jp);
	
	
	tmp_list = list_head;
	
	
	while( tmp_list != NULL) {
		if(S_ISDIR(tmp_list->mode)){
			
			// '.' 과 '..'은 재귀 에서 배제함
			if( strcmp(".",tmp_list->filename) == 0 || strcmp("..",tmp_list->filename) == 0 ){
				tmp_list = tmp_list->next;
				continue;
			}
			
		}
		
		tmp_list = tmp_list->next;
		
	}
	chdir("..");
	
	closedir(dp);
	freeList(list_head);	//파일정보가 저장된 메모리를 해제함
}

int remoteFileManager::parseToJson(FILEINFO *list_head, jsonString* jp){
	FILEINFO *cur_list;
		unsigned long tmp_perm;
		struct tm *tm_ptr;
		int i;

		cur_list = list_head;
	
		while(cur_list != NULL){
			tmp_perm = cur_list->mode;
			char fileName[FILENAME_SIZE]={'\0',};
			char otherInfo[1024]={'\0',};
			
			char type[16]={'\0',};
			
			//File Type에따라서 출력할 색을 달리함
			if(S_ISREG(cur_list->mode)){
				/*if(cur_list->mode & 01001001)	//exe file?
					printf("%c[1;32m",27);
				else
					printf("%c[0m",27);
				*/
				
				strcpy(type,"REG");
				
			}else if(S_ISDIR(cur_list->mode)){
				strcpy(type,"DIR");

			}
			else{
				strcpy(type,"ETC");
			}

			tm_ptr = gmtime(&cur_list->atime);	//시간값을 분석하여 구조체에 저장
			
			sprintf(fileName, "%s", cur_list->filename);
			sprintf(otherInfo, "%s/%u/%02d-%02d-%2d %02d:%02d", type,(unsigned int)cur_list->size,tm_ptr->tm_year%100,tm_ptr->tm_mon,tm_ptr->tm_mday,tm_ptr->tm_hour,tm_ptr->tm_min);
			jp->addItem(fileName,otherInfo);
			
			/*
			else if(S_ISCHR(cur_list->mode)){
				printf("%c[1;37m",27);
				printf("CHR  ");
			}else if(S_ISBLK(cur_list->mode)){
				printf("%c[1;33m",27);
				printf("BLK  ");
			}else if(S_ISFIFO(cur_list->mode)){
				printf("%c[1;37m",27);		
				printf("FIFO ");
			}else if(S_ISLNK(cur_list->mode)){
				printf("%c[1;36m",27);
				printf("LNK  ");
			}else if(S_ISSOCK(cur_list->mode)){
				printf("%c[1;35m",27);
				printf("SOCK ");
			}


			//File Size 
			printf(" %12u",(unsigned int)cur_list->size);
			
			//마지막 수정시간 출력
			tm_ptr = gmtime(&cur_list->atime);	//시간값을 분석하여 구조체에 저장
			printf(" %02d/%02d/%2d %02d:%02d",tm_ptr->tm_year%100,tm_ptr->tm_mon,tm_ptr->tm_mday,tm_ptr->tm_hour,tm_ptr->tm_min);
			

			
			//Permission 
			for(i=0;i<3;i++)
			{
				if(tmp_perm & S_IRUSR)
					printf("r");
				else
					printf("-");
				
				if(tmp_perm & S_IWUSR)
					printf("w");
				else
					printf("-");
				
				if(tmp_perm & S_IXUSR)
					printf("x");
				else
					printf("-");
				
				tmp_perm <<=3;
			}*/

			/*
			//hard-link 
			printf(" %2u",(unsigned int)cur_list->nlink);
			
			//User ID
			printf(" %5u",(unsigned int)cur_list->uid);
			
			//Group ID
			printf(" %5u",(unsigned int)cur_list->gid);
			
			
			
			//이 파일이 차지하는 block수
			printf("%5u",(unsigned int)cur_list->blocks);
			*/
		
			cur_list = cur_list->next;
		}

}

void remoteFileManager::saveAllFileInfo(FILEINFO **list_head,struct stat *cur_stat,struct dirent *cur_dir){
	FILEINFO *cur_list=(*list_head);
		
		if( *list_head != NULL)
			while( cur_list->next != NULL)
				cur_list = cur_list->next;
			
			if( cur_dir->d_name[0] == '.' )
				
					return ; //all옵션이 없으면 .으로 시작하는 파일은 스킵힌다. 
				
				if( (*list_head) == NULL){	 //리스트의 맨처음일때
					cur_list = (FILEINFO *)malloc(sizeof(FILEINFO));
					cur_list->next = NULL;
					*list_head = cur_list;
				}
				else{
					cur_list->next = (FILEINFO *)malloc(sizeof(FILEINFO));
					cur_list = cur_list->next;
					cur_list->next = NULL;
				}
				
				
				cur_list->inode = cur_stat->st_ino;
				cur_list->mode = cur_stat->st_mode;
				strcpy(cur_list->filename ,cur_dir->d_name);
				
				//hard-link 수
				cur_list->nlink = cur_stat->st_nlink;
				
				//User ID  
				cur_list->uid = cur_stat->st_uid;
				
				//Group ID
				cur_list->gid = cur_stat->st_gid;
				
				//File Size 
				cur_list->size = cur_stat->st_size;
				
				//마지막 수정시간 
				cur_list->atime = cur_stat->st_atime;	//시간값을 분석하여 구조체에 저장
				
				//이 파일이 차지하는 block수
				cur_list->blocks = cur_stat->st_blocks;

}

void remoteFileManager::freeList(FILEINFO *list_head){
	FILEINFO *tmp_list;
	
	while(list_head != NULL){     //메모리 해제하기
		tmp_list = list_head;
		list_head = list_head->next;
		free(tmp_list);
	}
}

void remoteFileManager::sortList(FILEINFO *list_head){	//sorted by ascending order of name
		FILEINFO *tmp_list_left;
		FILEINFO *tmp_list_right;
		
		FILEINFO tmp_list;
		FILEINFO *tmp_listp;
		
		
		if( list_head == NULL )
			return ;
		
		if( list_head->next == NULL)
			return ;
		
		tmp_list_left = list_head;
		tmp_list_right = list_head->next;
		
		
		while( tmp_list_left->next != NULL){
			
			while(tmp_list_right != NULL){
				
				//두개의 리스트를 서로 exchange하는 코드
				if( strcmp(tmp_list_left->filename,tmp_list_right->filename) >0 ){	
					memcpy(&tmp_list , tmp_list_left,sizeof(FILEINFO));
					memcpy(tmp_list_left , tmp_list_right,sizeof(FILEINFO));
					memcpy(tmp_list_right , &tmp_list,sizeof(FILEINFO));
					tmp_listp = tmp_list_left->next;
					tmp_list_left->next = tmp_list_right->next;
					tmp_list_right->next = tmp_listp;
				}
				
				tmp_list_right = tmp_list_right->next;
			}
			tmp_list_left = tmp_list_left->next;
			tmp_list_right = tmp_list_left->next;
		}

}



