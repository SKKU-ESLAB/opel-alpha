# opel-alpha
##RPI3 Build Sequence (Test Done)
###1. 본 Repository로부터 raspberry-pi2_3 branch 다운로드
###2. Dependency libarary 설치
```
$apt-get install g++-4.8 wiringpi libdbus-1-dev glib-2.0 libdbus-glib-1-2 libdbus-glib-1-2-dbg libdbus-glib-1-dev zip sqlite3 libsqlite3-dev cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev 
```
###3. bluez 설치
```
$cd dep/bluez-4.101
$./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --libexecdir=/lib 
$make
$sudo make install
```
###4. libxml 설치
```
$cd dep/libxml2-2.7.7
$./configure --prefix=/usr/local/xml
$make
$sudo make install
```
###5. udhcpd 설치
```
$sudo apt-get install udhcpd
$sudo touch /var/lib/misc/udhcpd.leases
```
###6. opel.conf 복사
```
$cd dep/
$sudo cp opel.conf /etc/dbus-1/system.d/
```
###7. build
```
$cmake ./
$make
```
###8. pairing
```
$./pairing
Android에서 BT및 WFD연결
```
###9. Daemon 실행
```
$./run_daemons start
Android OPEL Manager 실행
```

##Directory path
```
/home/pi/opel 					 Workspace (OPEL_DIR)
/home/pi/opel/bin  				 각 Manager의 실행 binary file 
/home/pi/opel/bin/appManager/application	 설치 된 OPEL Application 파일
/home/pi/opel/node 				 NIL API에 관한 코드 및 모듈
/home/pi/opel/opel_#framework 		 각 Framework의 소스코드
/home/pi/opel/dep 				 Dependency Library 파일
```
##Dependency Library 설치
```
apt-get install g++-4.8 wiringpi libdbus-1-dev glib-2.0 libdbus-glib-1-2 libdbus-glib-1-2-dbg libdbus-glib-1-dev zip sqlite3 libsqlite3-dev cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
```
###dependency library (따로 설치 필요 – /OPEL_DIR/dep 디렉토리 내에 위치)
#### bluez 
```
$ cd /OPEL_DIR/dep/bluez-4.101
$ ./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --libexecdir=/lib 
$ make 
$ sudo make install
```
#### udhcpd
```
$sudo apt-get install udhcpd
$sudo touch /var/lib/misc/udhcpd.leases
```

#### hostap/wpa_supplicant
```
$ cd /OPEL_DIR/dep/hostap/wpa_supplicant
$ make
```
####libxml2-2.7.7
```
$ cd /OPEL_DIR/dep/libxml2-2.7.7
$ ./configure --prefix=/usr/local/xml
$ make && make install
```
####opencv-3.0.0
```
$ sudo apt-get install libv4l-dev
$ cd /usr/include/linux
$ sudo ln -s ../libv4l1-videodev.h videodev.h
$ cd /OPEL_DIR/dep/opencv-3.0.0
$ mkdir build
$ cd build
$ cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF ..
$ make -j 4
$ sudo make install
```
####libuv-v1.7.5
```
$ sh autogen.sh
$ ./configure
$ make
$ make check
$ make install
```

##Build 방법
1. OPEL Framework Build 
	* OPEL Directory 내의 makefile을 통해 build
	```
	$ cd /OPEL_DIR
	$ make
	```
2.	OPEL NIL API Build 
	* JS App에 제공되는 OPEL API의 모듈을 생성하는 과정 
	* NIL API에 대한 코드 수정이 필요시에만 빌드 
	```
	$ cd /OPEL_DIR
	$ cd node
	$ cd src    # src내 하위 디렉토리에 Framework별 build.sh 존재
	$ cd sensfw
	$ ./build.sh
	```

3.	Android OPEL Manager Build
	* Android SDK (Android Studio)를 통해 build (Target SDK Version 6.0.2)
	* Android Package File을 통해 설치 (APK 파일 실행)

##OPEL Communication Setup
**최초 연결시 페어링 필요**
###OPEL device
현재 OPEL에서 사용중인 Network Interface는 BCM43143 모델로, brcmfmac (nl80211) driver를 사용합니다.

기타 Realtek 모델 등 nl80211와 호환되지 않는 모델은 현재 지원하지 않습니다. (Raspberry PI3는 별도 모듈없이 바로 사용가능합니다.)
```
$./paring
```
###Android device
* Bluetooth 설정에서 Raspberry-pi 찾아 연결
* Wifi Setting -> Advanced -> Wi-fi Direct -> OPEL 장치 연결
	* PIN: 12345670
	* 이 후, PIN 필요 없고 프로그램 실행 시 자동 연결.

##Program 실행 방법
###OPEL device
* **OPEL Directory 내의 run_daemons [start/stop]으로 데몬들 실행 및 종료**
```
$ cd /OPEL_DIR/
$./run_daemons start	(실행)
$./run_daemons stop	(종료)
```
###Android device
* OPEL Manager Application을 실행하면 자동으로 연결
 

