// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include "CLD.h"
#include "D:/JaewanKim/lab/dwLIC/dwLIC.h"

using namespace cv;
using namespace std;


Mat rotate_field(Mat& src, float theta) {

	int width = src.cols;
	int height = src.rows;
	Mat rotated = Mat::zeros(height, width, CV_32FC1);
	
	for (int i = 0; i < src.rows; ++i) {
		for (int j = 0; j < src.cols; ++j) {

			Vec2f v = src.at<Vec2f>(i, j);
			float rx = v[0] * cos(theta) - v[1] * sin(theta);
			float ry = v[1] * cos(theta) + v[0] * sin(theta);

			rotated.at<Vec2f>(i, j) = Vec2f(rx, ry);
		}
	}
}

float weight_spatial(int h, int w, int x, int y, int r) { 	// Eq(2)
	
	float distance = sqrt(pow(h - y, 2) + pow(w - x, 2));
	float radius = (float)r;

	if (distance < radius) {
		return 1;
	}
	else {
		return 0;
	}
}

float weight_magnitude (float gradmg_x, float gradmg_y) {	// Eq(3)
	float wm = (1 + tanh(gradmg_y - gradmg_x)) / 2;
	return wm;
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
int main() {

	// Init ETF
	Mat original_image = imread("D:\\JaewanKim\\lab\\Image\\lenna.png");
	Mat image;
	normalize(original_image, image, 0.0, 1.0, NORM_MINMAX, CV_32FC1);

	Mat grad_x, grad_y;
	Mat gradientMag;
	Sobel(original_image, grad_x, CV_32FC1, 1, 0, 5);
	Sobel(original_image, grad_y, CV_32FC1, 0, 1, 5);

	magnitude(grad_x, grad_y, gradientMag);
	normalize(gradientMag, gradientMag, 0.0, 1.0, NORM_MINMAX);

	Mat flow_field = Mat::zeros(image.size(), CV_32FC3);
	rotate_field(flow_field, 90.0);

	Mat grad;
	int height = image.rows;
	int width = image.cols;
	int kernel = 5;

	// Refine ETF
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			Vec2f t_cur_x = flow_field.at<Vec2f>(h, w);
			Vec2f t_new = Vec2f(0, 0);

			for (int r = h - kernel; r < h + kernel; r++) {
				for (int c = w - kernel; c < w + kernel; c++) {
					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					Vec2f t_cur_y = flow_field.at<Vec2f>(r, c);

					float phi = get_phi(t_cur_x, t_cur_y);
					float w_s = weight_spatial(h, w, c, r, kernel);
					float w_m = weight_magnitude(gradientMag.at<float>(h, w), norm(gradientMag.at<float>(r, c)));
					float w_d = weight_direction(t_cur_x, t_cur_y);

					t_new += phi * t_cur_y * w_s * w_m * w_d;
				}
			}
			normalize(t_new);
		}
		// LIC
		dwLIC lic(width, height);

		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				lic.pVectr[(height - j - 1) * width * 2 + i * 2] = temp.tx;
				lic.pVectr[(height - j - 1) * width * 2 + i * 2 + 1] = -temp.ty;
			}
		}

		if (m_bVisualizeFlow) {
			lic.FlowImagingLIC();
			IplImage* pFlowImage = lic.GetLICimage();
			USES_CONVERSION;
			cvShowImage(W2A(str.GetString()), pFlowImage);
		}

		return 0;
	}



	namedWindow("Grayscale", WINDOW_AUTOSIZE);
	imshow("Grayscale", image);
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
