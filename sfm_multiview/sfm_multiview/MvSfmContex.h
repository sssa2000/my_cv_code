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
	cv::Mat essMatrix;
	cv::Mat mask;
	cv::Mat cameraR;
	cv::Mat cameraT;

};

//用于记录：1张图片中的特征点 和 三维空间点的对应关系
class SImgPointMapping
{
public:
	SImgPointMapping() {}
	~SImgPointMapping() {}
	void Reset(int feaPoNum);
	void SetIdxMapping(int feaPointIdx, int _3dPosIdx);
	int GetIdxMapping(int feaPointIDx);
private:
	std::vector<int> m_3d_idx_mapping;
};

//管理整个过程中的所有的重建结果。去掉重复的点，齐次坐标处理
class SfmResult
{
public:
	SfmResult() = default;
	~SfmResult() = default;
	void ResetAllFeaPoint(const std::vector<image_info>& allimg);

	int RegResultIdx(int imgIdx, int feaPointIdx, int point3dIdx);
	int Query3dPointIdx(int imgIdx, int feaPointIdx);
	void Add3DPoint(int img0idx, int img1idx, const cv::DMatch& ma, const cv::Point3f& pos,const cv::Vec3b& cols);
	const cv::Point3f& Get3dPoint(int idx);
	const std::vector<cv::Point3f>& GetAllResult() { return m_finaly_result; }
	const std::vector<cv::Vec3b>& GetAllColor() { return m_finaly_col_result; }
private:
	SImgPointMapping& GetFeaPointMap(int imgidx) { return m_fusion_booking.at(imgidx); }
	std::vector<SImgPointMapping> m_fusion_booking; //和m_images数量一样
	std::vector<cv::Point3f> m_finaly_result; //最终的结果用于保存到文件或者渲染
	std::vector<cv::Vec3b> m_finaly_col_result;
};
//每一个match_res对象表示某两张图片特征点匹配的结果
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
	const std::vector<cv::DMatch>& GetDMData() { return bestMatch; }
	const std::vector<cv::Point2f>& GetPosData(int leftright) { return matched_pos[leftright]; }
	const std::vector<cv::Vec3b>& GetColorData(int leftright) { return matched_color[leftright]; }
private:
	image_info* img0;
	image_info* img1;
	std::vector<cv::Point2f> matched_pos[2];//匹配的点的位置
	std::vector<cv::Vec3b> matched_color[2];//匹配的点的颜色
	
	std::vector<cv::DMatch> bestMatch;
};

//用于 solvePnPRansac 的数据
//表示的是 第i幅图像中重建得到三维点，以及这些三维点在第i+1幅图像中对应的特征点
class pnpQueryData
{
public:
	pnpQueryData(int idx0, int idx1) {
		m_idx0 = idx0, m_idx1 = idx1;
	}
	~pnpQueryData(){}
	const std::vector<cv::Point3f>& Get3dPoint() { return m_3d_points; }
	const std::vector<cv::Point2f>& GetFeaPoint(){ return m_fea_points; }
	void AddPoint(const cv::Point3f& _3d, const cv::Point2f& kp);
protected:
	int m_idx0;
	int m_idx1;
	std::vector<cv::Point3f> m_3d_points;
	std::vector<cv::Point2f> m_fea_points;

};

//一个reconRecipe表示的是两张图片重建 需要用的数据，所以称之为配方
class reconRecipe
{
public:
	reconRecipe(int imgidx0, int imgidx1,match_res* mr);
	~reconRecipe() = default;

	int GetImgIdx(int leftright) { return m_image_idx[leftright]; }
	void SetCameraRMatrix(cv::Mat& R);
	void SetCameraRVector(cv::Mat& r);
	void SetCameraT(cv::Mat& T);
	const cv::Mat& GetCameraRMatrix() {return m_R;}
	const cv::Mat& GetCameraT() { return m_T; }

	int SetMask(const cv::Mat& mask);
	const cv::Mat& GetCameraProjMatrix(int leftright);
	void CalcCameraProjMatrix(const cv::Mat& fK);
	const std::vector<cv::Point2f>& GetPosData(int leftright);
	match_res* GetMatchData() { return m_match_data; }
	void AddResconStructMat(const cv::Mat& mat);
	const cv::Point3f& GetReconStructRes(int idx);
protected:
	int m_image_idx[2];
	cv::Mat m_R;
	cv::Mat m_T;
	cv::Mat m_proj[2];
	match_res* m_match_data;
	bool m_bCulled; //是否经过了mask的剔除
	std::vector<cv::Point2f> m_passed_pos[2];//匹配的点的位置（mask剔除之后）
	std::vector<cv::Vec3b> m_passed_color[2];//匹配的点的颜色（mask剔除之后）
	//cv::Mat m_reconstruct_res;	// 重建的结果，4行N列的矩阵，每一列代表空间中的一个点
	std::vector<cv::Point3f> m_recons_res;

};
//保存在重建过程中的数据
class MvSfmContex
{
public:
	MvSfmContex();
	virtual ~MvSfmContex();
	void InitResult();
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
	
	void FusionResult1st();
	void FusionResult(int img0_idx, int img1_idx, reconRecipe* pr);
	void SetEssMat(essMatrixRes& res);
	const essMatrixRes& GetEssMat() { return m_emr; }
	reconRecipe* RequireRecipes(int img0_idx, int img1_idx, match_res* mr);
	pnpQueryData* Query3d2dIntersection(int img0_idx, int img1_idx);
	void saveResToFile();
protected:
	void Clean();
	cv::Mat m_camera_K;
	essMatrixRes m_emr; //保存本质矩阵的计算结果
	std::vector<image_info> m_images;
	match_res** m_match_res;//二维数组 长宽都等于图片的数量，表示match的匹配关系 其实这里只需要存一个三角形的矩阵即可
	std::vector<pnpQueryData*> m_pnpQueryDatas;
	std::vector<reconRecipe*> m_Recipes;
	SfmResult m_result;
};
