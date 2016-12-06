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



MvSfmContex::MvSfmContex()
{
	m_match_res = 0;
}

MvSfmContex::~MvSfmContex()
{
	Clean();
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