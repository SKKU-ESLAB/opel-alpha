#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <limits.h>
using namespace std;
using namespace cv;

int main(int argc, char** argv){
    time_t start, end;
		int counter = 0;
		double sec;
		double fps;
		VideoCapture cap(-1);
		if (!cap.isOpened())
    {
        cout << "Cannot Open Camera" << endl;
        return -1;
    }
    while (1)
    {
				if(counter == 0)
				{
								time(&start);
				}
        Mat frame;
        bool bSuccess = cap.read(frame);
				if(!bSuccess)
				{
								cout << "Cannot read a frame from camera" <<endl;
								break;
				}
				time(&end);
				counter++;
				sec = difftime(end, start);
				fps = counter/sec;
				if(counter > 30)
								printf("%.2f \n", fps);
				if(counter == (INT_MAX - 1000))
								counter = 0;
 				
				//      imshow("Output", frame);

        if (waitKey(30) == 27)
        {
        cout << "Exit" << endl;
        break;
        }
    }
    return 0;
}
