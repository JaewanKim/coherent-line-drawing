
#define _CRT_SECURE_NO_WARNINGS

#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2/video/tracking.hpp"
#include <vector>
#include <stdio.h>

#include "ETF.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC2.h"
#include "D:\lab\ETF\dwLIC\dwColors.h"
#include <ppl.h>

using namespace cv;
using namespace std;

int randomRange(int n1, int n2) {
    return (int)(rand() % (n2 - n1 + 1)) + n1;
}

void getJitteredHSV(const float hRange, const float sRange, const float vRange, float& jitteredH, float& jitteredS, float& jitteredV) {
    jitteredH = randomRange(-hRange, hRange);
    jitteredS = randomRange(-sRange, sRange);
    jitteredV = randomRange(-vRange, vRange);
}

int main() {

    dwColors color;

    int width = 638;
    int height = 269;

    int idx = 1;
    int totalFrame = 5;
    vector<Mat> videoFrame;
    vector<Mat> videoGrayFrame;
    vector<Mat> adjacentFlow;
    vector<Mat> jitteredImg;
    vector<Mat> hsvField;
    vector<Mat> ResultImg;

    Mat flagField = Mat::zeros(Size(width, height), CV_8U);

    videoFrame.reserve(totalFrame);
    videoGrayFrame.reserve(totalFrame);
    adjacentFlow.reserve(totalFrame);
    hsvField.reserve(totalFrame);
    jitteredImg.reserve(totalFrame);

    while (idx <= totalFrame) {
        string filePath("D:\\lab\\ETF\\Image\\video-clip\\");
        char charIdx[3];
        string fileExt = ".bmp";
        sprintf(charIdx, "%03d", idx);
        string fileIdx(charIdx);

        string fileName = filePath + fileIdx + fileExt;

        Mat img = imread(fileName);
        Mat grayImg;
        resize(img, img, Size(width, height));
        cvtColor(img, grayImg, COLOR_BGR2GRAY);

        videoFrame.push_back(img);
        videoGrayFrame.push_back(grayImg);

        adjacentFlow.push_back(Mat::zeros(Size(width, height), CV_32FC2));
        hsvField.push_back(Mat::zeros(Size(width, height), CV_32FC3));
        jitteredImg.push_back(Mat::zeros(Size(width, height), CV_8UC3));

        ResultImg.push_back(Mat::zeros(Size(width, height), CV_8UC3));

        idx++;
    }
    printf("\n");

    printf("OpticalFlow \n");
    for (int idx = 1; idx < videoGrayFrame.size(); idx++)
        calcOpticalFlowFarneback(videoGrayFrame.at(idx), videoGrayFrame.at(idx - 1), adjacentFlow.at(idx - 1), 0.5, 8, 64, 3, 7, 1.5, OPTFLOW_FARNEBACK_GAUSSIAN);

    printf("hsvFieldGen 1 \n");
    const float hRange = 36;
    const float sRange = 0.25;
    const float vRange = 36;
    float jitteredH, jitteredS, jitteredV;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            getJitteredHSV(hRange, sRange, vRange, jitteredH, jitteredS, jitteredV);
            hsvField.at(0).at<Vec3f>(j, i) = Vec3f(jitteredH, jitteredS, jitteredV);
        }
    }
    for (int idx = 1; idx < hsvField.size(); idx++) {
        printf("hsvFieldGen %d \n", idx + 1);

        Mat flagField = Mat::zeros(Size(width, height), CV_8U);

        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                float dx = adjacentFlow.at(idx - 1).at<Vec2f>(h, w)[0];
                float dy = adjacentFlow.at(idx - 1).at<Vec2f>(h, w)[1];
                int r = h + dy + 0.5f;
                int c = w + dx + 0.5f;

                if (r < 0 || r >= height || c < 0 || c >= width) {
                    getJitteredHSV(hRange, sRange, vRange, jitteredH, jitteredS, jitteredV);
                    hsvField.at(idx).at<Vec3f>(h, w) = Vec3f(jitteredH, jitteredS, jitteredV);
                }
                else {
                    if (flagField.at<unsigned char>(r, c) != 0) {
                        getJitteredHSV(hRange, sRange, vRange, jitteredH, jitteredS, jitteredV);
                        hsvField.at(idx).at<Vec3f>(h, w) = Vec3f(jitteredH, jitteredS, jitteredV);
                    }
                    else {
                        flagField.at<unsigned char>(r, c) = 1;
                        getJitteredHSV(hRange, sRange, vRange, jitteredH, jitteredS, jitteredV);
                        hsvField.at(idx).at<Vec3f>(h, w) = hsvField.at(idx - 1).at<Vec3f>(r, c);
                    }
                }
            }
        }
    }

    printf("Convert hsvFields to RGB \n");
    for (int idx = 0; idx < hsvField.size(); idx++) {
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {

                int b = videoFrame.at(idx).at<Vec3b>(j, i)[0];
                int g = videoFrame.at(idx).at<Vec3b>(j, i)[1];
                int r = videoFrame.at(idx).at<Vec3b>(j, i)[2];

                float h, s, v;
                color.RGB2HSV(r, g, b, h, s, v);
                float imgH = h + hsvField.at(idx).at<Vec3f>(j, i)[0];
                float imgS = s + hsvField.at(idx).at<Vec3f>(j, i)[1];
                float imgV = v + hsvField.at(idx).at<Vec3f>(j, i)[2];

                if (imgH < 0.f) imgH = 0.f;	else if (imgH > 360.f) imgH = 360.f;
                if (imgS < 0.f) imgS = 0.f;	else if (imgS > 1.f) imgS = 1.f;
                if (imgV < 0.f) imgV = 0.f;	else if (imgV > 255.f) imgV = 255.f;

                int imgR, imgG, imgB;
                color.HSV2RGB(imgH, imgS, imgV, imgR, imgG, imgB);

                jitteredImg.at(idx).at<Vec3b>(j, i) = Vec3b((unsigned char)imgB, (unsigned char)imgG, (unsigned char)imgR);
            }
        }
    }

    ETF etf;

    const int length = 30;
    const float lic_sigma = 1.0f;
    const int channel = 3;
    const int widthstep = videoFrame.at(0).step1();
    dwLIC2 lic2(width, height, channel, widthstep, length, lic_sigma);
    lic2.init();

    Mat flow_field = Mat::zeros(Size(width, height), CV_32FC2);
    Mat refined_field = Mat::zeros(Size(width, height), CV_32FC2);
    Mat grad_mg = Mat::zeros(Size(width, height), CV_32FC1);

    const int ksize = 10;

    for (int idx = 0; idx < jitteredImg.size(); idx++) {
        printf("%d \n", idx + 1);

        Mat image = jitteredImg.at(idx);
        cvtColor(image, image, COLOR_BGR2GRAY);

        printf("initializing refine ETF\n");
        etf.init_ETF(image, flow_field, grad_mg);
        printf("refine ETF 1\n");
        refined_field = etf.refine_ETF(image, ksize, flow_field, grad_mg);
        for (int i = 2; i <= 5; i++) {
            printf("refine ETF %d\n", i);
            refined_field = etf.refine_ETF(image, ksize, refined_field, grad_mg);
        }

        printf("setNoiseField & setFlowField \n");
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                unsigned char b = jitteredImg.at(idx).at<Vec3b>(j,i)[0];
                unsigned char g = jitteredImg.at(idx).at<Vec3b>(j,i)[1];
                unsigned char r = jitteredImg.at(idx).at<Vec3b>(j,i)[2];
                lic2.setNoiseField2(i, j, r, g, b);
                lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
            }
        }

        printf("LIC \n");
        lic2.doLICForward(2);

        printf("Image %d \n", idx + 1);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                ResultImg.at(idx).at<Vec3b>(j, i)[0] = lic2.m_pLICField[j * ResultImg.at(idx).step1() + i * ResultImg.at(idx).channels() + 0];
                ResultImg.at(idx).at<Vec3b>(j, i)[1] = lic2.m_pLICField[j * ResultImg.at(idx).step1() + i * ResultImg.at(idx).channels() + 1];
                ResultImg.at(idx).at<Vec3b>(j, i)[2] = lic2.m_pLICField[j * ResultImg.at(idx).step1() + i * ResultImg.at(idx).channels() + 2];
            }
        }


        printf("Save %d \n", idx + 1);
        string filePath("D:\\lab\\ETF\\Output\\200222\\test1\\");
        string fileExt = ".bmp";
        string fileNum = to_string(idx + 1);
        string fileName = filePath + fileNum + fileExt;
        cout << fileName << "\n";
        imwrite(fileName, ResultImg.at(idx));
        printf("Saved\n");
    }
    printf("\n");

    videoFrame.clear();
    adjacentFlow.clear();
    jitteredImg.clear();
    hsvField.clear();
    ResultImg.clear();

    return 0;
}
