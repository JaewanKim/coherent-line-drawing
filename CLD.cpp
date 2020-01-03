// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define USES_CONVERSION

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <opencv2\core\core_c.h>
#include <vector>
#include "D:\lab\ETF\dwLIC\dwLIC.h"

using namespace cv;
using namespace std;

void rotate_field(Mat src, float theta) {
	printf("Start rotate field \n");

	int height = src.rows;
	int width = src.cols;
	Mat rotated = Mat::zeros(height, width, CV_32FC2);

	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			Vec2f v = src.at<Vec2f>(i, j);
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);

			rotated.at<Vec2f>(i, j) = Vec2f(rx, ry);
		}
	}
	// return rotated;
	printf("End rotate field \n");
}

float weight_spatial(int h, int w, int r, int c, float radius) { 	// Eq(2)

	if (sqrt(pow(h - c, 2) + pow(w - r, 2)) < radius) {
		return 1;
	}
	else {
		return 0;
	}
}

float weight_magnitude(float gradmg_x, float gradmg_y, float n) {	// Eq(3)
	return 0.5 * (1 + tanh(n * (gradmg_y - gradmg_x)));
}

float weight_direction(Vec2f& x, Vec2f& y) { 				// Eq(4)
	return abs(x.dot(y));
}

float get_phi(Vec2f& x, Vec2f& y) {							// Eq(5)
	if (x.dot(y) > 0) {
		return 1;
	}
	else {
		return -1;
	}
}

void init_ETF(Mat* pImg, Mat* pFlowField, Mat grad_mg) {

	printf("Start init ETF \n");

	Mat grad_x, grad_y;
	Mat flow_field = *pFlowField;
	Mat src = *pImg;

	Sobel(src, grad_x, CV_32FC1, 1, 0, 5);
	Sobel(src, grad_y, CV_32FC1, 0, 1, 5);
	magnitude(grad_x, grad_y, grad_mg);
	normalize(grad_mg, grad_mg, 0.0, 1.0, NORM_MINMAX);

	printf("src.rows: %d     ", src.rows);
	printf("src.cols: %d\n", src.cols);
	for (int r = 0; r < src.rows; r++) {
		for (int c = 0; c < src.cols; c++) {
			float u = grad_x.at<float>(r, c);
			float v = grad_y.at<float>(r, c);

			flow_field.at<Vec2f>(r, c) = normalize(Vec2f(v, u));
		}
	}

	rotate_field(flow_field, 90.0);
	printf("End init ETF \n");
}

Mat refine_ETF(Mat* pImg, int ksize, Mat flow_field, Mat grad_mg) {

	printf("Start refine ETF \n");
	Mat src = *pImg;
	Mat refined_field = Mat::zeros(src.size(), CV_32FC2);
	int width = src.cols;
	int height = src.rows;

	// Refine ETF
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			Vec2f t_cur_x = flow_field.at<Vec2f>(h, w);
			Vec2f t_new = Vec2f(0, 0);
			float k = 0.0;

			for (int r = h - ksize; r < h + ksize; r++) {
				for (int c = w - ksize; c < w + ksize; c++) {
					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					Vec2f t_cur_y = flow_field.at<Vec2f>(r, c);

					float phi = get_phi(t_cur_x, t_cur_y);
					float w_s = weight_spatial(h, w, c, r, ksize);
					float w_m = weight_magnitude(grad_mg.at<float>(h, w), norm(grad_mg.at<float>(r, c)), 1);
					float w_d = weight_direction(t_cur_x, t_cur_y);

					t_new += phi * t_cur_y * w_s * w_m * w_d;
					k += phi * w_s * w_m * w_d;
				}
			}
			// normalize t_new
			t_new /= k;
			refined_field.at<Vec2f>(h, w) = t_new;
		}
	}

	printf("End refine ETF \n");
	return refined_field;
}

int main(void) {
	Mat original_image = imread("D:\\lab\\ETF\\Image\\lenna.jpg");
	Mat image;
	//Mat norm_image;
	cvtColor(original_image, image, COLOR_BGR2GRAY);
	//normalize(norm_image, image, 0.0, 1.0, NORM_MINMAX, CV_32FC1);

	int width = image.cols;
	int height = image.rows;

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field2 = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);
	Mat temp = Mat::zeros(image.size(), CV_32FC3);

	Mat* pFlowField = &flow_field;
	Mat* pImg = &image;

	const int ksize = 5;

	printf("Main - Call init ETF \n");
	init_ETF(pImg, pFlowField, grad_mg);
	printf("Main - Call refine ETF \n");
	refined_field = refine_ETF(pImg, ksize, flow_field, grad_mg);


	// LIC
	printf("Main - Call LIC \n");
	dwLIC lic(width, height);

	printf("Main - Usage LIC - for문 \n");
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			lic.pVectr[(height - j - 1) * width * 2 + i * 2] = temp.at<Vec3f>(j, i)[0];
			lic.pVectr[(height - j - 1) * width * 2 + i * 2 + 1] = -temp.at<Vec3f>(j, i)[1];
		}
	}

	printf("Main - Usage LIC - FlowImaging \n");
	lic.FlowImagingLIC();

	//IplImage* pFlowImage = lic.GetLICimage();
	//USES_CONVERSION;
	//cvShowImage(W2A(str.GetString()), pFlowImage);

	printf("Main - Usage LIC - Convert to Mat \n");
	Mat* pFlowImage = lic.GetLICimage();
	USES_CONVERSION;

	namedWindow("ETF with dwLIC", WINDOW_AUTOSIZE);
	Mat matImg = cv::cvarrToMat(pFlowImage);
	Mat* pOutputImg = &matImg;
	imshow(" ", *pOutputImg);
	waitKey();

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
