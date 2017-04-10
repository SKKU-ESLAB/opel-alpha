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

#include "RemoteFileManager.h"

RemoteFileManager::RemoteFileManager(){

}

//Or need fork process and handle that cuz changing path is affected of whole of the system when some job related to cur path is executed
//save cur dir and recover the path****
int RemoteFileManager::seekDir(char *dir, JsonString* jp){
	DIR *dp;
	struct dirent *entry;
	struct stat tmp_stat;
	FILEINFO *tmp_list;
	FILEINFO *list_head;
	
	list_head = NULL;
		
	if((dp = opendir(dir)) == NULL){	//���丮 ��Ʈ���� Open��
		fprintf(stderr,"directory open error: %s\n",dir);
		return 0;
	}
	
	chdir(dir);
	
	while((entry = readdir(dp)) != NULL){	//���丮������ �Ѱ��� �о��
		lstat(entry->d_name, &tmp_stat);

		saveAllFileInfo(&list_head,&tmp_stat,entry);
	}
	
	
	sortList(list_head);	//�о�� �ڷḦ ������
		
	parseToJson(list_head, jp);
	
	
	tmp_list = list_head;
	
	
	while( tmp_list != NULL) {
		if(S_ISDIR(tmp_list->mode)){
			
			// '.' �� '..'�� ��� ���� ������
			if( strcmp(".",tmp_list->filename) == 0 || strcmp("..",tmp_list->filename) == 0 ){
				tmp_list = tmp_list->next;
				continue;
			}
			
		}
		
		tmp_list = tmp_list->next;
		
	}
	chdir("..");
	
	closedir(dp);
	freeList(list_head);	//���������� ����� �޸𸮸� ������
}

int RemoteFileManager::parseToJson(FILEINFO *list_head, JsonString* jp){
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
			
			//File Type������ ����� ���� �޸���
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

			tm_ptr = gmtime(&cur_list->atime);	//�ð����� �м��Ͽ� ����ü�� ����
			
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
			
			//������ �����ð� ���
			tm_ptr = gmtime(&cur_list->atime);	//�ð����� �м��Ͽ� ����ü�� ����
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
			
			
			
			//�� ������ �����ϴ� block��
			printf("%5u",(unsigned int)cur_list->blocks);
			*/
		
			cur_list = cur_list->next;
		}

}

void RemoteFileManager::saveAllFileInfo(FILEINFO **list_head,struct stat *cur_stat,struct dirent *cur_dir){
	FILEINFO *cur_list=(*list_head);
		
		if( *list_head != NULL)
			while( cur_list->next != NULL)
				cur_list = cur_list->next;
			
			if( cur_dir->d_name[0] == '.' )
				
					return ; //all�ɼ��� ������ .���� �����ϴ� ������ ��ŵ����. 
				
				if( (*list_head) == NULL){	 //����Ʈ�� ��ó���϶�
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
				
				//hard-link ��
				cur_list->nlink = cur_stat->st_nlink;
				
				//User ID  
				cur_list->uid = cur_stat->st_uid;
				
				//Group ID
				cur_list->gid = cur_stat->st_gid;
				
				//File Size 
				cur_list->size = cur_stat->st_size;
				
				//������ �����ð� 
				cur_list->atime = cur_stat->st_atime;	//�ð����� �м��Ͽ� ����ü�� ����
				
				//�� ������ �����ϴ� block��
				cur_list->blocks = cur_stat->st_blocks;

}

void RemoteFileManager::freeList(FILEINFO *list_head){
	FILEINFO *tmp_list;
	
	while(list_head != NULL){     //�޸� �����ϱ�
		tmp_list = list_head;
		list_head = list_head->next;
		free(tmp_list);
	}
}

void RemoteFileManager::sortList(FILEINFO *list_head){	//sorted by ascending order of name
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
				
				//�ΰ��� ����Ʈ�� ���� exchange�ϴ� �ڵ�
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



