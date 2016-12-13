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
struct essMatrixRes
{
	int feasible_count;
	Mat essMatrix;
	Mat mask;
};
//每一个match_res对象表示某两张图片特征点匹配的结果
//todo 这个命名不太好 这个对象的职责是保存两张图片的计算结果，包括特征匹配，本质矩阵，相机位置估算
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
	void AddMatchData(const cv::DMatch& m);//todo 把dmatch的概念藏起来
	size_t GetMatchedPointCount() { return bestMatch.size(); }
	const std::vector<cv::Point2f>& GetPosData(int leftright) { return matched_pos[leftright]; }
	const std::vector<cv::Vec3b>& GetColorData(int leftright) { return matched_color[leftright]; }
	int CullByMask(const cv::Mat& mask);
	cv::Mat& GetCameraR() { return m_R; }
	cv::Mat& GetCameraT() { return m_T; }
	essMatrixRes& GetEMatRes() { return m_emr; }
private:
	image_info* img0;
	image_info* img1;
	essMatrixRes m_emr; //保存本质矩阵的计算结果
	cv::Mat m_R,m_T; //根据两张图片推导出来的相机的旋转和位移
	std::vector<cv::Point2f> matched_pos[2];//匹配的点的位置
	std::vector<cv::Vec3b> matched_color[2];//匹配的点的颜色
	
	std::vector<cv::Point2f> culled_pos[2];//匹配的点的位置（mask剔除之后）
	std::vector<cv::Vec3b> culled_color[2];//匹配的点的颜色（mask剔除之后）


	std::vector<cv::DMatch> bestMatch; //for debug
};

//保存在重建过程中的数据
class MvSfmContex
{
public:
	MvSfmContex();
	virtual ~MvSfmContex();

	double GetCameraFocal(); //相机焦距
	cv::Point2d GetCameraPrinP();//获得相机的光心位置
	void BeginAddImage();
	void AddImageFromFile(const char* fn);
	void EndAddImage();
	void ResetContex(size_t imageCount);

	image_info* SetImageData(int idx, const char* fn);

	image_info* GetImageByIdx(size_t idx);

	size_t GetImageCount();

	match_res* GetMatchData(size_t idx0, size_t idx1);
	const cv::Mat& GetCameraK() { return m_camera_K; }
protected:
	void Clean();
	cv::Mat m_camera_K;
	std::vector<image_info> m_images;
	match_res** m_match_res;
	//std::vector<std::vector<match_res>> m_match_res; //二维数组 长宽都等于图片的数量，表示match的匹配关系 其实这里只需要存一个三角形的矩阵即可
};
