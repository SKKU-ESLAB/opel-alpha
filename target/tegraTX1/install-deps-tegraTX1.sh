#!/bin/bash
#
##########################################################################
#
# Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
# 
# Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
##########################################################################
# install-deps-tegraTX1.sh: Script to install dependent packages in
#                           tegraTX1 target board
##########################################################################

# print_progress(): Print the progress
#  - $1(STEP_NUM): (int) Progress step's number
#  - $2(STEP_DESC): (string) Progress step's description
print_progress() {
  STEP_NUM=$1
  STEP_DESC=$2

  WARN_COLO="\033[31;47m"
  INFO_COLO="\033[36m"
  INIT_COLO="\033[0m"
  
  echo -e "${INFO_COLO}Step ${STEP_NUM}. ${STEP_DESC} ${INIT_COLO}"
}

# Step 1. Install packages by apt-get
print_progress 1 "Install dependent packages..."
sudo apt-get update
sudo apt-get install g++-4.8 libdbus-1-dev glib-2.0 libdbus-glib-1-2 \
  libdbus-glib-1-2-dbg libdbus-glib-1-dev zip sqlite3 libsqlite3-dev cmake    \
  libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev git  \
  python-dev python-numpy libjpeg-dev libpng-dev libtiff-dev libjasper-dev    \
  libdc1394-22-dev automake libtool libssl-dev libnl-3-dev libnl-genl-3-dev   \
  python3 udhcpd libv4l-dev libboost-serialization-dev libgstreamer1.0-dev    \
  libgstreamer-plugins-good1.0-dev libgstreamer-plugins-base1.0-dev           \
  libgstreamer-plugins-bad1.0-dev

# Get the absolute path of OPEL repository directory
OPEL_REPO_DIR=$(dirname "$0")/../..
eval OPEL_REPO_DIR=`readlink --canonicalize ${OPEL_REPO_DIR}`

# Step 2. Build and reinstall bluez-4.101
print_progress 2 "Build and reinstall bluez-4.101..."
# Remove existing bluez
sudo apt-get remove bluez
# Install bluez
cd ${OPEL_REPO_DIR}/dep/bluez-4.101
./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc \
  --localstatedir=/var --libexecdir=/lib 
make 
sudo make install

# Step 3. Set udhcpd config
print_progress 3 "Set udhcpd config..."
sudo touch /var/lib/misc/udhcpd.leases

# Step 4. Build and install libxml2-2.9.4-rc2
print_progress 4 "Build and install libxml2-2.9.4-rc2..."
cd ${OPEL_REPO_DIR}/dep/libxml2-2.9.4-rc2
./configure --prefix=/usr/local/xml
make
sudo make install

# Step 5. Build and install opencv-3.0.0
print_progress 5 "Build and install opencv-3.0.0..."
cd /usr/include/linux
sudo ln -s ../libv4l1-videodev.h videodev.h
sudo ln -s /usr/lib/aarch64-linux-gnu/dbus-1.0/include/dbus/dbus-arch-deps.h /usr/include/dbus-1.0/dbus/

cd ${OPEL_REPO_DIR}/dep/opencv-3.0.0
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D \
  BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF ..
make -j 4
sudo make install

# Step 6. Build and install libuv-v1.7.5
print_progress 6 "Build and install libuv-v1.7.5..."
cd ${OPEL_REPO_DIR}/dep/libuv-v1.7.5
sh autogen.sh
./configure
make
make check
sudo make install

# Step 7. Copy dbus config file
print_progress 7 "Copy dbus config file for OPEL..."
sudo cp ${OPEL_REPO_DIR}/dep/opel-dbus-config/opel.conf

# Step 8. Install wpa_supplicant, wpa_cli and deletesem
print_progress 8 "Install wpa_supplicant, wpa_cli and deletesem..."

cd ${OPEL_REPO_DIR}/dep/hostap/wpa_supplicant
make
cd ${OPEL_REPO_DIR}/dep/deletesem
gcc -o deletesem deletesem.c -lpthread

mkdir -p /usr/bin/opel-deps
sudo cp ${OPEL_REPO_DIR}/dep/hostap/wpa_supplicant/wpa_supplicant /usr/bin/opel-deps/
sudo cp ${OPEL_REPO_DIR}/dep/hostap/wpa_supplicant/wpa_cli /usr/bin/opel-deps/
sudo cp ${OPEL_REPO_DIR}/dep/deletesem/deletesem /usr/bin/opel-deps/
chmod +x /usr/bin/opel-deps/*

# Step 9. Build and install nodejs-4.0.0
print_progress 9 "Build and install nodejs-4.0.0..."
git clone https://github.com/nodejs/node ${OPEL_REPO_DIR}/dep/nodejs-4.0.0 \
  -b v4.0.0 --depth=1
cd ${OPEL_REPO_DIR}/dep/nodejs-4.0.0
./configure
make -j4
sudo make install

sudo npm install -g node-gyp

# Step 10. Install nan package 
cd ${OPEL_REPO_DIR}
npm install nan

WARN_COLO="\033[31;47m"
INFO_COLO="\033[36m"
INIT_COLO="\033[0m"

echo -e "${WARN_COLO}Prerequisites install for TegraTX1 completed${INIT_COLO}"
