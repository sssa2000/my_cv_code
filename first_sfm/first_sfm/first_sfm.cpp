// first_sfm.cpp : 定义控制台应用程序的入口点。
//



#include <opencv\highgui.h>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>

#include "util.h"
#include "viewer_serialization.h"
#include <windows.h>


using namespace cv;
using namespace std;

fun_contex g_fun_contex;

#define FUN_TIMER fun_timer_obj obj(__FUNCTION__,&g_fun_contex)

//保存一张图片的数据和特征点信息
struct image_info
{
	image_info(const char* image_fn)
	{
		img = imread(image_fn);
	}
	Mat img;
	std::vector<KeyPoint> key_points;
	std::vector<Vec3b> colors;
	Mat descriptors;
};

struct match_res
{
	vector<Point2f> matched_pos_left;
	vector<Point2f> matched_pos_right;
	vector<Vec3b> matched_color_left;
	vector<Vec3b> matched_color_right;
	std::vector<DMatch> bestMatch; //for debug
};

struct essMatRes
{
	essMatRes(bool b)
	{
		bOk = b;
	}
	bool bOk;	// 本结构是否合法
	Mat R;		// 相机的旋转
	Mat T;		// 相机的位移
	Mat mask;	// mask中大于零的点代表匹配点，等于零代表失配点
};


match_res match_feature(image_info * imgs)
{
	FUN_TIMER;
	match_res res;
	//两幅图片的特征点 进行特征匹配
	std::vector<std::vector<DMatch>> knn_matches;
	BFMatcher matcher(NORM_L2); //暴力求解
	matcher.knnMatch(imgs[0].descriptors, imgs[1].descriptors, knn_matches, 2);

	//获取满足Ratio Test的最小匹配的距离
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
		const cv::DMatch& bestMatch = knn_matches[r][0];
		const cv::DMatch& betterMatch = knn_matches[r][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		//排除不满足Ratio Test的点和匹配距离过大的点
		if (
			distanceRatio > 0.6f ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//把匹配的点的位置和颜色排列好
		int idxL = knn_matches[r][0].queryIdx;
		int idxR= knn_matches[r][0].trainIdx;
		res.matched_pos_left.push_back(imgs[0].key_points[idxL].pt);
		res.matched_color_left.push_back(imgs[0].colors[idxL]);
		
		res.matched_pos_right.push_back(imgs[1].key_points[idxR].pt);
		res.matched_color_right.push_back(imgs[1].colors[idxR]);

		res.bestMatch.push_back(bestMatch);
	}
	return res;
}

void search_feature(image_info * img_array)
{
	FUN_TIMER;

	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	for (int i = 0; i < 2;++i)
	{
		image_info& image = img_array[i];
		sift->detectAndCompute(image.img, noArray(), image.key_points, image.descriptors);

		//特征点过少，则排除该图像
		if (image.key_points.size() <= 10)
			continue;

		//将每个keypoint的颜色保存起来
		for (auto& kp : image.key_points)
		{
			image.colors.push_back(image.img.at<Vec3b>(kp.pt.y, kp.pt.x));
		}
	}
}

essMatRes slovePosRotFromE(const Mat& K, const match_res& mr)
{
	FUN_TIMER;
	essMatRes res(true);
	//根据内参矩阵获取相机的焦距和光心坐标（主点坐标）
	double focal_length = 0.5*(K.at<double>(0) + K.at<double>(4));
	Point2d principle_point(K.at<double>(2), K.at<double>(5));

	//根据匹配点求取本征矩阵，使用RANSAC，进一步排除失配点
	Mat E = findEssentialMat(mr.matched_pos_left, mr.matched_pos_right, focal_length, principle_point, RANSAC, 0.999, 1.0, res.mask);
	if (E.empty()) 
		return essMatRes(false);

	double feasible_count = countNonZero(res.mask);
	std::cout << (int)feasible_count << " -in- " << mr.matched_pos_left.size() << endl;
	
	//对于RANSAC而言，outlier数量大于50%时，结果是不可靠的
	if (feasible_count <= 15 || (feasible_count / mr.matched_pos_left.size()) < 0.6)
		return essMatRes(false);

	//分解本征矩阵，获取相对变换
	int pass_count = recoverPose(E, mr.matched_pos_left, mr.matched_pos_right, res.R, res.T, focal_length, principle_point, res.mask);

	//位于两个相机前方的点的数量要足够大
	if (((double)pass_count) / feasible_count < 0.7)
		return essMatRes(false);

	return res;
}

void reconstruct(const Mat& K, const essMatRes& emr, const match_res& mr, Mat& structure)
{
	FUN_TIMER;
	//两个相机的投影矩阵[R T]，triangulatePoints只支持float型
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

	//三角重建
	cv::triangulatePoints(proj1, proj2, mr.matched_pos_left, mr.matched_pos_right, structure);
}

//根据mask的值把不用的点和颜色剔除掉
//返回剔除掉了多少个点
int cull_point_by_mask(const essMatRes& emr,match_res& mr)
{
	FUN_TIMER;
	int count = emr.mask.rows; //少写一个else
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

// Draw matches between two images
cv::Mat genMatchesImage(cv::Mat query, cv::Mat pattern, const std::vector<cv::KeyPoint>& queryKp, const std::vector<cv::KeyPoint>& trainKp, std::vector<cv::DMatch> matches, int maxMatchesDrawn)
{
	cv::Mat outImg;

	if (matches.size() > maxMatchesDrawn)
	{
		matches.resize(maxMatchesDrawn);
	}

	cv::drawMatches
		(
			query,
			queryKp,
			pattern,
			trainKp,
			matches,
			outImg,
			cv::Scalar(0, 200, 0, 255),
			cv::Scalar::all(-1),
			std::vector<char>(),
			cv::DrawMatchesFlags::DEFAULT
			);

	return outImg;
}

bool first_sfm()
{
	FUN_TIMER;

	//读取两张图片
	image_info imgs[] = {
		image_info("../media/0004s.jpg"),
		image_info("../media/0006s.jpg")
	};
	
	//camera的内部矩阵
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));

	//提取每个图片的特征点
	search_feature(imgs);

	//匹配两个图片的特征点
	match_res mr=match_feature(imgs);
	//draw debug
	cv::Mat debugImg=genMatchesImage(imgs[0].img, imgs[1].img,
		imgs[0].key_points, imgs[1].key_points,
		mr.bestMatch, 500);
	cv::imshow("match result debug view", debugImg);
	//int desktopw = GetSystemMetrics(SM_CXSCREEN);
	//int desktoph = GetSystemMetrics(SM_CYSCREEN);
	//cv::resizeWindow("match result debug view", desktopw, desktoph);


	//根据匹配的特征点 求本质矩阵E以及相机的R和T
	essMatRes ert = slovePosRotFromE(K, mr);

	//重建三维空间的位置 使用三角重建法
	Mat structure;	//4行N列的矩阵，每一列代表空间中的一个点
	int cull_count=cull_point_by_mask(ert, mr);
	reconstruct(K, ert, mr, structure);

	//save to file
	vector<Mat> rotations = { Mat::eye(3, 3, CV_64FC1), ert.R };
	vector<Mat> motions = { Mat::zeros(3, 1, CV_64FC1), ert.T };
	save_structure("structure.yml", rotations, motions, structure, mr.matched_color_left);

	
	return true;
}
