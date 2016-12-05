#include "viewer_serialization.h"
using namespace cv;
using namespace std;
void save_structure(const char* file_name, vector<Mat>& rotations, vector<Mat>& motions, Mat& structure, vector<Vec3b>& colors)
{
	
	int n = (int)rotations.size();

	FileStorage fs(file_name, FileStorage::WRITE);
	fs << "Camera Count" << n;
	fs << "Point Count" << structure.cols;

	fs << "Rotations" << "[";
	for (int i = 0; i < n; ++i)
	{
		fs << rotations[i];
	}
	fs << "]";

	fs << "Motions" << "[";
	for (int i = 0; i < n; ++i)
	{
		fs << motions[i];
	}
	fs << "]";

	fs << "Points" << "[";
	for (int i = 0; i < structure.cols; ++i)
	{
		Mat_<float> c = structure.col(i);
		c /= c(3);	//Æë´Î×ø±êdiv
		fs << Point3f(c(0), c(1), c(2));
	}
	fs << "]";

	fs << "Colors" << "[";
	for (size_t i = 0; i < colors.size(); ++i)
	{
		fs << colors[i];
	}
	fs << "]";

	fs.release();
}
