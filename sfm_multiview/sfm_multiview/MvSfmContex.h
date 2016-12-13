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
	Mat essMatrix;
	Mat mask;
};
//ÿһ��match_res�����ʾĳ����ͼƬ������ƥ��Ľ��
//todo ���������̫�� ��������ְ���Ǳ�������ͼƬ�ļ���������������ƥ�䣬���ʾ������λ�ù���
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
	const std::vector<cv::Point2f>& GetPosData(int leftright) { return matched_pos[leftright]; }
	const std::vector<cv::Vec3b>& GetColorData(int leftright) { return matched_color[leftright]; }
	int CullByMask(const cv::Mat& mask);
	cv::Mat& GetCameraR() { return m_R; }
	cv::Mat& GetCameraT() { return m_T; }
	essMatrixRes& GetEMatRes() { return m_emr; }
private:
	image_info* img0;
	image_info* img1;
	essMatrixRes m_emr; //���汾�ʾ���ļ�����
	cv::Mat m_R,m_T; //��������ͼƬ�Ƶ��������������ת��λ��
	std::vector<cv::Point2f> matched_pos[2];//ƥ��ĵ��λ��
	std::vector<cv::Vec3b> matched_color[2];//ƥ��ĵ����ɫ
	
	std::vector<cv::Point2f> culled_pos[2];//ƥ��ĵ��λ�ã�mask�޳�֮��
	std::vector<cv::Vec3b> culled_color[2];//ƥ��ĵ����ɫ��mask�޳�֮��


	std::vector<cv::DMatch> bestMatch; //for debug
};

//�������ؽ������е�����
class MvSfmContex
{
public:
	MvSfmContex();
	virtual ~MvSfmContex();

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
protected:
	void Clean();
	cv::Mat m_camera_K;
	std::vector<image_info> m_images;
	match_res** m_match_res;
	//std::vector<std::vector<match_res>> m_match_res; //��ά���� ��������ͼƬ����������ʾmatch��ƥ���ϵ ��ʵ����ֻ��Ҫ��һ�������εľ��󼴿�
};
