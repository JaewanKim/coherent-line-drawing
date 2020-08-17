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
//
//void recursive(Mat& ResultImg, int h, int leftPos, int rightPos, int jitteredValue) {
//    //printf("rec %d %d %d %f\n", h, leftPos, rightPos, jitteredValue);
//
//    if (jitteredValue < 0)
//        jitteredValue = 0;
//
//    if (h == 54 && leftPos == 288 && rightPos == 296)
//        h = 54;
//
//    int leftValue = ResultImg.at<unsigned char>(h, leftPos);
//    int rightValue = ResultImg.at<unsigned char>(h, rightPos);
//
//    int newjitteredValue = getJitteredAlpha(jitteredValue);
//
//    int centerValue = (int(leftValue + rightValue) / 2) + newjitteredValue;
//    if (centerValue < 0)        centerValue = 0;
//    else if (centerValue > 255) centerValue = 255;
//
//    ResultImg.at<unsigned char>(h, (rightPos + leftPos) / 2) = (unsigned char)centerValue;
//
//    //float newjitteredValue = jitteredValue - 1.f;
//    
//    //if (leftPos == rightPos)	return;// (leftValue + rightValue + jitteredValue) / 2;
//    //else {
//    //    recursive(ResultImg, h, leftPos, (rightPos - leftPos) / 2, jitteredValue);
//    //    recursive(ResultImg, h, (rightPos - leftPos) / 2, rightPos, jitteredValue);
//    //}
//
//    if (leftPos == rightPos - 2) return;
//    else {
//        recursive(ResultImg, h, leftPos, (rightPos + leftPos) / 2, --jitteredValue);
//        recursive(ResultImg, h, (rightPos + leftPos) / 2, rightPos, --jitteredValue);
//    }
//    
//}
//
//// Test4
///*
//int main() {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//
//    //int jittered = 32;
//    //int standard = 128;
//    //int offset = 32;
//
//    for (int standard = 0; standard < 256; standard += 8) {
//        printf("standard = %d \n", standard);
//        for (int offset = 0; offset <= 128; offset += 8) {
//            for (int jittered = 0; jittered <= 128; jittered += 8) {
//                Mat ResultImg = Mat::zeros(Size(width, height), CV_8U);
//
//                int sideMin = standard - offset;
//                int sideMax = standard + offset;
//                for (int h = 0; h < height; h++) {
//                    ResultImg.at<unsigned char>(h, 0) = randomRange(sideMin, sideMax);
//                    ResultImg.at<unsigned char>(h, width - 1) = randomRange(sideMin, sideMax);
//                    //for (int w = 0; w < width; w++) {
//                    //    if (w == 0 || w == (width - 1))
//                    //        ResultImg.at<unsigned char>(h, w) = randomRange(sideMin, sideMax);
//                    //    else
//                    //        ResultImg.at<unsigned char>(h, w) = 0;
//                    //}
//                }
//
//                //printf("recur\n");
//                for (int h = 0; h < height; h++) {
//                    //printf("h = %d \n", h);
//                    //unsigned char value = recursive(ResultImg, h, 0, width, jittered);
//                    recursive(ResultImg, h, 0, width - 1, jittered);
//                }
//
//                //printf("image\n");
//                string saveFilePath = "D:\\lab\\ETF\\Output\\200503\\paper\\";
//                char charStandard[3], charOffset[3], charSideMin[3], charSideMax[3], charJittered[3];
//
//                sprintf(charStandard, "%d", standard);
//                sprintf(charOffset, "%d", offset);
//                sprintf(charSideMin, "%d", sideMin);
//                sprintf(charSideMax, "%d", sideMax);
//                sprintf(charJittered, "%d", jittered);
//
//                string stringStandard(charStandard);
//                string stringOffset(charOffset);
//                string stringSideMin(charSideMin);
//                string stringSideMax(charSideMax);
//                string stringJittered(charJittered);
//
//                string saveFileExt = ".jpg";
//                string saveFileName = "standard" + stringStandard + "_offset" + stringOffset + "_min" + stringSideMin + "_max" + stringSideMax + "_jittered" + stringJittered;
//                string fileName = saveFilePath + saveFileName + saveFileExt;
//                //imshow(fileName, ResultImg);
//                imwrite(fileName, ResultImg);
//                waitKey(0);
//            }
//        }
//    }
//    printf("END\n");
//    return 0;
//}
// */
//
//// Test3
///*
//int main(int argc, char* argv[]) {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//    unsigned char value;
//
//    int mx = stoi(argv[1]);
//    int mn = stoi(argv[2]);
//    int w = stoi(argv[3]);
//    //float s = stof(argv[3]);
//    int a = stoi(argv[4]);
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
//    const float sigma = 1.f;
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
//            if ((w > 255 + length / 3) || (w < 255 - length / 3))   value = 255;
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
//    string saveFilePath = "D:\\lab\\ETF\\Output\\200503\\test3_7\\";
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
//    //imshow(fileName, ResultImg);
//    imwrite(fileName, ResultImg);
//    waitKey(0);  
//
//    return 0;
//}
//*/
//
//
//// Test2
//
///*
//int main(int argc, char* argv[]) {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//    unsigned char value;
//
//    int a = stoi(argv[1]);
//    int l = stoi(argv[2]);
//    float s = stof(argv[2]);
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8U);
//
//    int seedNum = height;
//    vector<unsigned char> seedField(seedNum, 255);
//    unsigned char randNum;
//
//    const int length = l;
//    const int channel = ResultImg.channels();
//    const int widthstep = ResultImg.step1();
//    const float stepLength = 1.f;
//    const float sigma = s;
//    dwLIC2 lic(width, height, channel, widthstep, length, stepLength, sigma);
//
//    printf("generate random with mean\n");
//    const unsigned char average = (unsigned char)a;
//    seedField.push_back(average);
//    if (average < 127) {
//        //for (int idx = 0; idx < seedNum / 2; idx++) {
//        for (int idx = 0; idx < seedNum * length / 3; idx++) {
//            randNum = randomRange(0, average);
//            seedField.push_back(randNum);
//            //seedField.push_back(average - randNum);   // 1, 2
//            seedField.push_back(2 * average - randNum); // 3        // 3이 맞다고 생각!
//        }
//    }
//    else {
//        //for (int idx = 0; idx < seedNum / 2; idx++) {
//        for (int idx = 0; idx < seedNum * length / 3; idx++) {
//            randNum = randomRange(average, 255);
//            seedField.push_back(randNum);
//            seedField.push_back(2 * average - randNum);
//        }
//    }
//
//    auto rng = default_random_engine{};
//    shuffle(begin(seedField), end(seedField), rng);
//
//
//    printf("set flow & noise\n");
//    Vec2f flow = Vec2f(1.f, 0.f);
//    for (int h = 0; h < height; h++) {
//        for (int w = 0; w < width; w++) {
//            lic.setFlowField(w, h, flow[0], flow[1]);
//
//            //if (w != 255)   value = 255;
//            if ((w > 255 + length / 3) || (w < 255 - length / 3))   value = 255;
//            else {
//                value = seedField.back();
//                seedField.pop_back();
//            }
//            lic.setNoiseField(w, h, value);
//        }
//    }
//
//    printf("lic\n");
//    int weightFlag = 2;
//    lic.doLICForward(weightFlag);
//
//    for (int h = 0; h < height; h++)
//        for (int w = 0; w < width; w++)
//            for (int k = 0; k < channel; k++)
//                ResultImg.at<unsigned char>(h, w) = lic.m_pLICField[h * ResultImg.step1() + w * channel + k];
//
//    printf("imshow\n");
//
//    string saveFilePath = "D:\\lab\\ETF\\Output\\200503\\test2_2\\";
//    char charAverage[3], charLength[3], charWeight[3], charSigma[3];
//    sprintf(charAverage, "%d", average);
//    sprintf(charLength, "%d", length);
//    sprintf(charWeight, "%d", weightFlag);
//    sprintf(charSigma, "%.1f", sigma);
//    string stringAverage(charAverage);
//    string stringLength(charLength);
//    string stringWeight(charWeight);
//    string stringSigma(charSigma);
//    string saveFileExt = ".jpg";
//    string saveFileName = "average" + stringAverage + "_length" + stringLength + "_weightFlag" + stringWeight +"_sigma" + stringSigma;
//    string fileName = saveFilePath + saveFileName + saveFileExt;
//    //imshow(fileName, ResultImg);
//    imwrite(fileName, ResultImg);
//    waitKey(0);
//
//    return 0;
//}
//*/
//
//// Test1
///*
//int main(int argc, char *argv[]) {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//    unsigned char value;
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8U);
//
//    int l = stoi(argv[1]);
//    float w = stof(argv[2]);
//
//    for (int length = l; length <= l; length += 10) {
//        for (int weightFlag = w; weightFlag <= w; weightFlag++) {
//            printf("length = %d \n", length);
//            //int length = 100;
//
//            const float stepLength = 1.f;
//            const int channel = ResultImg.channels();
//            const int widthstep = ResultImg.step1();
//            dwLIC2 lic(width, height, channel, widthstep, length, stepLength);
//
//            //printf("set flow & noise\n");
//            Vec2f flow = Vec2f(1.f, 0.f);
//            for (int h = 0; h < height; h++) {
//                for (int w = 0; w < width; w++) {
//                    lic.setFlowField(w, h, flow[0], flow[1]);
//
//                    //if (w != 255)   value = 255;
//                    if ((w > 255 + length / 2) || (w < 255 - length / 2))   value = 255;
//                    else            value = (range(rnd) > 127) ? (unsigned char)0 : (unsigned char)255;
//
//                    lic.setNoiseField(w, h, value);
//                }
//            }
//
//            //printf("lic\n");
//            printf("lic\n");
//            lic.doLICForward(weightFlag);
//
//            printf("img\n");
//            for (int h = 0; h < height; h++)
//                for (int w = 0; w < width; w++)
//                    for (int k = 0; k < channel; k++)
//                        ResultImg.at<unsigned char>(h, w) = lic.m_pLICField[h * ResultImg.step1() + w * channel + k];
//
//            printf("imshow\n");
//
//            string saveFilePath = "D:\\lab\\ETF\\Output\\200503\\test1_3\\";
//            char charLength[3], charWeight[3];
//            sprintf(charLength, "%d", length);
//            sprintf(charWeight, "%d", weightFlag);
//            string stringLength(charLength);
//            string stringWeight(charWeight);
//            string saveFileExt = ".jpg";
//            string saveFileName = "length" + stringLength + "_weightFlag" + stringWeight;
//            string fileName = saveFilePath + saveFileName + saveFileExt;
//            //imshow(fileName, ResultImg);
//            imwrite(fileName, ResultImg);
//            //waitKey(0);
//        }
//
//
//        //int l = stoi(argv[1]);
//        //float s = stof(argv[2]);
//
//        //for (float sigma = s; sigma <= s; sigma += 1.f) {
//        ////for (float sigma = 11.f; sigma <= 20.f; sigma += 1.f) {
//        //    printf("sigma = %f\n", sigma);
//        //    const float stepLength = 1.f;
//        //    const int channel = ResultImg.channels();
//        //    const int widthstep = ResultImg.step1();
//        //    dwLIC2 lic(width, height, channel, widthstep, length, stepLength, sigma);
//
//        //    printf("set flow & noise\n");
//        //    Vec2f flow = Vec2f(1.f, 0.f);
//        //    for (int h = 0; h < height; h++) {
//        //        for (int w = 0; w < width; w++) {
//        //            lic.setFlowField(w, h, flow[0], flow[1]);
//
//        //            //if (w != 255)   value = 255;
//        //            if ((w > 255 + length / 3) || (w < 255 - length / 3))   value = 255;
//        //            else            value = (range(rnd) > 127) ? (unsigned char)0 : (unsigned char)255;
//
//        //            lic.setNoiseField(w, h, value);
//        //        }
//        //    }
//
//        //    printf("lic\n");
//        //    int weightFlag = 2;
//        //    lic.doLICForward(weightFlag);
//
//        //    for (int h = 0; h < height; h++)
//        //        for (int w = 0; w < width; w++)
//        //            for (int k = 0; k < channel; k++)
//        //                ResultImg.at<unsigned char>(h, w) = lic.m_pLICField[h * ResultImg.step1() + w * channel + k];
//
//        //    printf("imshow\n");
//
//        //    string saveFilePath = "D:\\lab\\ETF\\Output\\200503\\test1_3\\";
//        //    char charLength[3], charWeight[3], charSigma[3];
//        //    sprintf(charLength, "%d", length);
//        //    sprintf(charWeight, "%d", weightFlag);
//        //    sprintf(charSigma, "%.1f", sigma);
//        //    string stringLength(charLength);
//        //    string stringWeight(charWeight);
//        //    string stringSigma(charSigma);
//        //    string saveFileExt = ".jpg";
//        //    string saveFileName = "length" + stringLength + "_weightFlag2" + "_sigma" + stringSigma;    // + stringWeight + 
//        //    string fileName = saveFilePath + saveFileName + saveFileExt;
//        //    //imshow(fileName, ResultImg);
//        //    imwrite(fileName, ResultImg);
//        //    waitKey(0);
//        //}
//    }
//
//    return 0;
//}
// */