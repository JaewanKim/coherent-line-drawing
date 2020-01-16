// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#define USES_CONVERSION
#define _USE_MATH_DEFINES

#include "CLD.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <opencv2\core\core_c.h>
#include <vector>
#include "D:\lab\ETF\dwLIC\dwLIC.h"

using namespace cv;
using namespace std;

void rotate_field(Mat& flowField, float degree) {
	printf("Start rotate field \n");
	const float theta = degree / 180.0 * M_PI;

	for (int i = 0; i < flowField.rows; i++) {
		for (int j = 0; j < flowField.cols; j++) {
			Vec2f v = flowField.at<Vec2f>(i, j);
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);

			flowField.at<Vec2f>(i, j) = Vec2f(rx, ry);
		}
	}
	printf("End rotate field \n");
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

float weight_direction(Vec2f& x, Vec2f& y) { 				// Eq(4)
	return abs(x.dot(y));
}

float get_phi(Vec2f& x, Vec2f& y) {							// Eq(5)
	if (x.dot(y) > 0) return 1;
	else return -1;
}

void init_ETF(const Mat& src, Mat& flow_field, Mat& grad_mg) {

	printf("Start init ETF \n");

	Mat grad_x, grad_y;

	Sobel(src, grad_x, CV_32FC1, 1, 0, 5);
	Sobel(src, grad_y, CV_32FC1, 0, 1, 5);
	magnitude(grad_x, grad_y, grad_mg);
	normalize(grad_mg, grad_mg, 0.0, 1.0, NORM_MINMAX);

	for (int r = 0; r < src.rows; r++) {
		for (int c = 0; c < src.cols; c++) {
			float u = grad_x.at<float>(r, c);
			float v = grad_y.at<float>(r, c);

			if ((u == 0.f) && (v == 0.f)) {
				grad_mg.at<Vec2f>(r, c) = 0.00001f;
				flow_field.at<Vec2f>(r, c) = Vec2f(1.f, 0.f);
			}
			else
				flow_field.at<Vec2f>(r, c) = normalize(Vec2f(u, v));
		}
	}

	rotate_field(flow_field, 90.f);
	printf("End init ETF \n");
}

Mat refine_ETF(const Mat& src, const int ksize, Mat& flow_field, const Mat& grad_mg) {

	printf("Start refine ETF \n");
	Mat refined_field = Mat::zeros(src.size(), CV_32FC2);
	int width = src.cols;
	int height = src.rows;

	// Refine ETF
	for (int h = 0; h < height; h++) {
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

			// normalize t_new
			if (k != 0.f)
				t_new /= k;
			refined_field.at<Vec2f>(h, w) = normalize(t_new);
		}
	}
	printf("End refine ETF \n");
	return refined_field;
}

int main(void) {
	Mat original_image = imread("D:\\lab\\ETF\\Image\\lenna.jpg");
	Mat image;
	cvtColor(original_image, image, COLOR_BGR2GRAY);

	int width = image.cols;
	int height = image.rows;

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);

	const int ksize = 5;

	printf("Main - Call init ETF \n");
	init_ETF(image, flow_field, grad_mg);
	printf("Main - Call refine ETF \n");
	refined_field = refine_ETF(image, ksize, flow_field, grad_mg);
	for (int i = 0; i < 3; i++) {
		refined_field = refine_ETF(image, ksize, refined_field, grad_mg);
	}

	/*
	// 파일 입력

	float* pMemory;
	pMemory = new float[width * height * 2];


	FILE* in;
	float ch;
	std::vector<float> v;

	if ((in = fopen("D:\\lab\\ETF\\SampleFiles\\lennajpg.etf", "rb")) == NULL) {
		fputs("fopen err", stderr);
		exit(1);
	}

	fread(pMemory, sizeof(float), width * height * 2, in);

	int idx = 0;
	for (int j = 0; j < height; j++) {		// Mat temp로 차곡차곡 옮김
		for (int i = 0; i < width; i++) {
			temp.at<Vec2f>(j, i)[0] = pMemory[idx++];
			temp.at<Vec2f>(j, i)[1] = pMemory[idx++];
		}
	}

	fclose(in);
	*/

	// LIC
	printf("Main - Call LIC \n");
	dwLIC lic(width, height);

	printf("Main - Usage LIC - for문 \n");
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			lic.pVectr[(height - j - 1) * width * 2 + i * 2] = refined_field.at<Vec2f>(j, i)[0];
			lic.pVectr[(height - j - 1) * width * 2 + i * 2 + 1] = -refined_field.at<Vec2f>(j, i)[1];
		}
	}

	printf("Main - Usage LIC - FlowImaging \n");
	lic.FlowImagingLIC();

	IplImage* pFlowImage = lic.GetLICimage();
	if (pFlowImage != NULL) {
		USES_CONVERSION;
		cvShowImage(" ", pFlowImage);
		waitKey();
		cvReleaseImage(&pFlowImage);
	}

	return 0;
}


// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
