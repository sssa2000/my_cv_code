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



bool sloveE(MvSfmContex* contex,int image_idx0,int image_idx1)
{
	FUN_TIMER;
	//�����ڲξ����ȡ����Ľ���͹������꣨�������꣩
	double f=contex->GetCameraFocal();
	Point2d pp=contex->GetCameraPrinP();
	
	//����ƥ�����ȡ��������ʹ��RANSAC����һ���ų�ʧ���
	match_res* mr = contex->GetMatchData(image_idx0, image_idx1);
	essMatrixRes& out = mr->GetEMatRes();
	out.essMatrix = findEssentialMat(
		mr->GetPosData(0), mr->GetPosData(1), 
		f, pp, RANSAC, 0.999, 1.0, 
		out.mask);

	//mask�� 0��ʾ�쳣�ĵ� 1��ʾ�����ĵ�
	out.feasible_count = countNonZero(out.mask);
	//����RANSAC���ԣ�outlier��������50%ʱ������ǲ��ɿ���
	if (out.feasible_count <= 15 ||
		((float)out.feasible_count / mr->GetPosData(0).size()) < 0.6f)
		return (false);
	return true;
}
void tri_reconstruct(MvSfmContex* contex,const Mat& R,const Mat& T,Mat& res)
{
	FUN_TIMER;
	//���������ͶӰ����[R T]��triangulatePointsֻ֧��float��
	Mat proj1(3, 4, CV_32FC1); //CV_32FC1 ��ʾ �����ÿ��Ԫ����float32�������float��ʾһ��ͨ��
	proj1(Range(0, 3), Range(0, 3)) = Mat::eye(3, 3, CV_32FC1);
	proj1.col(3) = Mat::zeros(3, 1, CV_32FC1);
	
	Mat proj2(3, 4, CV_32FC1);
	R.convertTo(proj2(Range(0, 3), Range(0, 3)), CV_32FC1);
	T.convertTo(proj2.col(3), CV_32FC1);

	const Mat& fK = contex->GetCameraK();
	proj1 = fK*proj1;
	proj2 = fK*proj2;

	//�����ؽ�
	cv::triangulatePoints(proj1, proj2, 
		contex->GetMatchData(0, 1)->GetCulledPosData(0),
		contex->GetMatchData(0, 1)->GetCulledPosData(1),
		res);

}

//init reconstruct use first and second image
bool reconstruct_fs(MvSfmContex* contex)
{
	contex->ResetAllFeaPoint();

	//����ͷ������Ƭ���E
	match_res* mr=contex->GetMatchData(0, 1);
	essMatrixRes& res= mr->GetEMatRes();
	bool b=sloveE(contex,0,1);
	
	//�ֽⱾ�����󣬻�ȡ��Ա任
	Mat& R = mr->GetCameraR();
	Mat& T = mr->GetCameraT();
	int pass_count = cv::recoverPose(res.essMatrix, 
		mr->GetPosData(0), 
		mr->GetPosData(1), 
		R, 
		T, 
		contex->GetCameraFocal(), 
		contex->GetCameraPrinP(), 
		res.mask);

	//λ���������ǰ���ĵ������Ҫ�㹻��
	if (((float)pass_count) / res.feasible_count < 0.7f)
		return false;

	//�ؽ���ά�ռ��λ�� ʹ�������ؽ���
	Mat structure;	//4��N�еľ���ÿһ�д���ռ��е�һ����
	int cull_count = contex->GetMatchData(0, 1)->CullByMask(res.mask);
	tri_reconstruct(contex,R,T,structure);

	//�Ǽ� ��Ӧ��ϵ
	contex->FusionResult1st(0, 1);

	return true;
}
//���
#ifndef LAUNCH_GTEST
bool sfm_mv()
#else
TEST(sfm_mv, all_exec)
#endif
{
	FUN_TIMER;
	MvSfmContex contex;




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

	//���ͷ���Ž����ؽ�
	reconstruct_fs(contex);

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