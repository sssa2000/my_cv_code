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

		//根据图片数量把 全部的image_info 创建出来
		for (size_t i = 0; i < imageCount; ++i)
			m_images.push_back(image_info());

		////根据图片数量把 全部的match_res 创建出来。二维数组 长宽都等于图片的数量	
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
	vector<vector<match_res>> m_match_res; //二维数组 长宽都等于图片的数量，表示match的匹配关系 其实这里只需要存一个三角形的矩阵即可
};


void calc_trans_rot_pnp()
{
	//找到 idx和idx+1 的匹配的结果，找到以下对应关系：
	//第idx图片的匹配点、三位点  《===》 第idx+1 图片的像素点

	//PnP求解变换矩阵

	//将旋转向量转换为旋转矩阵
}

//读取单个图片文件 并分析特征点
void read_analyse_image(size_t idx,MvSfmContex* contex, const string& fn)
{
	FUN_TIMER;
	image_info& image =*contex->SetImageData(idx,fn.c_str());

	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	sift->detectAndCompute(image.img, noArray(), image.key_points, image.descriptors);

	//特征点过少，排除该图像
	if (image.key_points.size() <= 10)
	{
		//todo :error
	}

	//将每个keypoint的颜色保存起来
	for (auto& kp : image.key_points)
	{
		image.colors.push_back(image.img.at<Vec3b>(kp.pt.y, kp.pt.x));
	}
	
}

//遍历文件夹下所有的图片文件 读取 并分析特征点
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
	BFMatcher matcher(NORM_L2); //暴力求解
	std::vector<std::vector<DMatch>> knn_matches;
	//knnMatch K = 2 ，即对每个匹配返回两个最近邻描述符，仅当第一个匹配与第二个匹配之间的距离足够小时，才认为这是一个匹配。
	matcher.knnMatch(img0->descriptors, img1->descriptors, knn_matches, 2);

	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		const cv::DMatch& bestMatch = knn_matches[r][0];
		const cv::DMatch& betterMatch = knn_matches[r][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		//排除不满足Ratio Test的点和匹配距离过大的点
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
	//只有3张图片
	MvSfmContex contex;
	read_analyse_images(&contex,"../media/mv_images/");
	EXPECT_NE(contex.GetImageByIdx(0), nullptr);
	EXPECT_NE(contex.GetImageByIdx(1), nullptr);
	EXPECT_NE(contex.GetImageByIdx(2), nullptr);
	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);

}


//入口
//bool sfm_mv()
//{
//	FUN_TIMER;
//	//写死 camera的内部矩阵，假设已经经过了标定
//	Mat K(Matx33d(
//		2759.48, 0, 1520.69,
//		0, 2764.16, 1006.81,
//		0, 0, 1));
//
//
//	//读取一个目录下的所有image
//	int img_num = read_analyse_images("./mv_imgs/");
//
//
//	//假设这里图片是有序的，连续的，将这些图片进行两两匹配
//	seq_match_feature();
//
//	//针对头两张进行重建
//	init_reconstruct();
//
//	//依次加入新的图片（idx+1） 把结果融合到重建结果中
//	//
//	for (int idx = 1; idx < n;++idx)
//	{
//		//根据 第idx图片的已经得到的3d点 和 第idx+1 图片的特征点，求得变换矩阵R和T
//		calc_trans_rot_pnp();
//
//		//根据之前求得的R，T进行三维重建
//		reconstruct();
//
//		//融合 去掉重复
//		dedup_fusion();
//
//
//	}
//	return true;
//}