#pragma once
#include <opencv\highgui.h>
#include <opencv2\features2d\features2d.hpp>
#include <vector>

//����һ��ͼƬ�����ݺ���������Ϣ
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

//���ڼ�¼��1��ͼƬ�е������� �� ��ά�ռ��Ķ�Ӧ��ϵ
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

//�������������е����е��ؽ������ȥ���ظ��ĵ㣬������괦��
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
	std::vector<SImgPointMapping> m_fusion_booking; //��m_images����һ��
	std::vector<cv::Point3f> m_finaly_result; //���յĽ�����ڱ��浽�ļ�������Ⱦ
	std::vector<cv::Vec3b> m_finaly_col_result;
};
//ÿһ��match_res�����ʾĳ����ͼƬ������ƥ��Ľ��
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
	void AddMatchData(const cv::DMatch& m);//todo ��dmatch�ĸ��������
	size_t GetMatchedPointCount() { return bestMatch.size(); }
	const std::vector<cv::DMatch>& GetDMData() { return bestMatch; }
	const std::vector<cv::Point2f>& GetPosData(int leftright) { return matched_pos[leftright]; }
	const std::vector<cv::Vec3b>& GetColorData(int leftright) { return matched_color[leftright]; }
private:
	image_info* img0;
	image_info* img1;
	std::vector<cv::Point2f> matched_pos[2];//ƥ��ĵ��λ��
	std::vector<cv::Vec3b> matched_color[2];//ƥ��ĵ����ɫ
	
	std::vector<cv::DMatch> bestMatch;
};

//���� solvePnPRansac ������
//��ʾ���� ��i��ͼ�����ؽ��õ���ά�㣬�Լ���Щ��ά���ڵ�i+1��ͼ���ж�Ӧ��������
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

//һ��reconRecipe��ʾ��������ͼƬ�ؽ� ��Ҫ�õ����ݣ����Գ�֮Ϊ�䷽
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
	bool m_bCulled; //�Ƿ񾭹���mask���޳�
	std::vector<cv::Point2f> m_passed_pos[2];//ƥ��ĵ��λ�ã�mask�޳�֮��
	std::vector<cv::Vec3b> m_passed_color[2];//ƥ��ĵ����ɫ��mask�޳�֮��
	//cv::Mat m_reconstruct_res;	// �ؽ��Ľ����4��N�еľ���ÿһ�д���ռ��е�һ����
	std::vector<cv::Point3f> m_recons_res;

};
//�������ؽ������е�����
class MvSfmContex
{
public:
	MvSfmContex();
	virtual ~MvSfmContex();
	void InitResult();
	double GetCameraFocal(); //�������
	cv::Point2d GetCameraPrinP();//�������Ĺ���λ��
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
	essMatrixRes m_emr; //���汾�ʾ���ļ�����
	std::vector<image_info> m_images;
	match_res** m_match_res;//��ά���� ��������ͼƬ����������ʾmatch��ƥ���ϵ ��ʵ����ֻ��Ҫ��һ�������εľ��󼴿�
	std::vector<pnpQueryData*> m_pnpQueryDatas;
	std::vector<reconRecipe*> m_Recipes;
	SfmResult m_result;
};
