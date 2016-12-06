#include "sfm_mv.h"


#include <opencv\highgui.h>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <memory>
#include "util.h"
#include "viewer_serialization.h"
#include "gtest/gtest.h"
#include <windows.h>

#include "MvSfmContex.h"

using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;
//��¼������ʱ��contex
fun_contex g_fun_contex;

//���ڼ�¼����ʱ��
#define FUN_TIMER fun_timer_obj obj(__FUNCTION__,&g_fun_contex)


void calc_trans_rot_pnp()
{
	//�ҵ� idx��idx+1 ��ƥ��Ľ�����ҵ����¶�Ӧ��ϵ��
	//��idxͼƬ��ƥ��㡢��λ��  ��===�� ��idx+1 ͼƬ�����ص�

	//PnP���任����

	//����ת����ת��Ϊ��ת����
}

//��ȡ����ͼƬ�ļ� ������������
void analyse_signle_image(MvSfmContex* contex,size_t idx)
{
	FUN_TIMER;
	image_info& image =*contex->GetImageByIdx(idx);

	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	sift->detectAndCompute(image.img, noArray(), image.key_points, image.descriptors);

	//��������٣��ų���ͼ��
	if (image.key_points.size() <= 10)
	{
		//todo :error
	}

	//��ÿ��keypoint����ɫ��������
	for (auto& kp : image.key_points)
	{
		image.colors.push_back(image.img.at<Vec3b>(kp.pt.y, kp.pt.x));
	}
	
}

//�����ļ��������е�ͼƬ�ļ� ��ȡ ������������
//���ض�ȡ�˶�����ͼƬ
int read_analyse_images(MvSfmContex* contex, const char* image_dir)
{
	FUN_TIMER;
	int idx = 0;
	contex->BeginAddImage();
	for (auto& p : fs::directory_iterator(image_dir))
	{
		auto _path = p.path();
		if (_path.extension() == ".png" ||
			_path.extension() == ".jpg")
		{
			contex->AddImageFromFile(_path.string().c_str());
			analyse_signle_image(contex, idx++);
		}
	}
	contex->EndAddImage();
	return idx;
}
//����ƥ��ɹ��Ĺؼ��������
int match_feature(image_info * img0,image_info* img1, match_res* res)
{
	FUN_TIMER;
	BFMatcher matcher(NORM_L2); //�������
	std::vector<std::vector<DMatch>> knn_matches;
	//knnMatch K = 2 ������ÿ��ƥ�䷵�������������������������һ��ƥ����ڶ���ƥ��֮��ľ����㹻Сʱ������Ϊ����һ��ƥ�䡣
	matcher.knnMatch(img0->descriptors, img1->descriptors, knn_matches, 2);
	int succNum = 0;
	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		const cv::DMatch& bestMatch = knn_matches[r][0];
		const cv::DMatch& betterMatch = knn_matches[r][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		//�ų�������Ratio Test�ĵ��ƥ��������ĵ�
		if (distanceRatio > 0.6f )
			continue;
		res->AddMatchData(bestMatch);
		++succNum;
	}
	return succNum;
}
void seq_match_feature(MvSfmContex* contex)
{
	FUN_TIMER;
	size_t num=contex->GetImageCount();
	for (size_t i = 0; i < num-1;++i) // num-1
	{
		image_info* img0 = contex->GetImageByIdx(i);
		image_info* img1 = contex->GetImageByIdx(i+1);
		match_res* mr=contex->GetMatchData(i,i+1);
		int sucNum=match_feature(img0, img1,mr);
		
	}
}
//TEST(sfm_mv, read_analyse_images)
//{
//	//ֻ��3��ͼƬ
//	MvSfmContex contex;
//	read_analyse_images(&contex,"../media/mv_images/1");
//	EXPECT_NE(contex.GetImageByIdx(0), nullptr);
//	EXPECT_NE(contex.GetImageByIdx(1), nullptr);
//	EXPECT_NE(contex.GetImageByIdx(2), nullptr);
//	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);
//
//}


//���
#ifndef LAUNCH_GTEST
bool sfm_mv()
#else
TEST(sfm_mv, all_exec)
#endif
{
	FUN_TIMER;
	MvSfmContex contex;

	//д�� camera���ڲ����󣬼����Ѿ������˱궨
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));


	//��ȡһ��Ŀ¼�µ�����image
	int img_num = read_analyse_images(&contex,"../media/mv_images/1");
	
	EXPECT_EQ(img_num, 3);
	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);

	//��������ͼƬ������ģ������ģ�����ЩͼƬ��������ƥ��
	seq_match_feature(&contex);
	EXPECT_GT(contex.GetMatchData(0, 1)->GetMatchedPointCount(), (size_t)0);
	EXPECT_GT(contex.GetMatchData(1, 2)->GetMatchedPointCount(), (size_t)0);
	EXPECT_GT(contex.GetMatchData(2, 3), nullptr);
	EXPECT_EQ(contex.GetMatchData(0, 0)->GetMatchedPointCount(), (size_t)0);
	EXPECT_EQ(contex.GetMatchData(2, 0), nullptr);

	////���ͷ���Ž����ؽ�
	//init_reconstruct();

	////���μ����µ�ͼƬ��idx+1�� �ѽ���ںϵ��ؽ������
	////
	//for (int idx = 1; idx < n;++idx)
	//{
	//	//���� ��idxͼƬ���Ѿ��õ���3d�� �� ��idx+1 ͼƬ�������㣬��ñ任����R��T
	//	calc_trans_rot_pnp();

	//	//����֮ǰ��õ�R��T������ά�ؽ�
	//	reconstruct();

	//	//�ں� ȥ���ظ�
	//	dedup_fusion();


	//}
	//return true;
}