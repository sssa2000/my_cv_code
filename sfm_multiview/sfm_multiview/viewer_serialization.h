#pragma once
#include <vector>
#include <opencv/cv.hpp>

void save_structure(const char* file_name, std::vector<cv::Mat>& rotations, std::vector<cv::Mat>& motions, cv::Mat& structure, std::vector<cv::Vec3b>& colors);