/* Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
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

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

extern "C"
{
#include "libz/miniunz.h"
}

#include "appPackageRepo.h"
#include "appPackageManager.h"
#include "globalData.h"
#include "commManager.h"
#include "jsonString.h"

#define _APPPACKAGEMANAGER

using namespace std;

appPackageManager::appPackageManager(){

	struct stat st = {0};
	
	char* opelEnvPath;
	opelEnvPath = getenv("OPEL_DIR");

	sprintf(PKG_STORAGE_PATH, "%s%s", opelEnvPath, "/bin/appManager/application/");

	if (stat(PKG_STORAGE_PATH, &st) == -1) {
    	mkdir(PKG_STORAGE_PATH, 0755);
	}
	
}


appPackageList* appPackageManager::getAppList(){

	appPkgRepo.selectAllAppPackage(appPkgList);

	return &appPkgList;
}

appPackageRepo appPackageManager::getAppRepo(){
	return appPkgRepo;	
}


// input : pkg file name  to install on PKGSTORAGEPATH
// output : App ID / Name / fileName Json on pkg DB
// assumption [TEMP] : pkg was downloaded on PKGSTORAGEPATH
// check reused name [TEMP]
jsonString appPackageManager::installPackage(char* pkgFileName){

	//save pkg file and decompress
	//update DB
	//update appList

	char unzipCommand[256]={'\0',};
	
	char pkgDirName[256]={'\0',};
	char pkgFullDirPath[256]={'\0',};							  // ./application/2015_xx_xx_xx_xx/
	char pkgFilePath[256]={'\0',};								  // ./application/2015_xx_xx_xx_xx.opk
	strncpy(pkgDirName, pkgFileName, strlen(pkgFileName)-4);
	sprintf(pkgFullDirPath, "%s%s", PKG_STORAGE_PATH, pkgDirName);


	struct stat st = {0};
	if (stat(pkgFullDirPath, &st) == -1) {
    	mkdir(pkgFullDirPath, 0755);
	}
	/*else{
		printf("pkg dir is already exist : %s\n", pkgFullDirPath);
		return ;
	}*/

	sprintf(pkgFilePath, "%s%s", PKG_STORAGE_PATH, pkgFileName);
//	sprintf(unzipCommand,"unzip -o %s -d %s/", pkgFilePath, pkgFullDirPath);
//	sprintf(unzipCommand,"tar xvf %s -C %s/", pkgFilePath, pkgFullDirPath);
//	{"","test", "-d", "./test/"};
	char* cmdUnzip[4] ={0,};
	cmdUnzip[0] = "";
	cmdUnzip[1] = pkgFilePath;
	cmdUnzip[2] = "-d";
	cmdUnzip[3] = pkgFullDirPath;

	do_unzip(4, cmdUnzip);
	chdir("../../");
	sync();

/*    cmdUnzip[0] = malloc(sizeof(char) * 1);
	cmdUnzip[1] = malloc(sizeof(char) * strlen(pkgFilePath));
	cmdUnzip[2] = malloc(sizeof(char) * 3);
	cmdUnzip[3] = malloc(sizeof(char) * strlen(pkgFileName));

	strcat(cmdUnzip[0], "");
	strcat(cmdUnzip[2], "-d");

	strcat(cmdUnzip[1], pkgFilePath);
 	strcat(cmdUnzip[3], pkgFileName);

	do_unzip(4, cmdUnzip);

	free(cmdUnzip[1]);
	free(cmdUnzip[3]);
*/
//	printf("[appPackageManager] Unzip >> command %s\n", unzipCommand);
//	system(unzipCommand);

	if ( remove(pkgFilePath) == -1 ){
		printf("[appPackageManager] Cannot remove pkg file : %s\n", pkgFilePath);
	}
	
	xmlDocPtr doc;
	xmlNodePtr cur;

///////////

/*	char dirname[1024];
	getcwd(dirname, 1024);
	printf("dirname : %s\n", dirname);
*/

	char manifestPath[512];
	sprintf(manifestPath, "%s/manifest.xml", pkgFullDirPath);

	doc = xmlParseFile( manifestPath );

	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");

		return NULL;
	}

	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return NULL;
	}

	
	if (xmlStrcmp(cur->name, (const xmlChar *) "application")) {
		fprintf(stderr,"document of the wrong type, root node != story");
		xmlFreeDoc(doc);
		return NULL;
	}

	cur = cur->xmlChildrenNode;

	char appIconFileName[256]={'/0',};
	char appLabel[256]={'/0',};
	char appMainFile[256]={'/0',};
	
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"icon"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appIconFileName, "%s", key);
			printf("[appPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appIconFileName);
			xmlFree(key);
		
		
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"label"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appLabel, "%s", key);
			printf("[appPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appLabel);
			xmlFree(key);
		
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainFile"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appMainFile, "%s", key);
			printf("[appPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appMainFile);			
			xmlFree(key);
		}
		
	cur = cur->next;
	}
	xmlFreeDoc(doc);


	// insert app info to repository
	char exeFilePath[128];
	sprintf(exeFilePath, "%s/%s", pkgFullDirPath,appMainFile);

	int appID = appPkgRepo.insertAppPackage( appLabel, pkgFullDirPath, exeFilePath);

	char appIDStr[128];
	sprintf(appIDStr, "%d", appID);


	//send IconFile with appId, appName, 


	jsonString jp;
	jp.addType(INSTALLPKG);
	jp.addItem("appID",appIDStr);
	jp.addItem("appName",appLabel);
	jp.addItem("appPath", pkgFullDirPath);
	jp.addItem("appIconName", appIconFileName);

	return jp;
}

bool appPackageManager::deletePackage(int appID){
	
	char rmCommand[256] = {'\0',};

	appPackage* appPkg = appPkgRepo.selectAppPackage(appID); 

	sprintf(rmCommand, "rm -rf %s", appPkg->getApFilePath());

	//Update DB
	appPkgRepo.deleteAppPackage(appID);

	//Remove file_dir	
	//system(rmCommand);

	delete appPkg;
	return true;

}


void appPackageManager::updatePkgList(){
	appPkgRepo.selectAllAppPackage(appPkgList);	
}

char* appPackageManager::getRunningPath(char* appID){

	appPackage* appPkg = appPkgRepo.selectAppPackage(atoi(appID)); 

	//Fixed

	int len = strlen(appPkg->getApExePath());

	char *ret = (char *)malloc(len+1);
	strcpy(ret, appPkg->getApExePath());
	delete appPkg;
	
	return ret;

}

char* appPackageManager::getAppDirPath(char* appID){


	appPackage* appPkg = appPkgRepo.selectAppPackage(atoi(appID));

	int len = strlen(appPkg->getApFilePath());
	char *ret = (char *)malloc(len+1);
	strcpy(ret, appPkg->getApFilePath());
	delete appPkg;
		

	return ret;
}


