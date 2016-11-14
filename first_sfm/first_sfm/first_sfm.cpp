// first_sfm.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv/highgui.h"

#pragma comment(lib,"opencv_core310d.lib")
#pragma comment(lib,"opencv_ml310d.lib")
#pragma comment(lib,"opencv_highgui310d.lib")
#pragma comment(lib,"opencv_imgcodecs310d.lib")


int main()
{
	IplImage* img = cvLoadImage("../media/Angry-baby-owl.jpg");
	//cvNamedWindow("window1", CV_WINDOW_AUTOSIZE);
	cvShowImage("first sfm", img);
	cvWaitKey(0);
	cvReleaseImage(&img);
	//cvDestroyWindow("window1");
	return 0;
}

