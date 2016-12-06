// test_cv_algo.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <opencv\cv.hpp>
#include <opencv\highgui.h>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include "opencv2/core/core.hpp"  
#include <opencv2/imgproc/imgproc.hpp>

//#include "opencv2/nonfree/features2d.hpp"
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>
#include <set>

#include "util.h"
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
	for (size_t r = 0; r < knn_matches.size(); ++r)
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

void test_opticalflow_detect_match(Mat& img0, Mat& img1)
{
	vector<DMatch> matchesRes;

	//detect keypoints for all images
	Ptr<FastFeatureDetector> ffd = FastFeatureDetector::create();
	std::vector<cv::KeyPoint> kps0,kps1;
	ffd->detect(img0, kps0);
	ffd->detect(img1, kps1);

	// making sure images are grayscale
	Mat prevgray, gray;
	if (img0.channels() == 3) {
		cvtColor(img0, prevgray, CV_RGB2GRAY);
		cvtColor(img1, gray, CV_RGB2GRAY);
	}
	else {
		prevgray = img0;
		gray = img1;
	}

	vector<Point2f> kps0_pts;
	KeyPointsToPoints(kps0, kps0_pts);

	vector<uchar> vstatus(kps0_pts.size());
	vector<float> verror(kps0_pts.size());
	vector<Point2f> of_res(kps0_pts.size()); //calculated new positions of input features in the second image
	calcOpticalFlowPyrLK(prevgray, gray, kps0_pts, of_res, vstatus, verror);

	double thresh = 1.0;
	vector<Point2f> to_find;
	vector<int> to_find_back_idx;
	for (unsigned int i = 0; i < vstatus.size(); i++) {
		if (vstatus[i] && verror[i] < 12.0f) {
			to_find_back_idx.push_back(i);
			to_find.push_back(of_res[i]);
		}
		else {
			vstatus[i] = 0;
		}
	}
	Mat to_find_flat = Mat(to_find).reshape(1, to_find.size());

	vector<Point2f> j_pts_to_find;
	KeyPointsToPoints(kps1, j_pts_to_find);
	Mat j_pts_flat = Mat(j_pts_to_find).reshape(1, j_pts_to_find.size());

	vector<vector<DMatch> > knn_matches;
	BFMatcher matcher(CV_L2);
	matcher.radiusMatch(to_find_flat, j_pts_flat, knn_matches, 2.0f);
	std::set<int> found_in_imgpts_j; // ȥ��
	
	for (int i = 0; i < knn_matches.size(); i++)
	{
		DMatch _m;
		if (knn_matches[i].size() == 1) 
		{
			_m = knn_matches[i][0];
		}
		else if (knn_matches[i].size()>1) 
		{
			if (knn_matches[i][0].distance / knn_matches[i][1].distance < 0.7) 
				_m = knn_matches[i][0];
			else 
				continue;
		}
		else 
			continue; // no match

		// ȥ��
		auto ins_res= found_in_imgpts_j.insert(_m.trainIdx);
		if (ins_res.second)
		{ 
			_m.queryIdx = to_find_back_idx[_m.queryIdx]; //back to original indexing of points for <i_idx>
			matchesRes.push_back(_m);
		}
	}

	Mat out_img;
	cv::drawMatches(img0, kps0,
		img1, kps1,
		matchesRes, out_img);

	char buff[256];
	sprintf_s(buff, 256, "OF Match,matched keypoint num=%d", matchesRes.size());
	drawTextLine(out_img, buff, cv::Point(10, -30), cv::Scalar(255, 255, 255));
	imshow("OF match", out_img);

	//cout << "pruned " << matches->size() << " / " << knn_matches.size() << " matches" << endl;
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
	//test_opticalflow_detect_match(img0, img1);
	waitKey(0);
    return 0;
}

