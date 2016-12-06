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


using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;
//��¼������ʱ��contex
fun_contex g_fun_contex;

//���ڼ�¼����ʱ��
#define FUN_TIMER fun_timer_obj obj(__FUNCTION__,&g_fun_contex)

//����һ��ͼƬ�����ݺ���������Ϣ
struct image_info
{
	image_info(const char* image_fn)
	{
		img = imread(image_fn);
	}
	image_info() {}
	~image_info()
	{

	}
	void reload_data(const char* image_fn)
	{
		img = imread(image_fn);
		key_points.clear();
		descriptors.release();
		colors.clear();
		
	}
	Mat img;
	std::vector<KeyPoint> key_points;
	std::vector<Vec3b> colors;
	Mat descriptors;
};
struct match_res
{
	match_res(image_info* _img0, image_info* _img1)
	{
		img0 = _img0;
		img1 = _img1;
	}
	image_info* img0;
	image_info* img1;
	std::vector<DMatch> bestMatch; //for debug
};
class MvSfmContex
{
public:
	MvSfmContex()
	{

	}

	virtual ~MvSfmContex()
	{

	}
	void ResetContex(size_t imageCount)
	{
		//clean
		size_t n = GetImageCount();
		for (size_t i = 0; i < n; ++i)
				m_match_res[i].clear();

		m_images.clear();

		//����ͼƬ������ ȫ����image_info ��������
		for (size_t i = 0; i < imageCount; ++i)
			m_images.push_back(image_info());

		////����ͼƬ������ ȫ����match_res ������������ά���� ��������ͼƬ������	
		for (size_t i = 0; i < imageCount; ++i)
		{
			vector<match_res> mr_row;
			image_info* img0 = GetImageByIdx(i);
			for (size_t j = 0; j < imageCount; ++j)
			{
				image_info* img1 = GetImageByIdx(j);
				mr_row.push_back(match_res(img0, img1));
			}
		}
	}

	image_info* SetImageData(int idx,const char* fn)
	{
		image_info* p = GetImageByIdx(idx);
		if (p)
			p->reload_data(fn);
		return p;
	}

	image_info* GetImageByIdx(size_t idx)
	{
		if (idx >= m_images.size())
			return 0;

		return &m_images.at(idx);
	}

	size_t GetImageCount() { return m_images.size(); }

	match_res* GetMatchData(int idx0, int idx1)
	{
		if (idx0 >= m_images.size() || idx1 >= m_images.size())
			return 0;
		return &m_match_res[idx0][idx1];
	}
protected:
	vector<image_info> m_images;
	vector<vector<match_res>> m_match_res; //��ά���� ��������ͼƬ����������ʾmatch��ƥ���ϵ ��ʵ����ֻ��Ҫ��һ�������εľ��󼴿�
};


void calc_trans_rot_pnp()
{
	//�ҵ� idx��idx+1 ��ƥ��Ľ�����ҵ����¶�Ӧ��ϵ��
	//��idxͼƬ��ƥ��㡢��λ��  ��===�� ��idx+1 ͼƬ�����ص�

	//PnP���任����

	//����ת����ת��Ϊ��ת����
}

//��ȡ����ͼƬ�ļ� ������������
void read_analyse_image(size_t idx,MvSfmContex* contex, const string& fn)
{
	FUN_TIMER;
	image_info& image =*contex->SetImageData(idx,fn.c_str());

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
void read_analyse_images(MvSfmContex* contex, const char* image_dir)
{
	FUN_TIMER;
	int idx = 0;
	for (auto& p : fs::directory_iterator(image_dir))
	{
		auto _path = p.path();
		if (_path.extension() == ".png" ||
			_path.extension() == ".jpg")
		{
			read_analyse_image(idx++,contex, _path.string());
		}
	}
}
 void match_feature(image_info * img0,image_info* img1, match_res* res)
{
	FUN_TIMER;
	BFMatcher matcher(NORM_L2); //�������
	std::vector<std::vector<DMatch>> knn_matches;
	//knnMatch K = 2 ������ÿ��ƥ�䷵�������������������������һ��ƥ����ڶ���ƥ��֮��ľ����㹻Сʱ������Ϊ����һ��ƥ�䡣
	matcher.knnMatch(img0->descriptors, img1->descriptors, knn_matches, 2);

	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		const cv::DMatch& bestMatch = knn_matches[r][0];
		const cv::DMatch& betterMatch = knn_matches[r][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		//�ų�������Ratio Test�ĵ��ƥ��������ĵ�
		if (distanceRatio > 0.6f )
			continue;
		res->bestMatch.push_back(bestMatch);
	}
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
		match_feature(img0, img1,mr);
		
	}
}
TEST(sfm_mv, read_analyse_images)
{
	//ֻ��3��ͼƬ
	MvSfmContex contex;
	read_analyse_images(&contex,"../media/mv_images/");
	EXPECT_NE(contex.GetImageByIdx(0), nullptr);
	EXPECT_NE(contex.GetImageByIdx(1), nullptr);
	EXPECT_NE(contex.GetImageByIdx(2), nullptr);
	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);

}


//���
//bool sfm_mv()
//{
//	FUN_TIMER;
//	//д�� camera���ڲ����󣬼����Ѿ������˱궨
//	Mat K(Matx33d(
//		2759.48, 0, 1520.69,
//		0, 2764.16, 1006.81,
//		0, 0, 1));
//
//
//	//��ȡһ��Ŀ¼�µ�����image
//	int img_num = read_analyse_images("./mv_imgs/");
//
//
//	//��������ͼƬ������ģ������ģ�����ЩͼƬ��������ƥ��
//	seq_match_feature();
//
//	//���ͷ���Ž����ؽ�
//	init_reconstruct();
//
//	//���μ����µ�ͼƬ��idx+1�� �ѽ���ںϵ��ؽ������
//	//
//	for (int idx = 1; idx < n;++idx)
//	{
//		//���� ��idxͼƬ���Ѿ��õ���3d�� �� ��idx+1 ͼƬ�������㣬��ñ任����R��T
//		calc_trans_rot_pnp();
//
//		//����֮ǰ��õ�R��T������ά�ؽ�
//		reconstruct();
//
//		//�ں� ȥ���ظ�
//		dedup_fusion();
//
//
//	}
//	return true;
//}