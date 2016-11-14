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

int main()
{
	//��ȡ����ͼƬ
	Mat imgs[] = {
	imread("../media/0004.jpg"),
	imread("../media/0006.jpg")
	};

	//д���ڲ�����
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));

	//��ȡ����
	feaure_key_point.clear();
	descriptor_for_all.clear();

	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	for(auto& image: imgs)
	{
		std::vector<KeyPoint> key_points;
		Mat descriptor;
		sift->detectAndCompute(image, noArray(), key_points, descriptor);

		//��������٣����ų���ͼ��
		if (key_points.size() <= 10) 
			continue;

		key_points_for_all.push_back(key_points);
		descriptor_for_all.push_back(descriptor);

		std::vector<Vec3b> colors(key_points.size());
		for (int i = 0; i < key_points.size(); ++i)
		{
			Point2f& p = key_points[i].pt;
			colors[i] = image.at<Vec3b>(p.y, p.x);
		}
		colors_for_all.push_back(colors);
	}
	return 0;
}

