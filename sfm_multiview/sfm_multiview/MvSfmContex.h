#pragma once
#include <opencv\highgui.h>
#include <opencv2\features2d\features2d.hpp>
#include <vector>

//保存一张图片的数据和特征点信息
struct image_info
{
	image_info(const char* image_fn);
	image_info() {}
	~image_info() {}
	
	void reload_data(const char* image_fn);

	cv::Mat img;
	std::vector<cv::KeyPoint> key_points;
	std::vector<cv::Vec3b> colors;
	cv::Mat descriptors;
};

//保存两张图片特征点匹配的结果
class match_res
{
public:
	match_res() {}
	match_res(image_info* _img0, image_info* _img1)
	{
		img0 = _img0;
		img1 = _img1;
	}
	void SetImg(image_info* _img0, image_info* _img1)
	{
		img0 = _img0;
		img1 = _img1;
	}
	void AddMatchData(const cv::DMatch& m) { bestMatch.push_back(m); } //todo 把dmatch的概念藏起来
	size_t GetMatchedPointCount() { return bestMatch.size(); }

private:
	image_info* img0;
	image_info* img1;
	std::vector<cv::DMatch> bestMatch; //for debug
};

//保存在重建过程中的数据
class MvSfmContex
{
public:
	MvSfmContex();
	virtual ~MvSfmContex();

	void BeginAddImage();
	void AddImageFromFile(const char* fn);
	void EndAddImage();
	void ResetContex(size_t imageCount);

	image_info* SetImageData(int idx, const char* fn);

	image_info* GetImageByIdx(size_t idx);

	size_t GetImageCount();

	match_res* GetMatchData(size_t idx0, size_t idx1);
protected:
	void Clean();
	std::vector<image_info> m_images;
	match_res** m_match_res;
	//std::vector<std::vector<match_res>> m_match_res; //二维数组 长宽都等于图片的数量，表示match的匹配关系 其实这里只需要存一个三角形的矩阵即可
};
