#pragma once

#include "200801_ETF.h"

void ETF::rotate_field(Mat& flowField, const float degree) {
	const float theta = degree / 180.0 * M_PI;

	for (int i = 0; i < flowField.rows; i++) {
		for (int j = 0; j < flowField.cols; j++) {
			Vec2f v = flowField.at<Vec2f>(i, j);
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);

			flowField.at<Vec2f>(i, j) = Vec2f(rx, ry);
		}
	}
}

Mat ETF::rotate_field2(const Mat& flowField, const float degree) {
	const float theta = degree / 180.0 * M_PI;
	Mat rotatedField = flowField.clone();

	for (int i = 0; i < flowField.rows; i++) {
		for (int j = 0; j < flowField.cols; j++) {
			Vec2f v = flowField.at<Vec2f>(i, j);
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);

			rotatedField.at<Vec2f>(i, j) = Vec2f(rx, ry);
		}
	}
	return rotatedField;
}

float ETF::weight_spatial(const int h, const int w, const int r, const int c, const float radius) { 	// Eq(2)
	if (sqrt(pow(h - c, 2) + pow(w - r, 2)) < radius)
		return 1;
	else
		return 0;
}

float ETF::weight_magnitude(const float gradmg_x, const float gradmg_y, const float n) {	// Eq(3)
	return 0.5 * (1 + tanh(n * (gradmg_y - gradmg_x)));
}

float ETF::weight_direction(const Vec2f& x, const Vec2f& y) { 				// Eq(4)
	return abs(x.dot(y));
}

float ETF::get_phi(const Vec2f& x, const Vec2f& y) {							// Eq(5)
	if (x.dot(y) > 0) return 1;
	else return -1;
}

void ETF::sobel_x(const Mat& src, Mat& grad_x) {
	int height = src.rows;
	int width = src.cols;

	for (int r = 0; r < height; r++) {
		for (int c = 1; c < width - 1; c++)
			grad_x.at<float>(r, c) = src.at<unsigned char>(r, c + 1) - src.at<unsigned char>(r, c - 1);
		grad_x.at<float>(r, 0) = src.at<unsigned char>(r, 1) - src.at<unsigned char>(r, 0);
		grad_x.at<float>(r, width - 1) = src.at<unsigned char>(r, width - 1) - src.at<unsigned char>(r, width - 2);
	}
}

void ETF::sobel_y(const Mat& src, Mat& grad_y) {
	int height = src.rows;
	int width = src.cols;

	for (int c = 0; c < width; c++) {
		for (int r = 1; r < height - 1; r++)
			grad_y.at<float>(r, c) = src.at<unsigned char>(r + 1, c) - src.at<unsigned char>(r - 1, c);
		grad_y.at<float>(0, c) = src.at<unsigned char>(1, c) - src.at<unsigned char>(0, c);
		grad_y.at<float>(height - 1, c) = src.at<unsigned char>(height - 1, c) - src.at<unsigned char>(height - 2, c);
	}
}

void ETF::init_ETF(const Mat& src, Mat& flow_field, Mat& grad_mg) {
	int height = src.rows;
	int width = src.cols;
	Mat grad_x = Mat::zeros(Size(width, height), CV_32FC1);
	Mat grad_y = Mat::zeros(Size(width, height), CV_32FC1);

	//Sobel(src, grad_x, CV_32FC1, 1, 0, 5);
	sobel_x(src, grad_x);
	//Sobel(src, grad_y, CV_32FC1, 0, 1, 5);
	sobel_y(src, grad_y);
	magnitude(grad_x, grad_y, grad_mg);

	for (int r = 0; r < src.rows; r++) {
		for (int c = 0; c < src.cols; c++) {
			float u = grad_x.at<float>(r, c);
			float v = grad_y.at<float>(r, c);

			if ((u == 0.f) && (v == 0.f)) {
				grad_mg.at<float>(r, c) = 0.00001f;
				flow_field.at<Vec2f>(r, c) = Vec2f(0.f, 1.f);
				//flow_field.at<Vec2f>(r, c) = Vec2f(1.f, 0.f);
			}
			else
				flow_field.at<Vec2f>(r, c) = normalize(Vec2f(u, v));
		}
	}
	rotate_field(flow_field, 90.f);
}

Mat ETF::refine_ETF(const Mat& src, const int ksize, Mat& flow_field, const Mat& grad_mg) {
	Mat refined_field = Mat::zeros(src.size(), CV_32FC2);
	int width = src.cols;
	int height = src.rows;

	parallel_for(int(0), height, [&](int h) {
		//for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			Vec2f t_cur_x = flow_field.at<Vec2f>(h, w);
			Vec2f t_new = Vec2f(0.f, 0.f);
			float k = 0.0;

			for (int r = h - ksize; r < h + ksize; r++) {
				for (int c = w - ksize; c < w + ksize; c++) {
					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					Vec2f t_cur_y = flow_field.at<Vec2f>(r, c);

					float phi = get_phi(t_cur_x, t_cur_y);
					if (phi == 0.f)
						continue;
					float w_s = weight_spatial(h, w, c, r, ksize);
					if (w_s == 0.f)
						continue;
					float w_m = weight_magnitude(grad_mg.at<float>(h, w), grad_mg.at<float>(r, c), 1.f);
					if (w_m == 0.f)
						continue;
					float w_d = weight_direction(t_cur_x, t_cur_y);
					if (w_d == 0.f)
						continue;

					t_new += (phi * t_cur_y * w_s * w_m * w_d);
					k += (phi * w_s * w_m * w_d);
				}
			}

			if (k != 0.f)
				t_new /= k;
			refined_field.at<Vec2f>(h, w) = normalize(t_new);
		}

		});

	return refined_field;
}