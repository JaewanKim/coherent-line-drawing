
// CLD.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#pragma warning(disable : 4996)
#define USES_CONVERSION
#define _USE_MATH_DEFINES

#include "ETF.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC2.h"
#include "D:\lab\ETF\dwLIC\dwColors.h"
#include "opencv2/highgui/highgui_c.h"
#include <ppl.h>
#include <random>

using namespace cv;
using namespace std;
using namespace concurrency;

int randomRange(int n1, int n2) {
	return (int)(rand() % (n2 - n1 + 1)) + n1;
}

int main(void) {

	ETF etf;
	dwColors color;
	
	const char* path = "D:\\lab\\ETF\\Image\\lenna.jpg";
	Mat originalImg = imread(path);

	int width = originalImg.cols;
	int height = originalImg.rows;

	// HSV Jittering
	vector<Mat> hsvField;
	hsvField.reserve(3);
	for (int idx = 0; idx < 3; idx++)
		hsvField.push_back(Mat::zeros(originalImg.size(), CV_32FC1));

	Mat jitteredImg = Mat::zeros(originalImg.size(), CV_8UC3);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			const float hRange = 12.5;
			const float sRange = 0.5;
			const float vRange = 32;
			float jitteredH = randomRange(-hRange, hRange);
			float jitteredS = randomRange(-sRange, sRange);
			float jitteredV = randomRange(-vRange, vRange);

			hsvField.at(0).at<float>(j, i) = jitteredH;
			hsvField.at(1).at<float>(j, i) = jitteredS;
			hsvField.at(2).at<float>(j, i) = jitteredV;
		}
	}

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int b = originalImg.at<Vec3b>(j, i)[0];
			int g = originalImg.at<Vec3b>(j, i)[1];
			int r = originalImg.at<Vec3b>(j, i)[2];
			float h, s, v;
			color.RGB2HSV(r, g, b, h, s, v);

			float imgH = h + hsvField.at(0).at<float>(j, i);
			float imgS = s + hsvField.at(1).at<float>(j, i);
			float imgV = v + hsvField.at(2).at<float>(j, i);

			if (imgH < 0.f) imgH = 0.f;	else if (imgH > 360.f) imgH = 360.f;
			if (imgS < 0.f) imgS = 0.f;	else if (imgS > 1.f) imgS = 1.f;
			if (imgV < 0.f) imgV = 0.f;	else if (imgV > 255.f) imgV = 255.f;

			int imgR, imgG, imgB;

			color.HSV2RGB(imgH, imgS, imgV, imgR, imgG, imgB);
			if (imgR < 0 || imgR > 255) imgR = r;
			if (imgG < 0 || imgG > 255) imgG = g;
			if (imgB < 0 || imgB > 255) imgB = b;

			jitteredImg.at<Vec3b>(j, i)[0] = (unsigned char)imgB;
			jitteredImg.at<Vec3b>(j, i)[1] = (unsigned char)imgG;
			jitteredImg.at<Vec3b>(j, i)[2] = (unsigned char)imgR;
		}
	}

	// ETF
	Mat image;
	cvtColor(originalImg, image, COLOR_BGR2GRAY);

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);

	const int ksize = 8;

	printf("refine ETF 1\n");
	etf.init_ETF(image, flow_field, grad_mg);
	refined_field = etf.refine_ETF(image, ksize, flow_field, grad_mg);
	for (int i = 2; i <= 4; i++) {
		printf("refine ETF %d\n", i);
		refined_field = etf.refine_ETF(image, ksize, refined_field, grad_mg);
	}

	// dwLIC2
	IplImage Img = cvIplImage(jitteredImg);
	IplImage* pImg = &Img;

	IplImage* pFlowImage = NULL;

	const int length = 30;
	const float step = 1.0f;

	printf("LIC\n");
	dwLIC2 lic2(width, height, length, step, pImg);
	lic2.init();

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
			lic2.setNoiseField(i, j);
		}
	}

	lic2.doLICForward(2);
	//lic2.doLICStraight(1);

	if (lic2.isImgExist())
		pFlowImage = cvCreateImage(cvSize(width, height), 8, 3);
	else
		pFlowImage = cvCreateImage(cvSize(width, height), 8, 1);

	printf("FLOW IMAGE \n");
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			for (int k = 0; k < pFlowImage->nChannels; k++)
				pFlowImage->imageData[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k] = lic2.m_pLICField[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k];

	Mat MatImg = cvarrToMat(pFlowImage);


	printf("FIN\n");
	cvShowImage(" ", pFlowImage);
	imwrite("D:\\lab\\ETF\\jitteredHSV_125_0.5_32_Gauss_k8r4_rk4.jpg", MatImg);
	waitKey(0);
	cvReleaseImage(&pFlowImage);

	hsvField.clear();

	return 0;
}
