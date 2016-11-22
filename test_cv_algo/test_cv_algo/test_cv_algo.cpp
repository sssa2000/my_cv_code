// test_cv_algo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <opencv\cv.hpp>
#include <opencv\highgui.h>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include "opencv2/core/core.hpp"  
//#include "opencv2/nonfree/features2d.hpp"
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

Rect drawTextLine(Mat img, const char* text, cv::Point coord, cv::Scalar color, float fontScale = 0.6f, int thickness = 1, int fontFace = FONT_HERSHEY_COMPLEX)
{
	//获取文本大小和基线 
	int baseline = 0;
	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;
	// 为左/右或上/下调整校正坐标
	if (coord.y >= 0) {
		//图像左上角的文本的左上角的坐标，因此按行向下移动
		coord.y += textSize.height;
	}
	else {
		//图像左下角的文本的左下角的坐标，因此从底部上来
		coord.y += img.rows - baseline + 1;
	}
	// 如果希望变成右侧调整
	if (coord.x < 0) {
		coord.x += img.cols - textSize.width + 1;
	}

	// 获取文本的边界矩形
	Rect boundingRect = Rect(coord.x, coord.y - textSize.height, textSize.width, baseline + textSize.height);

	// 画出平滑的文本
	putText(img, text, coord, fontFace, fontScale, color, thickness, CV_AA);
	return boundingRect;
}

Rect drawButton(Mat img, const char* text, cv::Point coord, int minWidth = 0)
{
	int B = 10;
	Point textCoord = Point(coord.x + B, coord.y + B);
	// 获取文本边界矩形
	Rect rcText = drawTextLine(img, text, textCoord, CV_RGB(0, 0, 0));
	// 在文本周围画一个填充的矩形
	Rect rcButton = Rect(rcText.x - B, rcText.y - B, rcText.width + 2 * B, rcText.height + 2 * B);
	// 设置按钮的最小宽度
	if (rcButton.width < minWidth)
		rcButton.width = minWidth;
	// 创建一个半透膜的白色矩形
	Mat matButton = img(rcButton);
	matButton += CV_RGB(90, 90, 90);
	//画一个非透明的白色边界 
	cv::rectangle(img, rcButton, CV_RGB(200, 200, 200), 1, CV_AA);

	drawTextLine(img, text, textCoord, CV_RGB(10, 55, 20));

	return rcButton;
}


void test_surf_dect(const char* name,Mat& img0)
{
	int minHessian = 400;
	Ptr<Feature2D> surf = xfeatures2d::SURF::create(minHessian);
	vector<KeyPoint> kps;
	Mat descriptors;
	surf->detectAndCompute(img0, noArray(), kps, descriptors);
	Mat out_img0;
	cv::drawKeypoints(img0, kps, out_img0);
	char buff[256];
	sprintf_s(buff, 256, "SURF,img name=%s,keypoint num=%d",name,kps.size());
	drawTextLine(out_img0, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow(name, out_img0);

}

void test_sift_dect(const char* name, Mat& img0)
{
	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	vector<KeyPoint> kps;
	Mat descriptors;
	sift->detectAndCompute(img0, noArray(), kps, descriptors);
	Mat out_img0;
	cv::drawKeypoints(img0, kps, out_img0);
	char buff[256];
	sprintf_s(buff, 256, "SIFT,img name=%s,keypoint num=%d", name, kps.size());
	drawTextLine(out_img0, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow(name, out_img0);

}
int main()
{
	Mat img0 = imread("../media/0004s.jpg");
	Mat img1 = imread("../media/0006s.jpg");
	test_surf_dect("img0_surf",img0);
	test_surf_dect("img1_surf",img1);
	
	test_sift_dect("img0_sift", img0);
	test_sift_dect("img1_sift", img1);

	
	waitKey(0);
    return 0;
}

