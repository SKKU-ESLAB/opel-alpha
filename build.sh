#g++ abtGstEncoder.c $(pkg-config --cflags --libs gstreamer-1.0) -lpthread -lgstapp-1.0 -I/home/ubuntu/workspace/2-a-brighter-tomorrow


g++ -c abtMisc.cpp -o abtMisc.o -lptread
g++ -c abtGstEncoder.cpp -o abtGstEncoder.o $(pkg-config --cflags --libs gstreamer-1.0) -lpthread -lgstapp-1.0
g++ -c main.c -o main.o $(pkg-config --cflags --libs gstreamer-1.0) -lpthread -lgstapp-1.0
g++ main.o abtMisc.o abtGstEncoder.o -o main $(pkg-config --cflags --libs gstreamer-1.0) -lpthread -lgstapp-1.0
