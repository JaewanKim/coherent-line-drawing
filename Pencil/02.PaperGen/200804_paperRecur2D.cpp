//// PencilPaperTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
//#define _CRT_SECURE_NO_WARNINGS
//
//#include "dwLIC2.h"
//#include "PencilPaperTest.h"
//#include <random>
//#include <vector>
//#include <algorithm>
//#include <iostream>
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
//void paperRecur2D(Mat& ResultImg, Mat& FlagField, int leftX, int rightX, int topY, int bottomY, int jitteredValue) {
//
//    if (jitteredValue < 0)
//        jitteredValue = 0;
//
//    //jitteredValue = 0;
//
//    int leftTopValue = ResultImg.at<unsigned char>(topY, leftX);
//    int rightTopValue = ResultImg.at<unsigned char>(topY, rightX);
//    int leftBottomValue = ResultImg.at<unsigned char>(bottomY, leftX);
//    int rightBottomValue = ResultImg.at<unsigned char>(bottomY, rightX);
//
//    //int newjitteredValue = getJitteredAlpha(jitteredValue);
//
//    int leftValue = (int((leftTopValue + leftBottomValue) / 2)) + getJitteredAlpha(jitteredValue);
//    int rightValue = (int((rightTopValue + rightBottomValue) / 2)) + getJitteredAlpha(jitteredValue);
//    int centerTopValue = (int((leftTopValue + rightTopValue) / 2)) + getJitteredAlpha(jitteredValue);
//    int centerBottomValue = (int((leftBottomValue + rightBottomValue) / 2)) + getJitteredAlpha(jitteredValue);
//    int centerValue = (int((leftValue + rightValue + centerTopValue + centerBottomValue) / 4)) + getJitteredAlpha(jitteredValue);
//    leftValue = checkRange(leftValue);
//    rightValue = checkRange(rightValue);
//    centerTopValue = checkRange(centerTopValue);
//    centerBottomValue = checkRange(centerBottomValue);
//    centerValue = checkRange(centerValue);
//
//    // 여기서 flag 확인
//    if (FlagField.at<unsigned char>((topY + bottomY) / 2, leftX) == 0) {
//        ResultImg.at<unsigned char>((topY + bottomY) / 2, leftX) = (unsigned char)leftValue;
//        FlagField.at<unsigned char>((topY + bottomY) / 2, leftX) = 1;
//    }
//
//    if (FlagField.at<unsigned char>((topY + bottomY) / 2, rightX) == 0) {
//        ResultImg.at<unsigned char>((topY + bottomY) / 2, rightX) = (unsigned char)rightValue;
//        FlagField.at<unsigned char>((topY + bottomY) / 2, rightX) = 1;
//    }
//
//    if (FlagField.at<unsigned char>(topY, (leftX + rightX) / 2) == 0) {
//        ResultImg.at<unsigned char>(topY, (leftX + rightX) / 2) = (unsigned char)centerTopValue;
//        FlagField.at<unsigned char>(topY, (leftX + rightX) / 2) = 1;
//    }
//
//    if (FlagField.at<unsigned char>(bottomY, (leftX + rightX) / 2) == 0) {
//        ResultImg.at<unsigned char>(bottomY, (leftX + rightX) / 2) = (unsigned char)centerBottomValue;
//        FlagField.at<unsigned char>(bottomY, (leftX + rightX) / 2) = 1;
//    }
//
//    if (FlagField.at<unsigned char>((topY + bottomY) / 2, (leftX + rightX) / 2) == 0) {
//        ResultImg.at<unsigned char>((topY + bottomY) / 2, (leftX + rightX) / 2) = (unsigned char)centerValue;
//        FlagField.at<unsigned char>((topY + bottomY) / 2, (leftX + rightX) / 2) = 1;
//    }
//
//    //ResultImg.at<unsigned char>((topY + bottomY) / 2, leftX) = (unsigned char)leftValue;
//    //ResultImg.at<unsigned char>((topY + bottomY) / 2, rightX) = (unsigned char)rightValue;
//    //ResultImg.at<unsigned char>(topY, (leftX + rightX) / 2) = (unsigned char)centerTopValue;
//    //ResultImg.at<unsigned char>(bottomY, (leftX + rightX) / 2) = (unsigned char)centerBottomValue;
//    //ResultImg.at<unsigned char>((topY + bottomY) / 2, (leftX + rightX) / 2) = (unsigned char)centerValue;
//
//    if (leftX == rightX - 2) return;
//    else {
//        paperRecur2D(ResultImg, FlagField, leftX, (leftX + rightX) / 2, topY, (topY + bottomY) / 2, jitteredValue - 1);     // jitteredValue - 1  //1
//        paperRecur2D(ResultImg, FlagField, leftX, (leftX + rightX) / 2, (topY + bottomY) / 2, bottomY, jitteredValue - 1);    // jitteredValue--
//        paperRecur2D(ResultImg, FlagField, (leftX + rightX) / 2, rightX, topY, (topY + bottomY) / 2, jitteredValue - 1);
//        paperRecur2D(ResultImg, FlagField, (leftX + rightX) / 2, rightX, (topY + bottomY) / 2, bottomY, jitteredValue - 1);
//    }
//}
//
//int main() {
//
//    int n = 10;     // 9 + 7 (2^7 = 128)
//
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//
//    for (int standard = 128; standard <= 128; standard += 8) {
//        printf("standard = %d \n", standard);
//        for (int offset = 32; offset <= 32; offset += 8) {
//            for (int jittered = 16; jittered <= 16; jittered += 8) {
//                Mat ResultImg = Mat::zeros(Size(width, height), CV_8U);
//                Mat FlagField = Mat::zeros(Size(width, height), CV_8U);
//
//                int sideMin = standard - offset;
//                int sideMax = standard + offset;
//                sideMin = checkRange(sideMin);
//                sideMax = checkRange(sideMax);
//
//                ResultImg.at<unsigned char>(0, 0) = randomRange(sideMin, sideMax);
//                ResultImg.at<unsigned char>(0, width - 1) = randomRange(sideMin, sideMax);
//                ResultImg.at<unsigned char>(height - 1, 0) = randomRange(sideMin, sideMax);
//                ResultImg.at<unsigned char>(height - 1, width - 1) = randomRange(sideMin, sideMax);
//
//                FlagField.at<unsigned char>(0, 0) = 1;
//                FlagField.at<unsigned char>(0, width - 1) = 1;
//                FlagField.at<unsigned char>(height - 1, 0) = 1;
//                FlagField.at<unsigned char>(height - 1, width - 1) = 1;
//
//                printf("recur\n");
//                paperRecur2D(ResultImg, FlagField, 0, width - 1, 0, height - 1, jittered);
//
//                printf("image\n");
//                string saveFilePath = "D:\\lab\\ETF\\Output\\200803\\preGenPaper\\2\\";
//                char charN[3], charStandard[3], charOffset[3], charSideMin[3], charSideMax[3], charJittered[3];
//
//                sprintf(charN, "%d", n);
//                sprintf(charStandard, "%d", standard);
//                sprintf(charOffset, "%d", offset);
//                sprintf(charSideMin, "%d", sideMin);
//                sprintf(charSideMax, "%d", sideMax);
//                sprintf(charJittered, "%d", jittered);
//
//                string stringN(charN);
//                string stringStandard(charStandard);
//                string stringOffset(charOffset);
//                string stringSideMin(charSideMin);
//                string stringSideMax(charSideMax);
//                string stringJittered(charJittered);
//
//                string saveFileExt = ".jpg";
//                string saveFileName = "n" + stringN + "_standard" + stringStandard + "_offset" + stringOffset + "_min" + stringSideMin + "_max" + stringSideMax + "_jittered" + stringJittered;
//                string fileName = saveFilePath + saveFileName + saveFileExt;
//                //imshow(fileName, ResultImg);
//                imwrite(fileName, ResultImg);
//                cout << fileName << endl;
//                waitKey(0);
//            }
//        }
//    }
//    printf("END\n");
//
//    return 0;
//}