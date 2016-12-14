#include "MvSfmContex.h"
#include <opencv2\imgcodecs.hpp>
using namespace cv;
using namespace std;

image_info::image_info(const char* image_fn)
{
	img = imread(image_fn);
}

void image_info::reload_data(const char* image_fn)
{
	img = imread(image_fn);
	key_points.clear();
	descriptors.release();
	colors.clear();

}
void match_res::AddMatchData(const cv::DMatch& m) 
{ 
	bestMatch.push_back(m); 
	matched_pos[0].push_back(img0->key_points[m.queryIdx].pt);
	matched_pos[1].push_back(img0->key_points[m.trainIdx].pt);

	matched_color[0].push_back(img0->colors[m.queryIdx]);
	matched_color[1].push_back(img0->colors[m.trainIdx]);

}
//返回剔除掉了多少个点
int match_res::CullByMask(const cv::Mat& mask)
{
	int count = mask.rows;
	culled_pos[0].clear();
	culled_pos[1].clear();
	culled_color[0].clear();
	culled_color[1].clear();


	for (int i = 0; i <mask.rows; ++i)
	{
		if (mask.at<uchar>(i) > 0)
		{
			count -= 1;
			culled_pos[0].push_back(matched_pos[0].at(i));
			culled_pos[1].push_back(matched_pos[1].at(i));
			culled_color[0].push_back(matched_color[0].at(i));
			culled_color[1].push_back(matched_color[1].at(i));
		}
	}

	return count;
}
double MvSfmContex::GetCameraFocal()
{
	double focal_length = (double)0.5*(m_camera_K.at<float>(0) + m_camera_K.at<float>(4));
	return focal_length;
}
Point2d MvSfmContex::GetCameraPrinP()
{
	Point2d principle_point((double)m_camera_K.at<float>(2), (double)m_camera_K.at<float>(5));
	return principle_point;
}

MvSfmContex::MvSfmContex()
{
	m_match_res = 0;
	//写死 camera的内部矩阵，假设已经经过了标定
	m_camera_K=Mat(Matx33f(
		2759.48f, 0, 1520.69f,
		0, 2764.16f, 1006.81f,
		0, 0, 1));
}

MvSfmContex::~MvSfmContex()
{
	Clean();
}

//初始化所有的feature point和3d点的对应关系
void MvSfmContex::ResetAllFeaPoint()
{
	m_fusion_booking.swap(std::vector<FeaPointMapping>(m_images.size()));
	int i = 0;
	for (auto& fm:m_fusion_booking)
	{
		fm.Reset(m_images[i++].key_points.size());
	}

}
void MvSfmContex::FusionResult1st(int img0_idx, int img1_idx)
{
	match_res* mr=GetMatchData(img0_idx, img1_idx);
	int idx = 0;
	auto& cvmatches = mr->GetDMData();
	auto& mask = mr->GetEMatRes().mask;
	for (size_t i = 0; i < cvmatches.size();++i)
	{
		if (mask.at<uchar>(i) == 0)
			continue;

		m_fusion_booking[img0_idx].SetIdxMapping(cvmatches[i].queryIdx,idx);
		m_fusion_booking[img1_idx].SetIdxMapping(cvmatches[i].trainIdx,idx);
		++idx;
	}

}
void MvSfmContex::Clean()
{
	//clean
	size_t n = GetImageCount();
	for (size_t i = 0; i < n; ++i)
	{
		match_res* row = *(m_match_res+i);
		delete[] row;
	}
	delete[] m_match_res;
	m_images.clear();
}
void MvSfmContex::BeginAddImage()
{
	Clean();

}

void MvSfmContex::AddImageFromFile(const char* fn)
{
	m_images.push_back(image_info(fn));
}

void MvSfmContex::EndAddImage()
{
	size_t imageCount = GetImageCount();
	m_match_res = new match_res*[imageCount];
	for (size_t col = 0; col < imageCount; ++col)
	{
		match_res* p = new match_res[imageCount];
		m_match_res[col] = p;
	}
	//根据图片数量把 全部的match_res 创建出来。二维数组 长宽都等于图片的数量	
	for (size_t row = 0; row < imageCount; ++row)
	{
		image_info* img0 = GetImageByIdx(row);
		
		for (size_t col = 0; col < imageCount; ++col)
		{
			image_info* img1 = GetImageByIdx(col);			
			m_match_res[row][col].SetImg(img0, img1);
			
		}
	}

}
void MvSfmContex::ResetContex(size_t imageCount)
{
	BeginAddImage();

	//根据图片数量把 全部的image_info 创建出来
	for (size_t i = 0; i < imageCount; ++i)
		m_images.push_back(image_info());

	EndAddImage();
}

image_info* MvSfmContex::SetImageData(int idx, const char* fn)
{
	image_info* p = GetImageByIdx(idx);
	if (p)
		p->reload_data(fn);
	return p;
}

image_info* MvSfmContex::GetImageByIdx(size_t idx)
{
	if (idx >= m_images.size())
		return 0;

	return &m_images.at(idx);
}

size_t MvSfmContex::GetImageCount()
{ 
	return m_images.size(); 
}

match_res* MvSfmContex::GetMatchData(size_t idx0, size_t idx1)
{
	if (idx0 >= m_images.size() || idx1 >= m_images.size())
		return 0;
	return &(m_match_res[idx0][idx1]);
}

void FeaPointMapping::Reset(int feaPoNum)
{
	m_3d_idx_mapping.swap(std::vector<int>(feaPoNum,-1));
}

void FeaPointMapping::SetIdxMapping(int feaPointIdx, int _3dPosIdx)
{
	m_3d_idx_mapping.at(feaPointIdx) = _3dPosIdx;
}
