//
//#define _CRT_SECURE_NO_WARNINGS
//
//#include "PencilPaperTest.h"
//#include <random>
//#include <vector>
//#include <algorithm>
//#include "dwLIC2.h"
//#include "paper.h"
//#include <opencv.hpp>
//
//using namespace cv;
//using namespace std;
//
//random_device rn;
//mt19937_64 rnd(rn());
//uniform_int_distribution<int> range(0, 255);
//
//float clamp(float x) {
//    if (x < 0) return 0.f;
//    if (x > 1.f) return 1.f;
//    else return x;
//}
//
//float peak(float pig) {
//    return clamp(2.0f * pig);
//}
//
//float valley(float pig) {
//    return clamp(-1.f * pig + 1.f);
//}
//
//int randomRange(int n1, int n2) {
//    return (int)(rand() % (n2 - n1 + 1)) + n1;
//}
//
//int getJitteredAlpha(const int range) {
//    return randomRange(-range, range);
//}
//
//int main() {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//    //for (int pencilM = 1; pencilM <= 5; pencilM += 1) {
//    for (int alpha = 128; alpha <= 128; alpha+=32) {
//
//        int paperN = 1;
//
//        printf("Pencil \n");
//        string PencilFilePath = "D:\\lab\\ETF\\Output\\200722\\Pencil\\";
//        char charA[5];
//        sprintf(charA, "%d", alpha);
//        string stringA(charA);
//        string PencilFileName = "min0_max248_alpha" + stringA + "_length200_weightFlag2_sigma100.0";
//        string PencilFileExt = ".jpg";
//        string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
//        Mat PencilField = imread(PencilFile, CV_8UC1);
//        if (PencilField.empty()) {
//            cout << "no pencil" << endl;
//            continue;
//        }
//        const int width = PencilField.cols;
//        const int height = PencilField.rows;
//        Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//        printf("Paper \n");
//        string PaperFilePath = "D:\\lab\\ETF\\Output\\200722\\Paper\\";
//        char charM[5];
//        sprintf(charM, "%d", paperN);
//        string stringM(charM);
//        string PaperFileName = stringM;
//        string PaperFileExt = ".jpg";
//        string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
//        Mat PaperField = imread(PaperFile, CV_8UC1);
//        cout << PaperFile << endl;
//        if (PaperField.empty()) {
//            cout << "no paper" << endl;
//            continue;
//        }
//
//        //resize(PencilField, PencilField, Size(width, height));
//
//        for (int j = 0; j < height; j++) {
//            for (int i = 0; i < width; i++) {
//                //float rand = (float)(getJitteredAlpha(1) / 2);   // (-0.5f, 0.5f)
//                float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
//                float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
//                //float weight = (j % 2 + 0.5f + rand) * altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                //float weight = altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                //float tmp = ((atan(50.f * (altitude - 0.5f)) / (M_PI_2)) + 1) * 0.5f;
//                //float tmp = ((atan((altitude - 0.5f)) / (M_PI_2)) + 1) - 0.5f;
//                //float tmp = ((atan(5*(altitude - 0.5f)) / (M_PI_2)) + 1) - 0.5f;
//                //float tmp = ((atan(5*(altitude - 0.5f)) / (M_PI_2)) + 1) - 0.3f;
//                //float tmp = altitude * altitude;
//                //float tmp = sqrt(altitude);
//                //float tmp = -altitude * altitude + 1;
//
//                float weight = altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                //float weight = tmp * peak(pigment) + (1.f - tmp) * valley(pigment);
//                int value = (255 * weight) + 0.5f;
//                if (value > 255)    value = 255;
//                else if (value < 0) value = 0;
//                ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//            }
//        }
//
//        printf("Save \n");
//        string saveFilePath = "D:\\lab\\ETF\\Output\\200722\\";
//        string saveFileName = "pencil_alpha" + stringA + "__paper" + stringM;
//        //string saveFileName = "pencil" + stringA + "_paper" + stringM;
//        string saveFileExt = ".jpg";
//        string fileName = saveFilePath + saveFileName + saveFileExt;
//        if (ResultImg.empty()) {
//            cout << "read fail" << endl;
//            exit(1);
//        }
//
//        //string savePencilPath = "D:\\lab\\ETF\\Output\\200714\\2.5\\Pencil\\";
//        //string savePencilFile = savePencilPath + PencilFileName + saveFileExt;
//        //string savePaperPath = "D:\\lab\\ETF\\Output\\200713\\2.5\\Paper(2)\\";
//        //string savePaperFile = savePaperPath + PaperFileName + saveFileExt;
//
//        cout << fileName << endl;
//        //imwrite(savePencilFile, PencilField);
//        //imwrite(savePaperFile, PaperField);
//        imwrite(fileName, ResultImg);
//        waitKey(0);
//
//        printf("Saved \n");
//    }
//
//
//    return 0;
//}
//
//
///*
//#define _CRT_SECURE_NO_WARNINGS
//
//#include "PencilPaperTest.h"
//#include <random>
//#include <vector>
//#include <algorithm>
//#include "dwLIC2.h"
//#include "paper.h"
//#include <opencv.hpp>
//
//using namespace cv;
//using namespace std;
//
//random_device rn;
//mt19937_64 rnd(rn());
//uniform_int_distribution<int> range(0, 255);
//
//float clamp(float x) {
//	if (x < 0) return 0.f;
//	if (x > 1.f) return 1.f;
//	else return x;
//}
//
//float peak(float pig) {
//	return clamp(1.0f * pig);
//}
//
//float valley(float pig) {
//	return clamp(2.f * pig - 1.f);
//}
//
//int randomRange(int n1, int n2) {
//    return (int)(rand() % (n2 - n1 + 1)) + n1;
//}
//
//int getJitteredAlpha(const int range) {
//    return randomRange(-range, range);
//}
//
//int main() {
//
//    int n = 9;
//    //const int width = pow(2, n) + 1;
//    //const int height = pow(2, n) + 1;
//
//    //Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//    for (int pencilM = 1; pencilM <= 5; pencilM +=1) {
//    //for (int alpha = 0; alpha <= 128; alpha+=32) {
//
//        int paperN = 1;
//
//        printf("Pencil \n");
//        string PencilFilePath = "D:\\lab\\ETF\\Output\\200722\\Pencil(1~9)\\";
//        char charA[5];
//        sprintf(charA, "%d", pencilM);
//        string stringA(charA);
//        string PencilFileName = stringA;
//        //string PencilFileName = "min0_max248_alpha" + stringA + "_length200_weightFlag2_sigma100.0";
//        string PencilFileExt = ".jpg";
//        string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
//        Mat PencilField = imread(PencilFile, CV_8UC1);
//        if (PencilField.empty()) {
//            cout << "no pencil" << endl;
//            continue;
//        }
//        const int width = PencilField.cols;
//        const int height = PencilField.rows;
//        Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//        printf("Paper \n");
//        string PaperFilePath = "D:\\lab\\ETF\\Output\\200722\\BigPaper(1~9)\\";
//        char charM[5];
//        sprintf(charM, "%d", paperN);
//        string stringM(charM);
//        string PaperFileName = stringM;
//        string PaperFileExt = ".jpg";
//        string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
//        Mat PaperField = imread(PaperFile, CV_8UC1);
//        cout << PaperFile << endl;
//        if (PaperField.empty()) {
//            cout << "no paper" << endl;
//            continue;
//        }
//
//        //resize(PencilField, PencilField, Size(width, height));
//
//        for (int j = 0; j < height; j++) {
//            for (int i = 0; i < width; i++) {
//                //float rand = (float)(getJitteredAlpha(1) / 2);   // (-0.5f, 0.5f)
//                float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
//                float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
//                //float weight = (j % 2 + 0.5f + rand) * altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                //float weight = altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                //float tmp = ((atan(50.f * (altitude - 0.5f)) / (M_PI_2)) + 1) * 0.5f;
//                //float tmp = ((atan((altitude - 0.5f)) / (M_PI_2)) + 1) - 0.5f;
//                //float tmp = ((atan(5*(altitude - 0.5f)) / (M_PI_2)) + 1) - 0.5f;
//                //float tmp = ((atan(5*(altitude - 0.5f)) / (M_PI_2)) + 1) - 0.3f;
//                float tmp = altitude * altitude;
//                //float tmp = sqrt(altitude);
//                //float tmp = -altitude * altitude + 1;
//
//                //float weight = altitude * peak(pigment) + (1.f - altitude) * valley(pigment);
//                float weight = tmp * peak(pigment) + (1.f - tmp) * valley(pigment);
//                int value = (255 * weight) + 0.5f;
//                if (value > 255)    value = 255;
//                else if (value < 0) value = 0;
//                ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//            }
//        }
//
//        printf("Save \n");
//        string saveFilePath = "D:\\lab\\ETF\\Output\\200722\\";
//        //string saveFileName = "pencil_alpha" + stringA + "__paper" + stringM;
//        string saveFileName = "pencil" + stringA + "_paper" + stringM;
//        string saveFileExt = ".jpg";
//        string fileName = saveFilePath + saveFileName + saveFileExt;
//        if (ResultImg.empty()) {
//            cout << "read fail" << endl;
//            exit(1);
//        }
//
//        //string savePencilPath = "D:\\lab\\ETF\\Output\\200714\\2.5\\Pencil\\";
//        //string savePencilFile = savePencilPath + PencilFileName + saveFileExt;
//        //string savePaperPath = "D:\\lab\\ETF\\Output\\200713\\2.5\\Paper(2)\\";
//        //string savePaperFile = savePaperPath + PaperFileName + saveFileExt;
//
//        cout << fileName << endl;
//        //imwrite(savePencilFile, PencilField);
//        //imwrite(savePaperFile, PaperField);
//        imwrite(fileName, ResultImg);
//        waitKey(0);
//
//        printf("Saved \n");
//    }
//
//
//    return 0;
//}*/