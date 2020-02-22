
#pragma warning(disable : 4996)

#define USES_CONVERSION
#define _USE_MATH_DEFINES

#include "HatchingTest2.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC2.h"

using namespace cv;
using namespace std;

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

			if (k != 0.f)
				t_new /= k;
			refined_field.at<Vec2f>(h, w) = normalize(t_new);
		}
	}
	return refined_field;
}

int main(void) {
	srand(1);

	const char* path = "D:\\lab\\ETF\\Image\\lenna.jpg";
	Mat original_image = imread(path);
	Mat image;
	cvtColor(original_image, image, COLOR_BGR2GRAY);

	int width = image.cols;
	int height = image.rows;

	Mat flow_field = Mat::zeros(image.size(), CV_32FC2);
	Mat refined_field = Mat::zeros(image.size(), CV_32FC2);
	Mat grad_mg = Mat::zeros(image.size(), CV_32FC1);

	const int ksize = 5;

	printf("refine ETF 1\n");
	init_ETF(image, flow_field, grad_mg);
	refined_field = refine_ETF(image, ksize, flow_field, grad_mg);
	for (int i = 2; i <= 4; i++) {
		printf("refine ETF %d\n", i);
		refined_field = refine_ETF(image, ksize, refined_field, grad_mg);
	}

	//for (int h = 0; h < height; h++) {
	//	for (int w = 0; w < width; w++) {
	//		refined_field.at<Vec2f>(h, w)[0] = 1.f;
	//		refined_field.at<Vec2f>(h, w)[1] = 0.f;
	//	}
	//}

	// Hatching Test with BlackNoise (Simple Sum Rotated LIC)
	const int length = 30;
	const float gauss_sigma = 1.0f;
	dwLIC2 lic2(width, height, length, gauss_sigma);

	const int level = 3;
	vector<IplImage*> pRotateField;
	for (int i = 0; i < level; ++i)
		pRotateField.push_back(cvCreateImage(cvSize(width, height), 8, 1));

	for (int lev = 0; lev < level; lev++) {
		printf("ROT %d\n", lev * 45);
		rotate_field(refined_field, 45.f);

		lic2.init();
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
				lic2.setBlackNoiseField(i, j, 240);
			}
		}

		printf("LIC %d\n", lev);
		lic2.doLICForward();

		for (int j = 0; j < height; j++)
			for (int i = 0; i < width; i++)
				for (int k = 0; k < pRotateField.at(lev)->nChannels; k++)
					pRotateField.at(lev)->imageData[j * pRotateField.at(lev)->widthStep + i * pRotateField.at(lev)->nChannels + k] = lic2.m_pLICField[j * pRotateField.at(lev)->widthStep + i * pRotateField.at(lev)->nChannels + k];

		//char name[2] = { ' ', '\0' };
		//itoa(lev, name, 10);
		//if (pRotateField.at(lev) != NULL) {
		//	printf("NOT NULL\n");
		//	cvShowImage(name, pRotateField.at(lev));
		//	waitKey(0);
		//}
	}

	//if (pRotateField.at(0) != NULL && pRotateField.at(1) != NULL && pRotateField.at(2) != NULL) {
	//	cvShowImage("RoateFlow 0", pRotateField.at(0));
	//	cvShowImage("RoateFlow 1", pRotateField.at(1));
	//	cvShowImage("RoateFlow 2", pRotateField.at(2));
	//	waitKey(0);
	//}

	// level 수 만큼의 명암 단계 설정
	IplImage* pFlowImage = NULL;
	pFlowImage = cvCreateImage(cvSize(width, height), 8, 1);

	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			for (int k = 0; k < pRotateField.at(0)->nChannels; k++)
				pFlowImage->imageData[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k] = 0;

	unsigned char tmp = (int)(256 / (level + 1));
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			for (int k = 0; k < pFlowImage->nChannels; k++) {
				//unsigned char sum = 0;
				unsigned int sum = 0;

				unsigned char darkness = 255 - image.at<unsigned char>(j, i);

				if (darkness >= 0 && darkness < tmp)		// level 0
					pFlowImage->imageData[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k] = (unsigned char)255;
				else {
					for (int idx = 0; idx < level; idx++)	// level 1 ~ n
						if (darkness >= tmp * (idx + 1) && darkness < 256)
							sum += (255 - pRotateField.at(idx)->imageData[j * pRotateField.at(idx)->widthStep + i * pRotateField.at(idx)->nChannels + k]);
					sum /= level;
					pFlowImage->imageData[j * pFlowImage->widthStep + i * pFlowImage->nChannels + k] = (sum > 255) ? (unsigned char)0 : (unsigned char)(255 - sum);
				}
			}
		}
	}

	printf("FIN\n");
	if (pFlowImage != NULL) {
		USES_CONVERSION;
		cvShowImage("TEST", pFlowImage);
		waitKey(0);
		cvReleaseImage(&pFlowImage);
		//cvReleaseImage(&pRotateField[0]);
		//cvReleaseImage(&pRotateField[1]);
		//cvReleaseImage(&pRotateField[2]);
		pRotateField.clear();
	}

	return 0;
}
