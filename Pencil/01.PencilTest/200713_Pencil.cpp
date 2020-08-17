//// PencilPaperTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
////
//
//#define _CRT_SECURE_NO_WARNINGS
//
//#include "dwLIC2.h"
//#include "PencilPaperTest.h"
//#include <random>
//#include <vector>
//#include <algorithm>
//
//using namespace cv;
//using namespace std;
//
//random_device rn;
//mt19937_64 rnd(rn());
//uniform_int_distribution<int> range(0, 255);
//
//
//int randomRange(int n1, int n2) {
//    return (int)(rand() % (n2 - n1 + 1)) + n1;
//}
//
//int getJitteredAlpha(const int range) {
//    return randomRange(-range, range);
//}
//int checkRange(int value) {
//    if (value < 0)        value = 0;
//    else if (value > 255) value = 255;
//
//    return value;
//}
//
//// Test3
//int main(/*int argc, char* argv[]*/) {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//    unsigned char value;
//
//    int mx = 255;//stoi(argv[1]);
//    int mn = 0;//stoi(argv[2]);
//    int w = 2;//stoi(argv[3]);
//    float s = 100.f;//stof(argv[3]);
//    int a = 0;//stoi(argv[4]);
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8U);
//
//    int seedNum = height;
//    vector<unsigned char> seedField(seedNum, 255);
//    unsigned char randNum;
//    
//    const int length = 200;
//    const float stepLength = 1.f;
//    const int channel = ResultImg.channels();
//    const int widthstep = ResultImg.step1();
//    const float sigma = s;
//    dwLIC2 lic(width, height, channel, widthstep, length, stepLength, sigma);
//
//    printf("min, max, d\n");
//
//    if (mx <= mn)
//        return 0;
//
//    unsigned char max = mx;
//    unsigned char min = mn;
//
//    long double d = ((long double)(max - min) / seedNum);
//    printf("d = %d \n", (int)d);
//    int alpha = a;
//    
//    printf("set flow & noise\n");
//    Vec2f flow = Vec2f(1.f, 0.f);
//    for (int h = 0; h < height; h++) {
//        for (int w = 0; w < width; w++) {
//            lic.setFlowField(w, h, flow[0], flow[1]);
//
//            //if (w != 255)   value = 255;
//            if ((h % 6 == 0) || (h % 6 == 1) || (h % 6 == 2) || (h % 6 == 3) || (h % 6 == 4))
//                if (((w > 255 + length / 2) || (w < 255 - length / 2)))   value = 255;
//            //if (((w > 255 + length / 3) || (w < 255 - length / 3)) && ((h % 4 == 2) || (h % 4 == 3)))   value = 255;
//            else {
//                long double tmp = min + d * h + getJitteredAlpha(alpha);
//                if (tmp < 0)        tmp = 0;
//                else if (tmp > 255) tmp = 255;
//                value = (unsigned char) tmp;
//                //printf("%d ", value);
//            }
//            lic.setNoiseField(w, h, value);
//        }
//    }
//
//    printf("lic\n");
//    int weightFlag = w;
//    lic.doLICForward(weightFlag);
//
//    for (int h = 0; h < height; h++)
//        for (int w = 0; w < width; w++)
//            for (int k = 0; k < channel; k++)
//                ResultImg.at<unsigned char>(h, w) = lic.m_pLICField[h * ResultImg.step1() + w * channel + k];
//
//    printf("imshow\n");
//
//    string saveFilePath = "D:\\lab\\ETF\\Output\\200713\\2\\";
//    char charMin[3], charMax[3], charAlpha[3], charLength[3], charWeight[3], charSigma[3];
//    sprintf(charMin, "%d", min);
//    sprintf(charMax, "%d", max);
//    sprintf(charAlpha, "%d", alpha);
//    sprintf(charLength, "%d", length);
//    sprintf(charWeight, "%d", weightFlag);
//    sprintf(charSigma, "%.1f", sigma);
//    string stringMin(charMin);
//    string stringMax(charMax);
//    string stringAlpha(charAlpha);
//    string stringLength(charLength);
//    string stringWeight(charWeight);
//    string stringSigma(charSigma);
//    string saveFileExt = ".jpg";
//    string saveFileName = "min" + stringMin + "_max" + stringMax + "_alpha" + stringAlpha + "_length" + stringLength + "_weightFlag" + stringWeight + "_sigma" + stringSigma;
//    string fileName = saveFilePath + saveFileName + saveFileExt;
//    imshow(fileName, ResultImg);
//    imwrite(fileName, ResultImg);
//    waitKey(0);  
//
//    return 0;
//}
//
