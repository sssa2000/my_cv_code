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


struct feature_res
{
	vector<KeyPoint> kps;
	Mat descriptors;
};
struct match_input
{
	Mat* image0;
	Mat* image1;
	feature_res* img_kp0;
	feature_res* img_kp1;
};
void test_surf_dect(const char* name,Mat& img0, feature_res& res)
{
	int minHessian = 400;
	Ptr<Feature2D> surf = xfeatures2d::SURF::create(minHessian);


	surf->detectAndCompute(img0, noArray(), res.kps, res.descriptors);
	Mat out_img0;
	cv::drawKeypoints(img0, res.kps, out_img0);
	char buff[256];
	sprintf_s(buff, 256, "SURF,img name=%s,keypoint num=%d",name, res.kps.size());
	drawTextLine(out_img0, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow(name, out_img0);

}

void  test_sift_dect(const char* name, Mat& img0, feature_res& res)
{
	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);

	sift->detectAndCompute(img0, noArray(), res.kps, res.descriptors);
	Mat out_img0;
	cv::drawKeypoints(img0, res.kps, out_img0);
	char buff[256];
	sprintf_s(buff, 256, "SIFT,img name=%s,keypoint num=%d", name, res.kps.size());
	drawTextLine(out_img0, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow(name, out_img0);

}

void test_bruteforce_matcher(const match_input& input)
{
	vector<DMatch> match_res;
	BFMatcher matcher(NORM_L2);
	matcher.match(input.img_kp0->descriptors, input.img_kp1->descriptors, match_res);
	Mat out_img;
	cv::drawMatches(
		*input.image0, input.img_kp0->kps, 
		*input.image1, input.img_kp1->kps, 
		match_res, out_img);

	char buff[256];
	sprintf_s(buff, 256, "BF Match,matched keypoint num=%d", match_res.size());
	drawTextLine(out_img, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow("bf match", out_img);

}

void test_bruteforce_knn_matcher(const match_input& input)
{
	vector<DMatch> match_res;
	vector<vector<DMatch>> knn_matches;
	BFMatcher matcher(NORM_L2);
	matcher.knnMatch(input.img_kp0->descriptors, input.img_kp1->descriptors, knn_matches, 2);

	//��ȡ����Ratio Test����Сƥ��ľ���
	float min_dist = FLT_MAX;
	for (int r = 0; r < knn_matches.size(); ++r)
	{
		//Ratio Test
		if (knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance)
			continue;

		float dist = knn_matches[r][0].distance;
		if (dist < min_dist) min_dist = dist;
	}

	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		//�ų�������Ratio Test�ĵ��ƥ��������ĵ�
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//����ƥ���
		match_res.push_back(knn_matches[r][0]);
	}

	Mat out_img;
	cv::drawMatches(
		*input.image0, input.img_kp0->kps,
		*input.image1, input.img_kp1->kps,
		match_res, out_img);

	char buff[256];
	sprintf_s(buff, 256, "BF knn Match,matched keypoint num=%d", match_res.size());
	drawTextLine(out_img, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow("bf knn match", out_img);

}
int main()
{
	Mat img0 = imread("../media/0004s.jpg");
	Mat img1 = imread("../media/0006s.jpg");
	feature_res surf_kp0;
	test_surf_dect("img0_surf", img0, surf_kp0);
	feature_res surf_kp1;
	test_surf_dect("img1_surf", img1, surf_kp1);
	
	//feature_res sift_kp0 = test_sift_dect("img0_sift", img0);
	//feature_res sift_kp1 = test_sift_dect("img1_sift", img1);

	match_input mipt;
	mipt.image0 = &img0;
	mipt.image1 = &img1;
	mipt.img_kp0 = &surf_kp0;
	mipt.img_kp1 = &surf_kp1;

	test_bruteforce_knn_matcher(mipt);
	waitKey(0);
    return 0;
}

