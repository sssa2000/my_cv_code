#include "MvSfmContex.h"
#include <opencv2\imgcodecs.hpp>
#include <opencv2\calib3d\calib3d.hpp>
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
//初始化所有的feature point和3d点的对应关系
void SfmResult::ResetAllFeaPoint(const std::vector<image_info>& allimg)
{
	m_fusion_booking.swap(std::vector<SImgPointMapping>(allimg.size()));
	int i = 0;
	for (auto& fm : m_fusion_booking)
	{
		fm.Reset(allimg.at(i++).key_points.size());
	}
	m_finaly_result.clear();

}
int SfmResult::RegResultIdx(int imgIdx, int feaPointIdx, int point3dIdx)
{
	m_fusion_booking.at(imgIdx).SetIdxMapping(feaPointIdx, point3dIdx);
	return 1;
}
int SfmResult::Query3dPointIdx(int imgIdx, int feaPointIdx)
{
	return m_fusion_booking.at(imgIdx).GetIdxMapping(feaPointIdx);
}

void SfmResult::Add3DPoint(int img0idx,int img1idx, const cv::DMatch& ma,const cv::Vec4f& pos)
{
	m_finaly_result.push_back(cv::Point3f(
		pos[0],	pos[1],	pos[2]));
	size_t lastidx = m_finaly_result.size()-1;
	m_fusion_booking.at(img0idx).SetIdxMapping(ma.queryIdx, lastidx);
	m_fusion_booking.at(img1idx).SetIdxMapping(ma.trainIdx, lastidx);

}

const cv::Point3d& SfmResult::Get3dPoint(int idx)
{
	return m_finaly_result.at(idx);
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

void MvSfmContex::InitResult()
{
	m_result.ResetAllFeaPoint(m_images);
}



//這裡的特殊之處在 只有第一次重建具有mask
void MvSfmContex::FusionResult1st()
{
	auto& cvmatches = GetMatchData(0, 1)->GetDMData();
	for (size_t i = 0,idx=0; i < cvmatches.size();++i)
	{
		if (m_emr.mask.at<uchar>(i) == 0)
			continue;
		m_result.RegResultIdx(0, cvmatches[i].queryIdx, idx);
		m_result.RegResultIdx(1, cvmatches[i].trainIdx, idx);
		++idx;
	}

}

//把本次重建的结果融合到m_finaly_result中去
void MvSfmContex::FusionResult(int img0_idx, int img1_idx, reconRecipe* pri)
{
	auto& cvmd=pri->GetMatchData()->GetDMData();
	cv::Mat res3d=pri->GetReconStructRes();
	//SImgPointMapping& img1_mapping = m_result.GetFeaPointMap(img1_idx);
	int i = 0;
	for (auto& m:cvmd)
	{
		//在img0中查询
		int residx= m_result.Query3dPointIdx(img0_idx, m.queryIdx);
		if (residx != -1)
			m_result.RegResultIdx(img1_idx, i, residx); //如果这个点已经存在了，把idx复制过来
		else
		{
			m_result.Add3DPoint(img0_idx, img1_idx,m,res3d.at<Vec4f>(i));


		}
		++i;
	}
}
void MvSfmContex::SetEssMat(essMatrixRes & res)
{
	m_emr = res;
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

	for (auto r : m_Recipes)
	{
		delete r;
	}
	m_Recipes.clear();

	for (auto r : m_pnpQueryDatas)
	{
		delete r;
	}
	m_pnpQueryDatas.clear();
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

	//init m_Recipes
	m_Recipes.swap(vector<reconRecipe*>(imageCount-1)); 
	std::fill(m_Recipes.begin(), m_Recipes.end(), nullptr);

	//init m_pnpQueryDatas
	m_pnpQueryDatas.swap(vector<pnpQueryData*>(imageCount-1));
	for (size_t i = 0; i < imageCount-1;++i)
	{
		m_pnpQueryDatas[i] = new pnpQueryData(i, i + 1);
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

reconRecipe* MvSfmContex::RequireRecipes(int img0_idx, int img1_idx, match_res* mr)
{
	//这里假设img0_idx和img1_idx一定是连续 并且img0_idx<img1_idx
	assert(img1_idx - img0_idx == 1);
	assert(img0_idx > 0);

	auto& r = m_Recipes.at(img0_idx-1);
	assert(r == 0);

	r = new reconRecipe(img0_idx, img1_idx, mr);
	return r;
}

//查询img1_idx的特征点中 有哪些是img0_idx中已经重建完毕的3d的点
pnpQueryData* MvSfmContex::Query3d2dIntersection(int img0_idx, int img1_idx)
{
	pnpQueryData* pnp = m_pnpQueryDatas.at(img0_idx - 1);
	auto& img1Kp=GetImageByIdx(img1_idx)->key_points;
	auto& cvmatches = GetMatchData(0, 1)->GetDMData();
	for (size_t i = 0, idx = 0; i < cvmatches.size(); ++i)
	{
		const DMatch& m = cvmatches.at(i);
		int _3didx=m_result.Query3dPointIdx(img0_idx, m.queryIdx);
		if(_3didx<0)
			continue;
		pnp->AddPoint(m_result.Get3dPoint(_3didx), img1Kp[m.trainIdx].pt);
	}
	return pnp;
}
void SImgPointMapping::Reset(int feaPoNum)
{
	m_3d_idx_mapping.swap(std::vector<int>(feaPoNum,-1));
}

void SImgPointMapping::SetIdxMapping(int feaPointIdx, int _3dPosIdx)
{
	m_3d_idx_mapping.at(feaPointIdx) = _3dPosIdx;
}
int SImgPointMapping::GetIdxMapping(int feaPointIDx)
{
	return m_3d_idx_mapping.at(feaPointIDx);
}

void reconRecipe::SetCameraRMatrix(cv::Mat& R)
{
	m_R = R;
}
void reconRecipe::SetCameraRVector(cv::Mat & r)
{
	cv::Rodrigues(r, m_R);//将旋转向量转换为旋转矩阵
}
void reconRecipe::SetCameraT(cv::Mat& T)
{
	m_T = T;
}
int reconRecipe::SetMask(const cv::Mat & mask)
{
	int count = mask.rows;
	m_passed_pos[0].clear();
	m_passed_pos[1].clear();
	m_passed_color[0].clear();
	m_passed_color[1].clear();

	auto& leftpos=m_match_data->GetPosData(0);
	auto& rightpos= m_match_data->GetPosData(1);
	auto& leftcol = m_match_data->GetColorData(0);
	auto& rightcol = m_match_data->GetColorData(1);
	for (int i = 0; i < mask.rows; ++i)
	{
		if (mask.at<uchar>(i) > 0)
		{
			count -= 1;			
			m_passed_pos[0].push_back(leftpos.at(i));
			m_passed_pos[1].push_back(rightpos.at(i));
			m_passed_color[0].push_back(leftcol.at(i));
			m_passed_color[1].push_back(rightcol.at(i));
		}
	}
	m_bCulled = count > 0;
	return count;
}
void reconRecipe::CalcCameraProjMatrix(const Mat& fK)
{
	//这个函数的前置条件是 R和T都已经就位

	//两个相机的投影矩阵[R T]，triangulatePoints只支持float型
	m_proj[0] = Mat(3, 4, CV_32FC1); //CV_32FC1 表示 矩阵的每个元素是float32，且这个float表示一个通道
	m_proj[0](Range(0, 3), Range(0, 3)) = Mat::eye(3, 3, CV_32FC1);
	m_proj[0].col(3) = Mat::zeros(3, 1, CV_32FC1);

	m_proj[1] = Mat(3, 4, CV_32FC1);
	m_R.convertTo(m_proj[1](Range(0, 3), Range(0, 3)), CV_32FC1);
	m_T.convertTo(m_proj[1].col(3), CV_32FC1);

	m_proj[0] = fK*m_proj[0];
	m_proj[1] = fK*m_proj[1];
}
const cv::Mat & reconRecipe::GetCameraProjMatrix(int leftright)
{

	return m_proj[leftright];
}

reconRecipe::reconRecipe(int imgidx0, int imgidx1, match_res* mr)
{
	m_image_idx[0] = imgidx0;
	m_image_idx[1] = imgidx1;
	m_match_data = mr;
}

const std::vector<cv::Point2f>& reconRecipe::GetPosData(int leftright)
{
	if (m_bCulled)
		return m_passed_pos[leftright];
	return m_match_data->GetPosData(leftright);
}


void pnpQueryData::AddPoint(const cv::Point3f& _3d, const cv::Point2f& kp)
{
	m_3d_points.push_back(_3d);
	m_fea_points.push_back(kp);
}