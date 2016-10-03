#bin/bash

gcc -shared -o libsensor.so sensor0_button.o
gcc `pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1` -lwiringPi -pthread -lc -lm -w -Wl,-rpath=/home/pi/opel-alpha/opel_sensor_manager -L/home/pi/opel-alpha/opel_sensor_manager -o test sensor_manager.c devices.c request.c -lsensor -I./


