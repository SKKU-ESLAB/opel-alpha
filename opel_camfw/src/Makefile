test: main.o cam_core.o cam_property.o cam_cap_check.o cam_status.o
	g++ -g -o test main.o cam_core.o cam_property.o cam_cap_check.o cam_status.o `pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1` -pthread
cam_core.o: cam_core.cpp cam_core.h 
	g++ -g -c cam_core.cpp cam_core.h 
main.o: main.cpp 
	g++ -g -c main.cpp `pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1`
cam_property.o: cam_property.cpp cam_property.h cam_core.h
	g++ -g -c cam_property.cpp cam_property.h cam_core.h
#cam_record.o: cam_record.cpp cam_core.h
#	g++ -g -c cam_record.cpp cam_core.h
cam_status.o: cam_status.cpp cam_status.h
	g++ -g -c cam_status.cpp cam_status.h
cam_cap_check.o: cam_cap_check.cpp
	g++ -g -c cam_cap_check.cpp
clean:
	rm -f test *.o *.gch *.swp
