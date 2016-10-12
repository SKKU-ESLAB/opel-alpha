/* Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Dongig Sin<dongig@skku.edu>
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

#include <vector>

#include "mainSysAppManager.h"
#include "globalData.h"
#include "appStatusManager.h"
#include "appPackageManager.h"
#include "DbusManager.h"
#include "commManager.h"
#include "remoteFileManager.h"
#include "jsonString.h"

appProcessTable* appProcList = NULL;
appPackageManager apManager;
appStatusManager asManager;  // keep tracking pid of running process (user app)
DbusManager dbusManager;
comManager* cm = NULL;
remoteFileManager rfm;

int pidOfCameraViewer;
int pidOfSensorViewer;

void sigchld_handler(int signum) {
  int status;
  int pid;

  if (pid > 0 && pid != pidOfCameraViewer && pid != pidOfSensorViewer) {
    // Handle 3rd-party App Termination Event on the main thread
    printf("[Main] SIGCHLD Handler >> Child was killed [User app pid : %d]\n",
        pid);

    // TODO(redcarrottt): send D-bus signal to sensor/camera manager
    // TODO(redcarrottt): update killed app info to android
    char appID[4] = {'\0', };
    appProcessInfo appProc;
    if (!appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)) {
      printf("[sigchld handler] Cannot find the procInfo >> pid : %d\n", pid);
      return;
    }

    snprintf(appID, sizeof(appID), "%s", appProc.getAppProcId());

    asManager.exitApplication(atoi(appID));
    cm->responseAppExitComplete(appID);
  } else {
    // Handle Built-in App Termination Event on the main thread
    // ex. camera viewer, sensor viewer
    if (pid == pidOfCameraViewer) {
      printf("[Main] SIGCHLD Handler >> Native Camera Child was killed\n");
      pidOfCameraViewer = 0;
    } else if (pid == pidOfSensorViewer) {
      printf("[Main] SIGCHLD Handler >> Native Sensor Child was killed\n");
      pidOfSensorViewer = 0;
    }
  }
}

void onCommandInstallAppPackage(jsonString js) {
  // Intall App Package
  printf("[MAIN] Request >> Install Package\n");
  char pkgFileName[MSGBUFSIZE] = {0, };

  if (!(cm->HandleInstallPkg(pkgFileName)))
    return;

  jsonString ret_js = apManager.installPackage(pkgFileName);

  if (&ret_js != NULL) {
    char pkgFileName[1024] = {'\0', };
    strncpy(pkgFileName, js.findValue("pkgFileName").c_str(), 1024);
    ret_js.addItem("pkgFileName", pkgFileName);

    cm->responsePkgInstallComplete(ret_js);
  }
}

void onCommandExecuteApp(jsonString js) {
  // Execute App
  printf("[MAIN] Request >> Execute App\n");

  char appID[16]={'\0', };
  strncpy(appID, js.findValue("appID").c_str(), 16);

  // Check if the app exists in running table
  if (!appProcList->isExistOnRunningTableByAppID(atoi(appID))) {
    char* runPath = apManager.getRunningPath(appID);
    char* dirPath = apManager.getAppDirPath(appID);

    js.addItem("dirPath", dirPath);
    printf("[MAIN] run app runpath : %s, dir path : %s\n",
        runPath, dirPath);

    if (asManager.runNewApplication(js, runPath)) {
      cm->responseAppRunComplete(js);
    }
    delete runPath;
    delete dirPath;
  } else {
    printf("[MAIN] appID : %s is already running\n", appID);
  }
}

void onCommandKillApp(jsonString js) {
  // Kill App
  printf("[MAIN] Request >> KILL App\n");
  int appId = atoi(js.findValue("appID").c_str());
  if (appProcList->isExistOnRunningTableByAppID(appId)) {
    if (dbusManager.makeTerminationEvent(js)) {
      // TODO(redcarrottt): termination event's return value
    }
  } else {
    printf("[MAIN] appID : %s is already dead\n",
        js.findValue("appID").c_str());
  }
}

void onCommandUpdateAppInfo(jsonString js) {
  // Update App Information
  printf("[MAIN] Request >> Update App Infomation\n");

  vector<appPackage*> *apList = apManager.getAppList()->getListVector();
  vector<appPackage*>::iterator apIter;

  js.addType(UPDATEAPPINFO);

  for (apIter = apList->begin(); apIter != apList->end(); ++apIter) {
    int appID_ = (*apIter)->getApID();
    char appID[16]= {'\0', };
    snprintf(appID, sizeof(appID), "%d", appID_);

    char appName[fileNameLength]= {'\0', };
    snprintf(appName, sizeof(appName), "%s", (*apIter)->getApName());

    if (appProcList->isExistOnRunningTableByAppID(appID_)) {
      // Append "/1" to appID
      snprintf(appID, sizeof(appID), "%s/1", appID);
    } else {
      // Append "/0" to appID
      snprintf(appID, sizeof(appID), "%s/0", appID);
    }
    js.addItem(appID, appName);
  }

  char addr[64] = {0, };
  if (cm->getIpAddress("wlan0", addr) > 0) {
    printf("[CommManager] get IP(wlan0) : %s\n", addr);
  } else if (cm->getIpAddress("eth0", addr) > 0) {
    printf("[CommManager] get IP(eth0) : %s\n", addr);
  }
  js.addItem("IP_ADDR__a", addr);

  cm->responseUpdatePkgList(js.getJsonData().c_str());
}

void onEventConfigSetting(jsonString js) {
  // Config Setting Event
  printf("[MAIN] Request >> Config Setting Event\n");

  // Check if the app exists in running table
  int appId = atoi(js.findValue("appID").c_str());
  if (appProcList->isExistOnRunningTableByAppID(appId)) {
    if (dbusManager.makeConfigEvent(js)) {
      // TODO(redcarrottt): handle config event's return value
    }
  } else {
    printf("[MAIN] appID : %s is already dead\n",
        js.findValue("appID").c_str());
  }
}

void onCommandRunNativeCameraViewer(jsonString js) {
  // Run native camera viwer app
  if (pidOfCameraViewer == 0) {
    while (false == cm->wfdOn()) {
      sleep(1);
    }
    pidOfCameraViewer = asManager.runNativeJSApp(1);
  }
}

void onCommandRunNativeSensorViewer(jsonString js) {
  // Run native sensor viewer app
  pidOfSensorViewer = asManager.runNativeJSApp(2);
}

void onCommandTerminateNativeCameraViewer(jsonString js) {
  // Terminate native camera viewer app
  if (pidOfCameraViewer != 0) {
    kill(pidOfCameraViewer, SIGKILL);
    dbusManager.sendTerminationToCameraManager();
  }
}

void onCommandTerminateNativeSensorViewer(jsonString js) {
  // Terminate native sensor viewer app
  if (pidOfSensorViewer != 0) {
    kill(pidOfSensorViewer, SIGKILL);
  }
}
void onEventAndroidTerminate(jsonString js) {
  // Terminate Android OPEL Manager Event
  printf("Android activity backed or pause\n");
  cm->closeConnection();
  cm->makeConnection();
}

void onCommandDeleteApp(jsonString js) {
  // Delete app
  printf("[MAIN] Request >> DELETE App\n");

  char appID[16]={'\0', };
  snprintf(appID, sizeof(appID), "%s", js.findValue("appID").c_str());

  if (!appProcList->isExistOnRunningTableByAppID(atoi(appID))) {
    // Delete whole of the file and update DB
    if (apManager.deletePackage(atoi(appID))) {
      cm->responsePkgUninstallComplete(js);
    } else {
      printf("[MAIN] appID : %s fail to delete\n", appID);
    }
  } else {
    printf("[MAIN] appID : %s is running, Cannot remove this app\n",
        appID);
  }
}

void onRFMCommandGetListOfCurrentPaths(jsonString js) {
  // Get list of current paths
  char path[1024] = {'\0', };
  snprintf(path, sizeof(path), "%s", js.findValue("path").c_str());

  jsonString sendJp;
  sendJp.addType(RemoteFileManager_getListOfCurPath);
  rfm.seekDir(path, &sendJp);

  cm->responseUpdateFileManager(sendJp);
}

void onRFMCommandRequestFile(jsonString js) {
  // Request a file from remote file manager
  cm->responseRequestFilefromFileManager(js);
}

int main() {
  // Spawn sigchld handler
  signal(SIGCHLD, sigchld_handler);
  char rsBuf[512] = {'/0', };

  cm = comManager::getInstance();
  appProcList = appProcessTable::getInstance();

  // Main Loop
  while (1) {
    char rcvMsg[MSGBUFSIZE] = {'\0', };
    // Get message from communication framework
    ssize_t numBytesRcvd = cm->getMsg(rcvMsg);

    if (numBytesRcvd == 0 || numBytesRcvd < 0) {
      printf("peer connection closed, accept again\n");
      cm->closeConnection();
      cm->makeConnection();
      continue;
    }

    printf("Received Msg : %s\n", rcvMsg);
    jsonString js(rcvMsg);

    char msgType[1024];
    strncpy(msgType, js.findValue("type").c_str(), 1024);

    // Branch to handlers corresponding to given message's type
    if (!strcmp(msgType, INSTALLPKG)) {
      onCommandInstallAppPackage(js);
    } else if (!strcmp(msgType, EXEAPP)) {
      onCommandExecuteApp(js);
    } else if (!strcmp(msgType, KILLAPP)) {
      onCommandKillApp(js);
    } else if (!strcmp(msgType, UPDATEAPPINFO)) {
      onCommandUpdateAppInfo(js);
    } else if (!strcmp(msgType, CONFIG_EVENT)) {
      onEventConfigSetting(js);
    } else if (!strcmp(msgType, RUN_NATIVE_CAMERAVIEWER)) {
      onCommandRunNativeCameraViewer(js);
    } else if (!strcmp(msgType, RUN_NATIVE_SENSORVIEWER)) {
      onCommandRunNativeSensorViewer(js);
    } else if (!strcmp(msgType, TERM_NATIVE_CAMERAVIEWER)) {
      onCommandTerminateNativeCameraViewer(js);
    } else if (!strcmp(msgType, TERM_NATIVE_SENSORVIEWER)) {
      onCommandTerminateNativeSensorViewer(js);
    } else if (!strcmp(msgType, ANDROID_TERMINATE)) {
      onEventAndroidTerminate(js);
    } else if (!strcmp(msgType, DELETEAPP)) {
      onCommandDeleteApp(js);
    } else if (!strcmp(msgType, RemoteFileManager_getListOfCurPath)) {
      onRFMCommandGetListOfCurrentPaths(js);
    } else if (!strcmp(msgType, RemoteFileManager_requestFile)) {
      onRFMCommandRequestFile(js);
    } else {
      printf("[MAIN] error_not define msg : %s\n", msgType);
    }
  }
}
