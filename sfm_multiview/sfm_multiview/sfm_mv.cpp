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
	Mat img;
	std::vector<KeyPoint> key_points;
	std::vector<Vec3b> colors;
	Mat descriptors;
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
	image_info* AppendImage(const char* fn)
	{
		image_info* p = new image_info(fn);
		m_images.push_back(p);
		return p;
	}

	image_info* get_image_by_idx(size_t idx)
	{
		if (idx >= m_images.size())
			return 0;

		return m_images.at(idx);
	}
protected:
	vector<image_info*> m_images;
};


void calc_trans_rot_pnp()
{
	//�ҵ� idx��idx+1 ��ƥ��Ľ�����ҵ����¶�Ӧ��ϵ��
	//��idxͼƬ��ƥ��㡢��λ��  ��===�� ��idx+1 ͼƬ�����ص�

	//PnP���任����

	//����ת����ת��Ϊ��ת����
}

//��ȡ����ͼƬ�ļ� ������������
void read_analyse_image(MvSfmContex* contex, const string& fn)
{
	FUN_TIMER;
	image_info& image =*contex->AppendImage(fn.c_str());

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
	for (auto& p : fs::directory_iterator(image_dir))
	{
		auto _path = p.path();
		if (_path.extension() == ".png" ||
			_path.extension() == ".jpg")
		{
			read_analyse_image(contex, _path.string());
		}
	}
}

TEST(sfm_mv, read_analyse_images)
{
	//ֻ��3��ͼƬ
	MvSfmContex contex;
	read_analyse_images(&contex,"../media/mv_images/");
	EXPECT_NE(contex.get_image_by_idx(0), nullptr);
	EXPECT_NE(contex.get_image_by_idx(1), nullptr);
	EXPECT_NE(contex.get_image_by_idx(2), nullptr);
	EXPECT_EQ(contex.get_image_by_idx(3), nullptr);

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
//	//��ȡ����image��������
//	search_feature_point();
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