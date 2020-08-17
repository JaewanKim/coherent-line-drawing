#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2\core\core_c.h>
#include <corecrt_math_defines.h>
#include <ppl.h>

#include <vector>

using namespace cv;
using namespace std;
using namespace concurrency;

class ImgProcess {

public:
	ImgProcess() {};
	~ImgProcess() {};

	Mat gaussian_blur(const Mat& src, int ksize, float sigma);
	Mat bilateral_filter(const Mat& src, const Mat& colRef_src, int ksize, double dist_sigma, double color_sigma);
	Mat alpha_blending(const Mat& src1, const Mat& src2, int alpha);
};