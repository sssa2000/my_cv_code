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
//记录函数耗时的contex
fun_contex g_fun_contex;

//用于记录函数时间
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


void calc_trans_rot_pnp()
{
	//找到 idx和idx+1 的匹配的结果，找到以下对应关系：
	//第idx图片的匹配点、三位点  《===》 第idx+1 图片的像素点

	//PnP求解变换矩阵

	//将旋转向量转换为旋转矩阵
}

//遍历文件夹下所有的图片文件 读取 并分析特征点
void read_analyse_images(const char* image_dir)
{
	for (auto& p : fs::directory_iterator(image_dir))
	{
		if (p.path().extension()==".png")
		{

		}
	}
}
//入口
bool sfm_mv()
{
	FUN_TIMER;
	//写死 camera的内部矩阵，假设已经经过了标定
	Mat K(Matx33d(
		2759.48, 0, 1520.69,
		0, 2764.16, 1006.81,
		0, 0, 1));


	//读取一个目录下的所有image
	int img_num = read_analyse_images("./mv_imgs/");

	//提取所有image的特征点
	search_feature_point();

	//假设这里图片是有序的，连续的，将这些图片进行两两匹配
	seq_match_feature();

	//针对头两张进行重建
	init_reconstruct();

	//依次加入新的图片（idx+1） 把结果融合到重建结果中
	//
	for (int idx = 1; idx < n;++idx)
	{
		//根据 第idx图片的已经得到的3d点 和 第idx+1 图片的特征点，求得变换矩阵R和T
		calc_trans_rot_pnp();

		//根据之前求得的R，T进行三维重建
		reconstruct();

		//融合 去掉重复
		dedup_fusion();


	}
	return true;
}