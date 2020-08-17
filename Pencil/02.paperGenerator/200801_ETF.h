#pragma once

#pragma warning(disable : 4996)
#define _USE_MATH_DEFINES

#include <opencv2/opencv.hpp>
#include <opencv2\core\core_c.h>
#include <corecrt_math_defines.h>
#include <ppl.h>

using namespace cv;
using namespace std;
using namespace concurrency;

class ETF {

public:
	ETF() {};
	~ETF() {};

	void rotate_field(Mat& flowField, const float degree);
	Mat rotate_field2(const Mat& flowField, const float degree);
	float weight_spatial(const int h, const int w, const int r, const int c, const float radius);	// Eq(2)
	float weight_magnitude(const float gradmg_x, const float gradmg_y, const float n);				// Eq(3)
	float weight_direction(const Vec2f& x, const Vec2f& y); 										// Eq(4)
	float get_phi(const Vec2f& x, const Vec2f& y);													// Eq(5)
	void sobel_x(const Mat& src, Mat& grad_x);
	void sobel_y(const Mat& src, Mat& grad_y);
	void init_ETF(const Mat& src, Mat& flow_field, Mat& grad_mg);
	Mat refine_ETF(const Mat& src, const int ksize, Mat& flow_field, const Mat& grad_mg);
};

