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
        Mat frame;
        bool bSuccess = cap.read(frame);
				imshow("result", frame);
		}
    
    return 0;
}
