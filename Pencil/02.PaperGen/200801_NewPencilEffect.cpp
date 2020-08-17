//
//#define _CRT_SECURE_NO_WARNINGS
//
//#include "PencilPaperTest.h"
//#include <random>
//#include <vector>
//#include <algorithm>
//#include <opencv.hpp>
//
//#include "200801_ETF.h"
//#include "200801_ImageProcess.h"
//#include "200801_dwLIC2.h"
//#include "paper.h"
//
//using namespace cv;
//using namespace std;
//
//random_device rn;
//mt19937_64 rnd(rn());
//uniform_int_distribution<int> range(0, 255);
//
///*
//	이미지 입력
//		RefImg : 채널1의 흑백으로 bilateral filtering을 수행
//			이미지의 특징인 edge는 살리면서 이미지 내 noise를 제거하기 위함
//		RefImg로부터 ETF 생성
//
//	LIC 수행
//		랜덤하게 Seed (0~255) 생성
//		white noise에 대하여 ETF를 참고한 LIC 수행
//
//	heightMap(종이 텍스쳐) 적용
//		방법 3 : grayscale의 LIC 결과와 방법 2의 종이를 참고한 새로운 height map 생성
//		고려되지 않은 RefImg의 밝기값 등에 대해서는 차차 보완해나갈 예정 (우선은 연필의 느낌이 나는가를 확인해야함)
//*/
//
//int main() {
//
//	ETF etf;
//	ImgProcess ip;
//	Paper p;
//	
//	srand(1);
//
//	const char* path = "D:\\lab\\ETF\\Image\\oldman1.jpg";
//	Mat original_image, image;
//	original_image = imread(path);
//	cvtColor(original_image, image, COLOR_BGR2GRAY);
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\init_source.jpg", image);
//	
//	image = ip.bilateral_filter(image, original_image, 5, 5.f, 30.f);
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\init_bilateral5-5-30.jpg", image);
//
//	int width = image.cols;
//	int height = image.rows;
//
//	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
//	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
//	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);
//
//	const int ksize = 4;
//
//	printf("refine ETF 1\n");
//	etf.init_ETF(image, flow_field, grad_mg);
//	refined_field = etf.refine_ETF(image, ksize, flow_field, grad_mg);
//	for (int i = 2; i <= 3; i++) {
//		printf("refine ETF %d\n", i);
//		refined_field = etf.refine_ETF(image, ksize, refined_field, grad_mg);
//	}
//
//	const int length = 100;
//	const float lic_sigma = 100.f;
//	dwLIC2 lic2(width, height, length, lic_sigma);
//	lic2.init();
//
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) { 
//			lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
//			lic2.setNoiseField(i, j, range(rnd));
//		}
//	}
//
//	lic2.doLICForward(1);
//
//	Mat LICImg = Mat::zeros(Size(width, height), CV_8UC1);
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) {
//			LICImg.at<unsigned char>(j, i) = lic2.m_pLICField[j * LICImg.step1() + i];
//		}
//	}
//
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\lic_length100-flag1.jpg", LICImg);
//
//	Mat preGenPaper = imread("D:\\lab\\ETF\\Output\\200803\\preGenPaper\\1\\n10_standard128_offset32_min96_max160_jittered16.jpg");
//	Mat newPaper = Mat::zeros(Size(width, height), CV_8UC1);
//
//	// Paper Generate
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) {
//			unsigned char intensity = LICImg.at<unsigned char>(j, i);
//			newPaper.at<unsigned char>(j, i) = preGenPaper.at<unsigned char>(intensity, i);
//		}
//	}
//
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\newPaper1d.jpg", newPaper);
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\newPaper11d.jpg", newPaper);
//	
//
//	const int lengthB = 100;
//	const float lic_sigmaB = 100.f;
//	dwLIC2 lic2black(width, height, lengthB, lic_sigmaB);
//	lic2black.init();
//
//	printf("Grid\n");
//	const int grid = 4;
//	if ((width % grid) != 0 || (height % grid) != 0)
//		cerr << "Grid Number ERROR" << endl;
//	int widthGrid = (int)(width / grid);
//	int heightGrid = (int)(height / grid);
//	int totalGrid = widthGrid * heightGrid;
//
//	vector<int> darkAvg(totalGrid, 0);
//
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) {
//			int jj = (int)(j / grid);
//			int ii = (int)(i / grid);
//			darkAvg.at(jj * widthGrid + ii) += (255 - image.at<unsigned char>(j, i));
//		}
//	}
//	for (int i = 0; i < totalGrid; i++)
//		darkAvg.at(i) /= (grid * grid);
//
//	printf("bnoise?\n");
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) {
//			int jj = (int)(j / grid);
//			int ii = (int)(i / grid);
//			lic2black.setBlackNoiseField(i, j, 255 - darkAvg.at(jj * widthGrid + ii));
//			int threshold = 255 - darkAvg.at(jj * widthGrid + ii);
//			int index = width * j + i;
//			lic2black.m_pNoiseField[index] = (range(rnd) > threshold) ? (unsigned char)0 : (unsigned char)255;
//
//			lic2black.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
//			//lic2black.m_pImageField[j * width + i] = image.at<unsigned char>(j, i);
//		}
//	}
//
//	printf("licfor?\n");
//	lic2black.doLICStraight(1);	// NULL:Average, 0:Average, 1:Linear, 2:Gaussian
//
//	Mat LICBlackImg = Mat::zeros(Size(width, height), CV_8UC1);
//	for (int j = 0; j < height; j++) {
//		for (int i = 0; i < width; i++) {
//			LICBlackImg.at<unsigned char>(j, i) = lic2black.m_pLICField[j * LICBlackImg.step1() + i];
//		}
//	}
//	printf("imwrite?\n");
//	//imshow("LICBlackImg", LICBlackImg);
//	//waitKey(0);
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\BlackImg_LICForward.jpg", LICBlackImg);
//
//
//	Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//    for (int j = 0; j < height; j++) {
//        for (int i = 0; i < width; i++) {
//            float pigment = 1.f - (LICBlackImg.at<unsigned char>(j, i)) / 255.f;
//            float altitude = 1.f - (newPaper.at<unsigned char>(j, i)) / 255.f;
//            //float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
//            float weight = altitude * p.peak(pigment) + (1.f - altitude) * p.valley(pigment);
//            int value = (255 * weight) + 0.5f;
//            if (value > 255)    value = 255;
//            else if (value < 0) value = 0;
//            ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//            //result_paper.at<unsigned char>(j, i) = (unsigned char)(weight * 255);
//        }
//    }
//
//	imshow("ResultImg", ResultImg);
//	waitKey(0);
//	//imwrite("D:\\lab\\ETF\\Output\\200803\\result(bilateralImg+newPaper)2d_blackforward.jpg", ResultImg);
//
//	return 0;
//}