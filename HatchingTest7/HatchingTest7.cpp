// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#pragma warning(disable : 4996)
#define USES_CONVERSION
#define _USE_MATH_DEFINES

#include "CLD.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC2.h"
#include <ppl.h>

using namespace cv;
using namespace std;
using namespace concurrency;

void rotate_field(Mat& flowField, const float degree) {
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

Mat rotate_field2(const Mat& flowField, const float degree) {
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

float weight_spatial(const int h, const int w, const int r, const int c, const float radius) { 	// Eq(2)
	if (sqrt(pow(h - c, 2) + pow(w - r, 2)) < radius)
		return 1;
	else
		return 0;
}

float weight_magnitude(const float gradmg_x, const float gradmg_y, const float n) {	// Eq(3)
	return 0.5 * (1 + tanh(n * (gradmg_y - gradmg_x)));
}

float weight_direction(const Vec2f& x, const Vec2f& y) { 				// Eq(4)
	return abs(x.dot(y));
}

float get_phi(const Vec2f& x, const Vec2f& y) {							// Eq(5)
	if (x.dot(y) > 0) return 1;
	else return -1;
}

void init_ETF(const Mat& src, Mat& flow_field, Mat& grad_mg) {
	Mat grad_x, grad_y;

	Sobel(src, grad_x, CV_32FC1, 1, 0, 5);
	Sobel(src, grad_y, CV_32FC1, 0, 1, 5);
	magnitude(grad_x, grad_y, grad_mg);

	for (int r = 0; r < src.rows; r++) {
		for (int c = 0; c < src.cols; c++) {
			float u = grad_x.at<float>(r, c);
			float v = grad_y.at<float>(r, c);

			if ((u == 0.f) && (v == 0.f)) {
				grad_mg.at<float>(r, c) = 0.00001f;
				flow_field.at<Vec2f>(r, c) = Vec2f(1.f, 0.f);
			}
			else
				flow_field.at<Vec2f>(r, c) = normalize(Vec2f(u, v));
		}
	}
	rotate_field(flow_field, 90.f);
}

Mat refine_ETF(const Mat& src, const int ksize, Mat& flow_field, const Mat& grad_mg) {
	Mat refined_field = Mat::zeros(src.size(), CV_32FC2);
	int width = src.cols;
	int height = src.rows;
	parallel_for(int(0), height, [&](int h) {
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

Mat gaussian_blur(const Mat& src, int ksize, float sigma) {
	Mat result = src.clone();

	double exp = 0.f;
	vector<vector<double>> gaussian_kernel;
	gaussian_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 0.f));

	for (int i = 0; i < 2 * ksize + 1; i++) {
		for (int j = 0; j < 2 * ksize + 1; j++) {
			exp = -((pow(ksize - i, 2) + pow(ksize - j, 2)) / (2.f * sigma * sigma));
			gaussian_kernel[i][j] = (1 / (6.283184 * (2.f * sigma * sigma))) * pow(2.718282, exp);
		}
	}

	int height = src.rows;
	int width = src.cols;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			double blur = 0.f;
			double B_blur = 0.f;
			double G_blur = 0.f;
			double R_blur = 0.f;

			double sum = 0.f;

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (src.channels() == 3) {
						B_blur += (src.at<Vec3b>(r, c)[0] * gaussian_kernel[i][j]);
						G_blur += (src.at<Vec3b>(r, c)[1] * gaussian_kernel[i][j]);
						R_blur += (src.at<Vec3b>(r, c)[2] * gaussian_kernel[i][j]);
					}
					else if (src.channels() == 1)
						blur += (src.at<unsigned char>(r, c) * gaussian_kernel[i][j]);

					sum += gaussian_kernel[i][j];
				}
			}

			if (src.channels() == 3) {
				result.at<Vec3b>(h, w)[0] = (B_blur / sum);
				result.at<Vec3b>(h, w)[1] = (G_blur / sum);
				result.at<Vec3b>(h, w)[2] = (R_blur / sum);
			}
			else if (src.channels() == 1)
				result.at<unsigned char>(h, w) = (unsigned char)(blur / sum);
		}
	}

	return result;
}

Mat bilateral_filter(const Mat& src, const Mat& colRef_src, int ksize, float dist_sigma, float color_sigma) {
	Mat result = src.clone();

	double exp = 0.f;
	vector<vector<double>> gaussian_kernel;
	vector<vector<double>> bilateral_kernel;
	gaussian_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 0.f));

	for (int i = 0; i < 2 * ksize + 1; i++) {
		for (int j = 0; j < 2 * ksize + 1; j++) {
			exp = -((pow(ksize - i, 2) + pow(ksize - j, 2)) / (2.f * dist_sigma * dist_sigma));
			gaussian_kernel[i][j] = (1 / (6.283184 * (2.f * dist_sigma * dist_sigma))) * pow(2.718282, exp);
		}
	}

	int height = src.rows;
	int width = src.cols;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			bilateral_kernel.clear();
			bilateral_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 1.f));

			double blur = 0.f;
			double B_blur = 0.f;
			double G_blur = 0.f;
			double R_blur = 0.f;

			int center_B, center_G, center_R, center_C;
			int new_B, new_G, new_R, new_C;

			double weight = 0.f;
			double color_diff = 0.f;

			if (colRef_src.channels() == 3) {
				center_B = colRef_src.at<Vec3b>(h, w)[0];
				center_G = colRef_src.at<Vec3b>(h, w)[1];
				center_R = colRef_src.at<Vec3b>(h, w)[2];
			}
			else
				center_C = colRef_src.at<unsigned char>(h, w);

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (colRef_src.channels() == 3) {
						new_B = colRef_src.at<Vec3b>(r, c)[0];
						new_G = colRef_src.at<Vec3b>(r, c)[1];
						new_R = colRef_src.at<Vec3b>(r, c)[2];
						color_diff = sqrt((new_B - center_B) * (new_B - center_B) + (new_G - center_G) * (new_G - center_G) + (new_R - center_R) * (new_R - center_R));
					}
					else {
						new_C = colRef_src.at<unsigned char>(r, c);
						color_diff = sqrt((new_C - center_C) * (new_C - center_C));
					}

					exp = -((pow(color_diff, 2)) / (2.f * color_sigma * color_sigma));
					bilateral_kernel[i][j] = (1 / (6.283184 * (2.f * color_sigma * color_sigma))) * pow(2.718282, exp);
				}
			}

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (src.channels() == 3) {
						B_blur += (src.at<Vec3b>(r, c)[0] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
						G_blur += (src.at<Vec3b>(r, c)[1] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
						R_blur += (src.at<Vec3b>(r, c)[2] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
					}
					else if (src.channels() == 1)
						blur += (src.at<unsigned char>(r, c) * gaussian_kernel[i][j] * bilateral_kernel[i][j]);

					weight += gaussian_kernel[i][j] * bilateral_kernel[i][j];
				}
			}
			if (weight != 0.f) {
				if (src.channels() == 3) {
					result.at<Vec3b>(h, w)[0] = (unsigned char)(B_blur / weight);
					result.at<Vec3b>(h, w)[1] = (unsigned char)(G_blur / weight);
					result.at<Vec3b>(h, w)[2] = (unsigned char)(R_blur / weight);
				}
				else if (src.channels() == 1)
					result.at<unsigned char>(h, w) = (unsigned char)(blur / weight);
			}
		}
	}

	return result;
}

Mat alpha_blending(const Mat& src1, const Mat& src2, int alpha) {
	Mat result = src1.clone();

	int height = result.rows;
	int width = result.cols;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			if (src1.channels() == 3 && src2.channels() == 3) {
				result.at<Vec3b>(h, w)[0] = src2.at<Vec3b>(h, w)[0] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[0] * alpha / 100;
				result.at<Vec3b>(h, w)[1] = src2.at<Vec3b>(h, w)[1] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[1] * alpha / 100;
				result.at<Vec3b>(h, w)[2] = src2.at<Vec3b>(h, w)[2] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[2] * alpha / 100;
			}
			else if (src1.channels() == 1 && src2.channels() == 1)
				result.at<unsigned char>(h, w) = src2.at<unsigned char>(h, w) * (100 - alpha) / 100 + src1.at<unsigned char>(h, w) * alpha / 100;
		}
	}

	return result;
}

int main(void) {
	srand(1);

	const char* path = "D:\\lab\\ETF\\Image\\lenna_mid.jpg";
	Mat original_image = imread(path);
	Mat image;
	cvtColor(original_image, image, COLOR_BGR2GRAY);

	int width = image.cols;
	int height = image.rows;

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);

	const int ksize = 10;

	printf("refine ETF 1\n");
	init_ETF(image, flow_field, grad_mg);
	refined_field = refine_ETF(image, ksize, flow_field, grad_mg);
	for (int i = 2; i <= 5; i++) {
		printf("refine ETF %d\n", i);
		refined_field = refine_ETF(image, ksize, refined_field, grad_mg);
	}

	//for (int j = 0; j < height; j++)
	//	for (int i = 0; i < width; i++)
	//		refined_field.at<Vec2f>(j, i) = Vec2f(0.7071f, 0.7071f);
	//

	// Painting Test with a lot of Jittered Seed Point & Paper Test (+ - paperThreshold)
	const int length = 30;
	const float lic_sigma = 1.0f;
	dwLIC2 lic2(width, height, length, lic_sigma);
	lic2.init();

	printf("Grid\n");
	const int grid = 8;
	if ((width % grid) != 0 || (height % grid) != 0)
		cerr << "Grid Number ERROR" << endl;
	int widthGrid = (int)(width / grid);
	int heightGrid = (int)(height / grid);
	int totalGrid = widthGrid * heightGrid;

	vector<int> darkAvg(totalGrid, 0);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int jj = (int)(j / grid);
			int ii = (int)(i / grid);
			darkAvg.at(jj * widthGrid + ii) += (255 - image.at<unsigned char>(j, i));
		}
	}
	for (int i = 0; i < totalGrid; i++)
		darkAvg.at(i) /= (grid * grid);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int jj = (int)(j / grid);
			int ii = (int)(i / grid);
			lic2.getJitteredSeedPoint(i, j, 1);
			lic2.m_pImageField[j * width + i] = image.at<unsigned char>(j, i);
		}
	}

	printf("LIC \n");
	IplImage* pFlowImage;
	Mat ResultImg;

	pFlowImage.push_back(cvCreateImage(cvSize(width, height), 8, 1));

	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);

	lic2.drawSeedStraight(10, 1);
	//lic2.drawSeedCurve(20);

	printf("Image \n");
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			for (int k = 0; k < pFlowImage->nChannels; k++)
				pFlowImage->imageData[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k] = lic2.m_pLICField[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k];

	ResultImg = cvarrToMat(pFlowImage);

	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			lic2.setPaperField(i, j);
	
	int th = 4;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int value = lic2.m_pLICField[j * width + i] + (rand() % 3 - 1) * th * lic2.m_pPaperField[j * width + i];

			if (value > 255) value = 255;
			else if (value < 0) value = 0;

			result.at<unsigned char>(j, i) = (unsigned char)value;
		}
	}
	
	printf("FIN\n");
	imshow("result", result);
	imwrite("D:\\lab\\ETF\\lenna_mid_LICStraigt_k10r5_l30_cth84_32_224_pth4.jpg", result);
	waitKey(0);

	return 0;
}

