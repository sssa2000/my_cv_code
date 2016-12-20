#pragma once
#include <vector>
#include <opencv/cv.hpp>

void save_structure(const char* file_name, 
	const std::vector<cv::Mat>& rotations, 
	const std::vector<cv::Mat>& motions,
	const std::vector<cv::Point3f>& structure,
	const std::vector<cv::Vec3b>& colors);