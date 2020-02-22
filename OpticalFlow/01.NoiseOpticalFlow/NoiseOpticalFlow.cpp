
#define _CRT_SECURE_NO_WARNINGS

#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2/video/tracking.hpp"
#include <vector>
#include <stdio.h>

#include "CLD.h"
#include "ETF.h"
#include <opencv2/opencv.hpp>
#include "D:\lab\ETF\dwLIC\dwLIC2.h"
#include <ppl.h>

using namespace cv;
using namespace std;

int main() {

    int width = 638;
    int height = 269;

    int idx = 1;
    int totalFrame = 127;
    vector<Mat> videoFrame;
    vector<Mat> adjacentFlow;
    vector<Mat> noiseField;
    Mat flagField = Mat::zeros(Size(width, height), CV_8U);

    videoFrame.reserve(totalFrame);
    adjacentFlow.reserve(totalFrame);
    noiseField.reserve(totalFrame);

    while (idx <= totalFrame) {
        string filePath("D:\\lab\\ETF\\Image\\video-clip\\");
        char charIdx[3];
        string fileExt = ".bmp";
        sprintf(charIdx, "%03d", idx);
        string fileIdx(charIdx);

        string fileName = filePath + fileIdx + fileExt;

        Mat img = imread(fileName);
        cvtColor(img, img, COLOR_BGR2GRAY);
        resize(img, img, Size(width, height));

        videoFrame.push_back(img);

        adjacentFlow.push_back(Mat::zeros(img.size(), CV_32FC2));
        noiseField.push_back(Mat::zeros(img.size(), CV_8U));

        idx++;
    }
    printf("\n");


    printf("OpticalFlow \n");
    for (int idx = 1; idx < videoFrame.size(); idx++)
        calcOpticalFlowFarneback(videoFrame.at(idx), videoFrame.at(idx - 1), adjacentFlow.at(idx - 1), 0.5, 8, 64, 3, 7, 1.5, OPTFLOW_FARNEBACK_GAUSSIAN);

    printf("NoiseGen 1 \n");
    for (int h = 0; h < height; h++)
        for (int w = 0; w < width; w++)
            noiseField.at(0).at<unsigned char>(h, w) = rand() % 256;

    for (int idx = 1; idx < noiseField.size(); idx++) {
        printf("NoiseGen %d \n", idx + 1);

        Mat flagField = Mat::zeros(Size(width, height), CV_8U);

        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                float dx = adjacentFlow.at(idx - 1).at<Vec2f>(h, w)[0];
                float dy = adjacentFlow.at(idx - 1).at<Vec2f>(h, w)[1];
                int r = h + dy + 0.5f;
                int c = w + dx + 0.5f;

                if (r < 0 || r >= height || c < 0 || c >= width)
                    noiseField.at(idx).at<unsigned char>(h, w) = rand() % 256;
                else {
                    if (flagField.at<unsigned char>(r, c) == 1)
                        noiseField.at(idx).at<unsigned char>(h, w) = rand() % 256;
                    else {
                        flagField.at<unsigned char>(r, c) = 1;
                        noiseField.at(idx).at<unsigned char>(h, w) = noiseField.at(idx - 1).at<unsigned char>(r, c);
                    }
                }
            }
        }
    }

    vector<Mat> ResultImg;

    for (int idx = 0; idx < videoFrame.size(); idx++)
        ResultImg.push_back(Mat::zeros(Size(width, height), CV_8U));

    ETF etf;

    const int length = 30;
    const float lic_sigma = 1.0f;

    dwLIC2 lic2(width, height, length, lic_sigma);
    lic2.init();

    Mat flow_field = Mat::zeros(Size(width, height), CV_32FC2);
    Mat refined_field = Mat::zeros(Size(width, height), CV_32FC2);
    Mat grad_mg = Mat::zeros(Size(width, height), CV_32FC1);

    const int ksize = 10;

    printf("ResultImg: %d\n", ResultImg.size());
    printf("\n");
    for (int idx = 0; idx < videoFrame.size(); idx++) {
        printf("%d \n", idx + 1);

        Mat image = videoFrame.at(idx);
        resize(image, image, Size(638, 269));

        printf("refine ETF 1\n");
        etf.init_ETF(image, flow_field, grad_mg);
        refined_field = etf.refine_ETF(image, ksize, flow_field, grad_mg);
        for (int i = 2; i <= 4; i++) {
            printf("refine ETF %d\n", i);
            refined_field = etf.refine_ETF(image, ksize, refined_field, grad_mg);
        }

        printf("setNoiseField & setFlowField \n");
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                lic2.setNoiseField2(i, j, noiseField.at(idx).at<unsigned char>(j, i));
                lic2.setFlowField(i, j, refined_field.at<Vec2f>(j, i)[0], refined_field.at<Vec2f>(j, i)[1]);
            }
        }

        printf("LIC \n");
        lic2.doLICForward(1);

        printf("Image %d \n", idx + 1);
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                unsigned char tmp = lic2.m_pLICField[j * width + i];
                ResultImg.at(idx).at<unsigned char>(j, i) = tmp;
            }
        }

        printf("Save %d \n\n", idx + 1);
        string filePath("D:\\lab\\ETF\\Output\\200217\\test6\\");
        char charIdx[3];
        string fileExt = ".bmp";
        sprintf(charIdx, "%d", idx + 1);
        string fileIdx(charIdx);

        string fileName = filePath + fileIdx + fileExt;
        imwrite(fileName, ResultImg.at(idx));
    }

    videoFrame.clear();
    adjacentFlow.clear();
    noiseField.clear();
    ResultImg.clear();

    return 0;
}
