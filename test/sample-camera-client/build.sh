#!/bin/bash
g++  OPELdbusInterface.cpp -o test `pkg-config --libs --cflags dbus-1 glib-2.0`

