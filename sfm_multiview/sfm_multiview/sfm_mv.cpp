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

#include "MvSfmContex.h"

using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;
//记录函数耗时的contex
fun_contex g_fun_contex;

//用于记录函数时间
#define FUN_TIMER fun_timer_obj obj(__FUNCTION__,&g_fun_contex)



//读取单个图片文件 并分析特征点
void analyse_signle_image(MvSfmContex* contex,size_t idx)
{
	FUN_TIMER;
	image_info& image =*contex->GetImageByIdx(idx);

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
		image.colors.push_back(image.img.at<Vec3b>((int)(kp.pt.y), (int)(kp.pt.x)));
	}
	
}

//遍历文件夹下所有的图片文件 读取 并分析特征点
//返回读取了多少张图片
int read_analyse_images(MvSfmContex* contex, const char* image_dir)
{
	FUN_TIMER;
	int idx = 0;
	contex->BeginAddImage();
	for (auto& p : fs::directory_iterator(image_dir))
	{
		auto _path = p.path();
		if (_path.extension() == ".png" ||
			_path.extension() == ".jpg")
		{
			contex->AddImageFromFile(_path.string().c_str());
			analyse_signle_image(contex, idx++);
		}
	}
	contex->EndAddImage();
	return idx;
}
//返回匹配成功的关键点的数量
int match_feature(image_info * img0,image_info* img1, match_res* res)
{
	FUN_TIMER;
	BFMatcher matcher(NORM_L2); //暴力求解
	std::vector<std::vector<DMatch>> knn_matches;
	//knnMatch K = 2 ，即对每个匹配返回两个最近邻描述符，仅当第一个匹配与第二个匹配之间的距离足够小时，才认为这是一个匹配。
	matcher.knnMatch(img0->descriptors, img1->descriptors, knn_matches, 2);
	int succNum = 0;
	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		const cv::DMatch& bestMatch = knn_matches[r][0];
		const cv::DMatch& betterMatch = knn_matches[r][1];
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		//排除不满足Ratio Test的点和匹配距离过大的点
		if (distanceRatio > 0.6f )
			continue;
		res->AddMatchData(bestMatch);
		++succNum;
	}
	return succNum;
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
		int sucNum=match_feature(img0, img1,mr);
		
	}
}
//TEST(sfm_mv, read_analyse_images)
//{
//	//只有3张图片
//	MvSfmContex contex;
//	read_analyse_images(&contex,"../media/mv_images/1");
//	EXPECT_NE(contex.GetImageByIdx(0), nullptr);
//	EXPECT_NE(contex.GetImageByIdx(1), nullptr);
//	EXPECT_NE(contex.GetImageByIdx(2), nullptr);
//	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);
//
//}



bool sloveE(MvSfmContex* contex,int image_idx0,int image_idx1)
{
	FUN_TIMER;
	//根据内参矩阵获取相机的焦距和光心坐标（主点坐标）
	double f=contex->GetCameraFocal();
	Point2d pp=contex->GetCameraPrinP();
	
	//根据匹配点求取本征矩阵，使用RANSAC，进一步排除失配点
	match_res* mr = contex->GetMatchData(image_idx0, image_idx1);
	essMatrixRes out;
	out.essMatrix = findEssentialMat(
		mr->GetPosData(0), mr->GetPosData(1), 
		f, pp, RANSAC, 0.999, 1.0, 
		out.mask);

	//mask中 0表示异常的点 1表示正常的点
	out.feasible_count = countNonZero(out.mask);
	contex->SetEssMat(out);

	//对于RANSAC而言，outlier数量大于50%时，结果是不可靠的
	if (out.feasible_count <= 15 ||
		((float)out.feasible_count / mr->GetPosData(0).size()) < 0.6f)
		return (false);
	return true;
}

void tri_reconstruct(MvSfmContex* contex, reconRecipe* pr)
{
	FUN_TIMER;

	//根据R和T 建立投影矩阵
	pr->CalcCameraProjMatrix(contex->GetCameraK());

	//三角重建
	auto md = contex->GetMatchData(pr->GetImgIdx(0), pr->GetImgIdx(1));
	cv::Mat res;
	cv::triangulatePoints(
		pr->GetCameraProjMatrix(0), 
		pr->GetCameraProjMatrix(1),
		pr->GetPosData(0),
		pr->GetPosData(1),
		res);

	pr->AddResconStructMat(res);

}

reconRecipe* reconstruct_other(int idx0, int idx1, MvSfmContex* contex)
{
	auto md = contex->GetMatchData(idx0, idx1);
	reconRecipe* recipe = contex->RequireRecipes(idx0, idx1, md);
	pnpQueryData* pnp = contex->Query3d2dIntersection(idx0, idx1);
	//根据 第idx图片的已经得到的3d点 和 第idx+1 图片的特征点，求得变换矩阵R和T

	//求解变换矩阵
	Mat r, T;
	cv::solvePnPRansac(pnp->Get3dPoint(),
		pnp->GetFeaPoint(),
		contex->GetCameraK(),
		noArray(),
		r, T);
	recipe->SetCameraT(T);
	recipe->SetCameraRVector(r);

	tri_reconstruct(contex, recipe);

	return recipe;
}
//init reconstruct use first and second image
reconRecipe* reconstruct_fs(MvSfmContex* contex)
{
	
	int idx0 = 0;
	int idx1 = 1;
	match_res* mr = contex->GetMatchData(idx0, idx1);
	reconRecipe* recipe = contex->RequireRecipes(idx0, idx1, mr);

	//根据头两张照片求解E
	bool b=sloveE(contex,0,1);
	const essMatrixRes& essMat = contex->GetEssMat();

	//分解本征矩阵，获取相对变换
	Mat R,T,mask;
	int pass_count = cv::recoverPose(essMat.essMatrix,
		mr->GetPosData(0), 
		mr->GetPosData(1), 
		R,
		T,
		contex->GetCameraFocal(), 
		contex->GetCameraPrinP(), 
		mask);
	//位于两个相机前方的点的数量要足够大
	if (((float)pass_count) / essMat.feasible_count < 0.7f)
		return false;

	recipe->SetCameraT(T);
	recipe->SetCameraRMatrix(R);
	int cull_count = recipe->SetMask(essMat.mask);


	//重建三维空间的位置 使用三角重建法
	tri_reconstruct(contex,recipe);


	return recipe;
}
//入口
#ifndef LAUNCH_GTEST
bool sfm_mv()
#else
TEST(sfm_mv, all_exec)
#endif
{
	FUN_TIMER;
	MvSfmContex contex;

	//读取一个目录下的所有image
	int img_num = read_analyse_images(&contex,"../media/mv_images/");
	contex.InitResult();

	EXPECT_EQ(img_num, 3);
	EXPECT_EQ(contex.GetImageByIdx(3), nullptr);

	//假设这里图片是有序的，连续的，将这些图片进行两两匹配
	seq_match_feature(&contex);
	EXPECT_GT(contex.GetMatchData(0, 1)->GetMatchedPointCount(), (size_t)0);
	EXPECT_GT(contex.GetMatchData(1, 2)->GetMatchedPointCount(), (size_t)0);
	EXPECT_EQ(contex.GetMatchData(2, 3), nullptr);
	EXPECT_EQ(contex.GetMatchData(0, 0), nullptr);
	EXPECT_EQ(contex.GetMatchData(2, 0), nullptr);


	//针对头两张进行重建
	reconRecipe* rp0=reconstruct_fs(&contex);
	contex.FusionResult1st();

	//依次加入新的图片（idx+1） 把结果融合到重建结果中
	//
	for (int idx = 1; idx < img_num - 1;++idx)
	{
		//根据之前求得的R，T进行三维重建
		int nextIdx = idx + 1;
		reconRecipe* pother=reconstruct_other(idx, nextIdx,&contex);
		contex.FusionResult(idx, nextIdx, pother);

	}

	//save to file

	contex.saveResToFile();

	

	//return true;
}