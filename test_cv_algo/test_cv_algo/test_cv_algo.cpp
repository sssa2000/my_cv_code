// test_cv_algo.cpp : 定义控制台应用程序的入口点。
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

	//获取满足Ratio Test的最小匹配的距离
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
		//排除不满足Ratio Test的点和匹配距离过大的点
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//保存匹配点
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
	std::set<int> found_in_imgpts_j; // 去重
	
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

		// 去重
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

