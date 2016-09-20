# opel-alpha

##Supported Hardware
###Android Side
####Nexus 5 (Google reference device) Marshmallow
<img src="https://upload.wikimedia.org/wikipedia/commons/e/ed/Nexus_5_(1).jpg" width=20%>
<img src="https://s.aolcdn.com/hss/storage/midas/965fc84051391e2ed53ce19881178931/202486815/10+am+Official+Announce_G_FB+copy.jpg" width=20%>
###OPELTarget Board
####Raspberry-pi 2 
#####Device
<a href="https://www.raspberrypi.org/products/raspberry-pi-2-model-b/">
<img src="https://www.raspberrypi.org/wp-content/uploads/2015/01/Pi2ModB1GB_-comp.jpeg" width=20%">
</a>
#####Peripherals
 + Bluetooth dongle  
<a href="https://www.eunicorn.co.kr/kimsboard7/_product.php?inc=xu400b">
<img src="http://image.auction.co.kr/itemimage/d5/ee/1b/d5ee1be25.jpg" width=20%>
</a>
 + Wi-fi dongle  
<a href="https://www.adafruit.com/product/2638">
<img src="https://cdn-shop.adafruit.com/970x728/2638-01.jpg" width=20%>
</a> **or** <a href="https://www.pi-supply.com/product/broadcom-wifi-adapter-2-port-usb-hub-raspberry-pi/">
<img src="http://www.broadcom.com/blog/wp-content/uploads/2014/08/50789_wwwHero-WICED_Pi_BlogImg.jpg" width=20%>
</a>  

> [Release File Download](http://nyx.skku.ac.kr/opel/160711.zip)
> 
	opel_pi.img : 저희가 사용하는 Raspberry-Pi2의 전체 Image 파일입니다. (16GB이상의 SD Card를 사용하여 이미지를 구우시면 됩니다.)
	opel-Manager_android.zip : Android Project 입니다.
	opel-pi2.tar.gz : OPEL Code와 Dependency가 있는 Third-party 묶음입니다.
	README.docx : 저희가 작성한 Build guideline이며 해당 내용을 보시면 build 부터 어떻게 실행하는지에 대한 모든 내용이 상세하게 설명되어있습니다.

####Raspberry-pi 3
#####Device
<a href="https://www.raspberrypi.org/products/raspberry-pi-3-model-b/">
<img src="https://www.raspberrypi.org/wp-content/uploads/2016/03/pi3.jpg" width=20%>
</a>
> [Release Image Download](http://nyx.skku.ac.kr/opel/160713opel_rpi3.zip)  
>
	$cd ~/workspace/opel_alpha
	$cmake ./
	$make

----

##RPI3 Build Sequence (Test Done)
#####1. 본 Repository로부터 raspberry-pi2_3 branch 다운로드
#####2. Dependency libarary 설치

	$sudo apt-get update
	$sudo apt-get install g++-4.8 wiringpi libdbus-1-dev glib-2.0 libdbus-glib-1-2 libdbus-glib-1-2-dbg libdbus-glib-1-dev zip sqlite3 libsqlite3-dev cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev automake libtool

#####3. bluez 설치

	$cd dep/bluez-4.101
	$./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --libexecdir=/lib 
	$make
	$sudo make install

#####4. libxml 설치

	$cd dep/libxml2-2.7.7
	$./configure --prefix=/usr/local/xml
	$make
	$sudo make install

#####5. udhcpd 설치

	$sudo apt-get install udhcpd
	$sudo touch /var/lib/misc/udhcpd.leases
	
#####6. libuv 설치
	
	$cd dep/libuv-v1.7.5/
	$./autogen.sh
	$./configure
	$make
	$sudo make install
	$sudo ldconfig

#####7. opel.conf 복사

	$cd dep/
	$sudo cp opel.conf /etc/dbus-1/system.d/

#####8. build

	$cmake ./
	$make

#####9. Daemon 실행
	$./run_daemons start
	Android OPEL Manager 실행

---

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
$sudo apt-get update
$sudo apt-get install g++-4.8 wiringpi libdbus-1-dev glib-2.0 libdbus-glib-1-2 libdbus-glib-1-2-dbg libdbus-glib-1-dev zip sqlite3 libsqlite3-dev cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev automake libtool
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
 

