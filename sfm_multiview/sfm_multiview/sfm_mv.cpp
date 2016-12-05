#include "sfm_mv.h"


#include <opencv\highgui.h>
#include <opencv2\xfeatures2d\nonfree.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\calib3d\calib3d.hpp>
#include <opencv2\imgcodecs.hpp>
#include <iostream>
#include <vector>
#include <filesystem>

#include "util.h"
#include "viewer_serialization.h"
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


void calc_trans_rot_pnp()
{
	//�ҵ� idx��idx+1 ��ƥ��Ľ�����ҵ����¶�Ӧ��ϵ��
	//��idxͼƬ��ƥ��㡢��λ��  ��===�� ��idx+1 ͼƬ�����ص�

	//PnP���任����

	//����ת����ת��Ϊ��ת����
}

//�����ļ��������е�ͼƬ�ļ� ��ȡ ������������
void read_analyse_images(const char* image_dir)
{
	for (auto& p : fs::directory_iterator(image_dir))
	{
		if (p.path().extension()==".png")
		{

		}
	}
}
//���
bool sfm_mv()
{
	FUN_TIMER;
	//д�� camera���ڲ����󣬼����Ѿ������˱궨
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));


	//��ȡһ��Ŀ¼�µ�����image
	int img_num = read_analyse_images("./mv_imgs/");

	//��ȡ����image��������
	search_feature_point();

	//��������ͼƬ������ģ������ģ�����ЩͼƬ��������ƥ��
	seq_match_feature();

	//���ͷ���Ž����ؽ�
	init_reconstruct();

	//���μ����µ�ͼƬ��idx+1�� �ѽ���ںϵ��ؽ������
	//
	for (int idx = 1; idx < n;++idx)
	{
		//���� ��idxͼƬ���Ѿ��õ���3d�� �� ��idx+1 ͼƬ�������㣬��ñ任����R��T
		calc_trans_rot_pnp();

		//����֮ǰ��õ�R��T������ά�ؽ�
		reconstruct();

		//�ں� ȥ���ظ�
		dedup_fusion();


	}
	return true;
}