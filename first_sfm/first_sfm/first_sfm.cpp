// first_sfm.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "opencv/highgui.h"
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>

#pragma comment(lib,"opencv_core310d.lib")
#pragma comment(lib,"opencv_ml310d.lib")
#pragma comment(lib,"opencv_highgui310d.lib")
#pragma comment(lib,"opencv_imgcodecs310d.lib")

using namespace cv;
using namespace std;
//����һ��ͼƬ�����ݺ���������Ϣ
struct image_info
{
	image_info(const char* image_fn)
	{
		img = imread(image_fn);
	}
	Mat img;
	std::vector<KeyPoint> key_points;
	std::vector<Vec3b> colors;
	std::vector<Mat> descriptors;
};

struct match_res
{
	vector<Point2f> matched_pos_left;
	vector<Point2f> matched_pos_right;
	vector<Vec3b> matched_color_left;
	vector<Vec3b> matched_color_right;
};
match_res match_feature(image_info * imgs)
{
	match_res res;
	//����ͼƬ�������� ��������ƥ��
	std::vector<std::vector<DMatch>> knn_matches;
	BFMatcher matcher(NORM_L2); //�������
	matcher.knnMatch(imgs[0].descriptors, imgs[1].descriptors, knn_matches, 2);

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

		//��ƥ��ĵ��λ�ú���ɫ���к�
		int idxL = knn_matches[r][0].queryIdx;
		int idxR= knn_matches[r][0].trainIdx;
		res.matched_pos_left.push_back(imgs[0].key_points[idxL].pt);
		res.matched_color_left.push_back(imgs[0].colors[idxL]);
		
		res.matched_pos_right.push_back(imgs[1].key_points[idxR].pt);
		res.matched_color_right.push_back(imgs[1].colors[idxR]);
	}
	return res;
}

void search_feature(image_info * imgs)
{
	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	for (int i = 0; i < 2;++i)
	{
		image_info& image = imgs[i];
		sift->detectAndCompute(image.img, noArray(), image.key_points, image.descriptors);

		//��������٣����ų���ͼ��
		if (image.key_points.size() <= 10)
			continue;

		//��ÿ��keypoint����ɫ��������
		for (auto& kp : image.key_points)
		{
			image.colors.push_back(image.img.at<Vec3b>(kp.pt.y, kp.pt.x));
		}
	}
}
struct essMatRes
{
	essMatRes(bool b)
	{
		bOk = b;
	}
	bool bOk;	// ���ṹ�Ƿ�Ϸ�
	Mat R;		// �������ת
	Mat T;		// �����λ��
	Mat mask;	// mask�д�����ĵ����ƥ��㣬���������ʧ���
};
essMatRes slovePosRotFromE(const Mat& K, const match_res& mr)
{
	essMatRes res(true);
	//�����ڲξ����ȡ����Ľ���͹������꣨�������꣩
	double focal_length = 0.5*(K.at<double>(0) + K.at<double>(4));
	Point2d principle_point(K.at<double>(2), K.at<double>(5));

	//����ƥ�����ȡ��������ʹ��RANSAC����һ���ų�ʧ���
	Mat E = findEssentialMat(mr.matched_pos_left, mr.matched_pos_right, focal_length, principle_point, RANSAC, 0.999, 1.0, res.mask);
	if (E.empty()) 
		return essMatRes(false);

	double feasible_count = countNonZero(res.mask);
	std::cout << (int)feasible_count << " -in- " << mr.matched_pos_left.size() << endl;
	
	//����RANSAC���ԣ�outlier��������50%ʱ������ǲ��ɿ���
	if (feasible_count <= 15 || (feasible_count / mr.matched_pos_left.size()) < 0.6)
		return essMatRes(false);

	//�ֽⱾ�����󣬻�ȡ��Ա任
	int pass_count = recoverPose(E, mr.matched_pos_left, mr.matched_pos_right, res.R, res.T, focal_length, principle_point, res.mask);

	//λ���������ǰ���ĵ������Ҫ�㹻��
	if (((double)pass_count) / feasible_count < 0.7)
		return essMatRes(false);

	return res;
}

void reconstruct(const Mat& K, const essMatRes& emr, const match_res& mr, Mat& structure)
{
	//���������ͶӰ����[R T]��triangulatePointsֻ֧��float��
	Mat proj1(3, 4, CV_32FC1);
	Mat proj2(3, 4, CV_32FC1);

	proj1(Range(0, 3), Range(0, 3)) = Mat::eye(3, 3, CV_32FC1);
	proj1.col(3) = Mat::zeros(3, 1, CV_32FC1);

	emr.R.convertTo(proj2(Range(0, 3), Range(0, 3)), CV_32FC1);
	emr.T.convertTo(proj2.col(3), CV_32FC1);

	Mat fK;
	K.convertTo(fK, CV_32FC1);
	proj1 = fK*proj1;
	proj2 = fK*proj2;

	//�����ؽ�
	cv::triangulatePoints(proj1, proj2, mr.matched_pos_left, mr.matched_pos_right, structure);
}

//����mask��ֵ�Ѳ��õĵ����ɫ�޳���
//�����޳����˶��ٸ���
int cull_point_by_mask(const essMatRes& emr,match_res& mr)
{
	int count = emr.mask.rows; //��дһ��else
	vector<Point2f> left_copy;
	vector<Point2f> right_copy;
	vector<Vec3b> left_col_copy;
	vector<Vec3b> right_col_copy;

	for (int i = 0; i < emr.mask.rows; ++i)
	{
		if (emr.mask.at<uchar>(i) > 0)
		{
			count -= 1;
			left_copy.push_back(mr.matched_pos_left[i]);
			right_copy.push_back(mr.matched_pos_right[i]);
			left_col_copy.push_back(mr.matched_color_left[i]);
			right_col_copy.push_back(mr.matched_color_right[i]);
		}
	}
	if (count > 0)
	{
		mr.matched_pos_left.swap(left_copy);
		mr.matched_pos_right.swap(right_copy);
		mr.matched_color_left.swap(left_col_copy);
		mr.matched_color_right.swap(right_col_copy);
	}

	return count;
}
int main()
{
	//��ȡ����ͼƬ
	image_info imgs[] = {
		image_info("../media/0004.jpg"),
		image_info("../media/0006.jpg")
	};
	
	//camera���ڲ�����
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));

	//��ȡÿ��ͼƬ��������
	search_feature(imgs);

	//ƥ������ͼƬ��������
	match_res mr=match_feature(imgs);

	//����ƥ��������� ���ʾ���E�Լ������R��T
	essMatRes ert = slovePosRotFromE(K, mr);

	//�ؽ���ά�ռ��λ�� ʹ�������ؽ���
	Mat structure;	//4��N�еľ���ÿһ�д���ռ��е�һ����
	int cull_count=cull_point_by_mask(ert, mr);
	reconstruct(K, ert, mr, structure);
	return 0;
}

