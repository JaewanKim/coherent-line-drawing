// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#pragma warning(disable : 4996)
#define USES_CONVERSION
#define _USE_MATH_DEFINES

#include "CLD.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC.h"
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

Mat bilateral_filter(const Mat& src, const Mat& colRef_src, int ksize, double dist_sigma, double color_sigma) {
	Mat result = src.clone();

	long double exp = 0.f;
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

	parallel_for(int(0), height, [&](int h) {

		//for (int h = 0; h < height; h++) {
		vector<vector<double>> bilateral_kernel;
		for (int w = 0; w < width; w++) {

			bilateral_kernel.clear();
			bilateral_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 1.f));

			long double blur = 0.f;
			long double B_blur = 0.f;
			long double G_blur = 0.f;
			long double R_blur = 0.f;

			int center_B, center_G, center_R, center_C;
			int new_B, new_G, new_R, new_C;

			long double weight = 0.f;
			long double color_diff = 0.f;

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
						//B_blur += (src.at<Vec3b>(r, c)[0] * gaussian_kernel[c][r] * bilateral_kernel[c][r]);
						//G_blur += (src.at<Vec3b>(r, c)[1] * gaussian_kernel[c][r] * bilateral_kernel[c][r]);
						//R_blur += (src.at<Vec3b>(r, c)[2] * gaussian_kernel[c][r] * bilateral_kernel[c][r]);
					}
					else if (src.channels() == 1)
						//blur += (src.at<unsigned char>(r, c) * gaussian_kernel[c][r] * bilateral_kernel[c][r]);
						blur += (src.at<unsigned char>(r, c) * gaussian_kernel[i][j] * bilateral_kernel[i][j]);

					//weight += gaussian_kernel[c][r] * bilateral_kernel[c][r];
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
		});

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

float clamp(float x)
{
	if (x < 0)
		return 0.f;
	if (x > 1.f)
		return 1.f;
	else
		return x;

}

float peak(float pig)
{
	return clamp(1.0f * pig);
};

float valley(float pig)
{
	return clamp(2.f * pig - 1.f);
};

int main(void) {
	srand(1);

	const char* path = "D:\\lab\\ETF\\Image\\lenna_mid.jpg";
	printf("image ");
	Mat original_image = imread(path);
	printf("loaded\n");
	Mat image;
	cvtColor(original_image, image, COLOR_BGR2GRAY);

	int width = image.cols;
	int height = image.rows;

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);

	const int ksize = 4;

	printf("refine ETF 1\n");
	init_ETF(image, flow_field, grad_mg);
	refined_field = refine_ETF(image, ksize, flow_field, grad_mg);
	for (int i = 2; i <= 3; i++) {
		printf("refine ETF %d\n", i);
		refined_field = refine_ETF(image, ksize, refined_field, grad_mg);
	}

	// Paper Effect & Hatching on the dark (Darken Mode)
	const int length = 30;
	const float lic_sigma = 1.0f;
	dwLIC2 lic2(width, height, length, lic_sigma);
	lic2.init();

	printf("Grid\n");
	const int grid = 4;
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
			lic2.setBlackNoiseField(i, j, 255 - darkAvg.at(jj * widthGrid + ii));
			lic2.m_pImageField[j * width + i] = image.at<unsigned char>(j, i);
		}
	}

	printf("Level\n");
	const int level = 2;
	vector<Mat> rotated_field;

	for (int i = 0; i < level; ++i)
		rotated_field.push_back(Mat::zeros(image.size(), CV_32FC2));

	vector<float> degree;
	degree.push_back(0.f);
	degree.push_back(90.f);

	for (int lev = 0; lev < level; lev++) {
		printf("ROT %.1f\n", degree.at(lev));
		Mat rotated = rotate_field2(refined_field, degree.at(lev));

		for (int j = 0; j < height; j++)
			for (int i = 0; i < width; i++)
				rotated_field.at(lev).at<Vec2f>(j, i) = Vec2f(rotated.at<Vec2f>(j, i)[0], rotated.at<Vec2f>(j, i)[1]);
	}

	printf("LIC \n");
	vector<IplImage*> pFlowImage;
	vector<Mat> Img, GaussianImg, BilateralImg;

	for (int i = 0; i < level; i++)
		pFlowImage.push_back(cvCreateImage(cvSize(width, height), 8, 1));

	for (int lev = 0; lev < level; lev++) {

		for (int j = 0; j < height; j++)
			for (int i = 0; i < width; i++)
				lic2.setFlowField(i, j, rotated_field.at(lev).at<Vec2f>(j, i)[0], rotated_field.at(lev).at<Vec2f>(j, i)[1]);

		//lic2.doLICForward(1);	// NULL:Average, 0:Average, 1:Linear, 2:Gaussian
		lic2.doLICStraight(1);	// NULL:Average, 0:Average, 1:Linear, 2:Gaussian

		printf("Image \n");
		for (int j = 0; j < height; j++)
			for (int i = 0; i < width; i++)
				for (int k = 0; k < pFlowImage.at(lev)->nChannels; k++)
					pFlowImage.at(lev)->imageData[j * pFlowImage.at(lev)->widthStep + i * pFlowImage.at(lev)->nChannels + k] = lic2.m_pLICField[j * pFlowImage.at(lev)->widthStep + i * pFlowImage.at(lev)->nChannels + k];

		Mat MatImg = cvarrToMat(pFlowImage.at(lev));
		Img.push_back(MatImg);
	}

	// Hatching -> Paper
	/*
	Mat ResultImg = Img.at(0).clone();
	printf("Hatching \n");

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			for (int k = 0; k < pFlowImage.at(0)->nChannels; k++) {

				float avg_darkness = 0.f;
				float weight = 1.f;
				float tot = 0.f;

				for (int ii = 0; ii < 2 * ksize + 1; ii++) {
					for (int jj = 0; jj < 2 * ksize + 1; jj++) {
						int r = j - ksize + jj;
						int c = i - ksize + ii;

						if (r < 0 || r >= height || c < 0 || c >= width)
							continue;

						avg_darkness += (255 - lic2.m_pImageField[r * pFlowImage.at(0)->widthStep + c * pFlowImage.at(0)->nChannels + k]) * weight;
						tot += weight;
					}
				}

				if (tot != 0)
					avg_darkness /= tot;

				if (255 - lic2.m_pImageField[j * pFlowImage.at(0)->widthStep + i * pFlowImage.at(0)->nChannels + k] > avg_darkness) {		// result
				//if (255 - lic2.m_pImageField[j * pFlowImage.at(0)->widthStep + i * pFlowImage.at(0)->nChannels + k] > 164) {				// result
				//if (255 - Img.at(1).at<unsigned char>(j, i) > 164) {																		// result2
					if (Img.at(0).at<unsigned char>(j, i) < Img.at(1).at<unsigned char>(j, i))
						ResultImg.at<unsigned char>(j, i) = Img.at(0).at<unsigned char>(j, i);
					else
						ResultImg.at<unsigned char>(j, i) = Img.at(1).at<unsigned char>(j, i);
				}
			}
		}
	}

	printf("Paper \n");
	Mat result_paper = ResultImg.clone();
	imwrite("D:\\lab\\ETF\\result_minAvg.jpg", ResultImg);

	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			lic2.setPaperField(i, j);

	Mat paper_color = imread("D:\\lab\\ETF\\Image\\paper2.jpg");
	Mat paper;
	cvtColor(paper_color, paper, COLOR_BGR2GRAY);

	const float resizeRatio = 1;
	Mat tinyNoiseField = Mat::zeros(cvSize(width* resizeRatio, height* resizeRatio), CV_8UC1);
	Mat NoiseField = Mat::zeros(cvSize(width, height), CV_8UC1);

	for (int j = 0; j < height * resizeRatio; j++)
		for (int i = 0; i < width * resizeRatio; i++)
			tinyNoiseField.at<unsigned char>(j, i) = (unsigned char)(paper.at<unsigned char>(j, i));
			//tinyNoiseField.at<unsigned char>(j, i) = (unsigned char)(lic2.m_pPaperField[j * width + i] * 255);
	resize(tinyNoiseField, NoiseField, Size(width, height), 0, 0, CV_INTER_CUBIC);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			float pigment = 1.f - (ResultImg.at<unsigned char>(j, i)) / 255.f;
			float height = NoiseField.at<unsigned char>(j, i) / 255.f;
			//float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
			float weight = height * peak(pigment) + (1.f - height) * valley(pigment);

			int value = (255 * weight) + 0.5f;

			if (value > 255) value = 255;
			else if (value < 0) value = 0;

			result_paper.at<unsigned char>(j, i) = (unsigned char)(255 - value);
			//result_paper.at<unsigned char>(j, i) = (unsigned char)(weight * 255);
		}
	}
	*/

	// Paper -> Hatching
	printf("Paper \n");
	vector<Mat> TexturedImg;
	for (int lev = 0; lev < level; lev++)
		TexturedImg.push_back(Img.at(lev));

	printf("image ");
	Mat paper_color = imread("D:\\lab\\ETF\\Image\\paper2.jpg");
	printf("loaded\n");
	Mat paper;
	cvtColor(paper_color, paper, COLOR_BGR2GRAY);

	const float resizeRatio = 1;
	Mat tinyNoiseField = Mat::zeros(cvSize(width * resizeRatio, height * resizeRatio), CV_8UC1);
	Mat NoiseField = Mat::zeros(cvSize(width, height), CV_8UC1);

	for (int j = 0; j < height * resizeRatio; j++)
		for (int i = 0; i < width * resizeRatio; i++)
			tinyNoiseField.at<unsigned char>(j, i) = (unsigned char)(paper.at<unsigned char>(j, i));
			//tinyNoiseField.at<unsigned char>(j, i) = (unsigned char)(lic2.m_pPaperField[j * width + i] * 255);

	resize(tinyNoiseField, NoiseField, Size(width, height), 0, 0, CV_INTER_CUBIC);

	for (int lev = 0; lev < level; lev++) {
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				float pigment = 1.f - (Img.at(lev).at<unsigned char>(j, i)) / 255.f;
				float height = NoiseField.at<unsigned char>(j, i) / 255.f;
				//float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
				float weight = height * peak(pigment) + (1.f - height) * valley(pigment);

				int value = (255 * weight) + 0.5f;

				if (value > 255) value = 255;
				else if (value < 0) value = 0;

				TexturedImg.at(lev).at<unsigned char>(j, i) = (unsigned char)(255 - value);
				//result_paper.at<unsigned char>(j, i) = (unsigned char)(weight * 255);
			}
		}
	}
	printf("image ");
	imwrite("D:\\lab\\ETF\\lennaM_biLinearAlpha10_textured1.jpg", TexturedImg.at(0));
	printf("saved\n");
	printf("image ");
	imwrite("D:\\lab\\ETF\\lennaM_biLinearAlpha10_textured2.jpg", TexturedImg.at(1));
	printf("saved\n");

	// Bilateral Filtering on Reference Image
	printf("Bilateral \n");
	BilateralImg.push_back(bilateral_filter(image, original_image, 5, 15.f, 40.f));

	printf("image ");
	imwrite("D:\\lab\\ETF\\lennaM_biLinearAlpha10_refImg.jpg", BilateralImg.at(0));
	printf("saved\n");

	Mat ResultImg = Img.at(0).clone();

	printf("Hatching \n");
	parallel_for(int(0), height, [&](int j) {
		for (int i = 0; i < width; i++) {
			for (int k = 0; k < pFlowImage.at(0)->nChannels; k++) {

				long double avg_darkness = 0.f;
				long double weight = 1.f;
				long double tot = 0.f;

				for (int ii = 0; ii < 2 * ksize + 1; ii++) {
					for (int jj = 0; jj < 2 * ksize + 1; jj++) {
						int r = j - ksize + jj;
						int c = i - ksize + ii;

						if (r < 0 || r >= height || c < 0 || c >= width)
							continue;

						// biLinear
						//if (sqrt((j - r) * (j - r) + (i - c) * (i - c) > ksize))
						//	continue;
						weight = ksize - sqrt((j - r) * (j - r) + (i - c) * (i - c));

						//weight = gaussian_kernel[r][c];
						//avg_darkness += (255 - lic2.m_pImageField[r * pFlowImage.at(0)->widthStep + c * pFlowImage.at(0)->nChannels + k]) * weight;
						avg_darkness += (255 - BilateralImg.at(0).at<unsigned char>(r, c)) * weight;
						tot += weight;
					}
				}

				if (tot != 0)
					avg_darkness /= tot;

				if (avg_darkness < 0) avg_darkness = 0.f;
				else if (avg_darkness > 255) avg_darkness = 255.f;

				//if (255 - BilateralImg.at(0).at<unsigned char>(j, i) > (unsigned char)avg_darkness) {		// result
				//if (255 - lic2.m_pImageField[j * pFlowImage.at(0)->widthStep + i * pFlowImage.at(0)->nChannels + k] > avg_darkness) {		// result
				//if (255 - lic2.m_pImageField[j * pFlowImage.at(0)->widthStep + i * pFlowImage.at(0)->nChannels + k] > 164) {				// result
				//if (255 - Img.at(1).at<unsigned char>(j, i) > 164) {																		// result2
				if (255 - BilateralImg.at(0).at<unsigned char>(j, i) > 128) {																		// result2
					//if (BilateralImg.at(0).at<unsigned char>(j, i) < BilateralImg.at(1).at<unsigned char>(j, i))
					//	ResultImg.at<unsigned char>(j, i) = Img.at(0).at<unsigned char>(j, i);
					//else
					//	ResultImg.at<unsigned char>(j, i) = Img.at(1).at<unsigned char>(j, i);
					if (Img.at(0).at<unsigned char>(j, i) < Img.at(1).at<unsigned char>(j, i))
						ResultImg.at<unsigned char>(j, i) = Img.at(0).at<unsigned char>(j, i);
					//ResultImg.at<unsigned char>(j, i) = Img.at(0).at<unsigned char>(j, i) * 0.7 + Img.at(1).at<unsigned char>(j, i) * 0.3;
					else
						ResultImg.at<unsigned char>(j, i) = Img.at(1).at<unsigned char>(j, i);
					//ResultImg.at<unsigned char>(j, i) = Img.at(0).at<unsigned char>(j, i) * 0.3 + Img.at(1).at<unsigned char>(j, i) * 0.7;
				}
			}
		}
		});

	printf("FIN\n");
	imshow("result", ResultImg);
	printf("image ");
	imwrite("D:\\lab\\ETF\\lennaM_biLinearAlpha10_textured12.jpg", ResultImg);
	printf("saved\n");
	waitKey(0);

	return 0;
}

