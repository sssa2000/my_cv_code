// test_cv_algo.cpp : �������̨Ӧ�ó������ڵ㡣
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
	//��ȡ�ı���С�ͻ��� 
	int baseline = 0;
	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;
	// Ϊ��/�һ���/�µ���У������
	if (coord.y >= 0) {
		//ͼ�����Ͻǵ��ı������Ͻǵ����꣬��˰��������ƶ�
		coord.y += textSize.height;
	}
	else {
		//ͼ�����½ǵ��ı������½ǵ����꣬��˴ӵײ�����
		coord.y += img.rows - baseline + 1;
	}
	// ���ϣ������Ҳ����
	if (coord.x < 0) {
		coord.x += img.cols - textSize.width + 1;
	}

	// ��ȡ�ı��ı߽����
	Rect boundingRect = Rect(coord.x, coord.y - textSize.height, textSize.width, baseline + textSize.height);

	// ����ƽ�����ı�
	putText(img, text, coord, fontFace, fontScale, color, thickness, CV_AA);
	return boundingRect;
}

Rect drawButton(Mat img, const char* text, cv::Point coord, int minWidth = 0)
{
	int B = 10;
	Point textCoord = Point(coord.x + B, coord.y + B);
	// ��ȡ�ı��߽����
	Rect rcText = drawTextLine(img, text, textCoord, CV_RGB(0, 0, 0));
	// ���ı���Χ��һ�����ľ���
	Rect rcButton = Rect(rcText.x - B, rcText.y - B, rcText.width + 2 * B, rcText.height + 2 * B);
	// ���ð�ť����С���
	if (rcButton.width < minWidth)
		rcButton.width = minWidth;
	// ����һ����͸Ĥ�İ�ɫ����
	Mat matButton = img(rcButton);
	matButton += CV_RGB(90, 90, 90);
	//��һ����͸���İ�ɫ�߽� 
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

