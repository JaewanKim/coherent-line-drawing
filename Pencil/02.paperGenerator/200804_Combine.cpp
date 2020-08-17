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
//
//int main() {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//
//    Paper p;
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//    int min = 0;
//    int max = 248;
//    int alpha = 128;
//    int standard = 128;
//    int offset = 32;
//    int papermin = 96;
//    int papermax = 160;
//    int jittered = 24;
//    
//    printf("Pencil \n");
//    string PencilFilePath = "D:\\lab\\ETF\\Output\\200804\\";
//    string PencilFileName = "pencil";
//    string PencilFileExt = ".jpg";
//    string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
//    Mat PencilField = imread(PencilFile, CV_8UC1);
//    if (PencilField.empty()) {
//        cout << "no pencil" << endl;
//        exit(1);
//    }
//
//    printf("Paper \n");
//    string PaperFilePath = "D:\\lab\\ETF\\Output\\200804\\paper\\2\\";
//    string PaperFileName = "n10_standard128_offset32_min96_max160_jittered16";
//    string PaperFileExt = ".jpg";
//    string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
//    Mat PaperField = imread(PaperFile, CV_8UC1);
//    cout << PaperFile << endl;
//    if (PaperField.empty()) {
//        cout << "no paper" << endl;
//        exit(1);
//    }
//
//    resize(PencilField, PencilField, Size(width, height));
//    for (int j = 0; j < height; j++) {
//        for (int i = 0; i < width; i++) {
//            float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
//            float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
//            //float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
//            float weight = altitude * p.peak(pigment) + (1.f - altitude) * p.valley(pigment);
//            int value = (255 * weight) + 0.5f;
//            if (value > 255)    value = 255;
//            else if (value < 0) value = 0;
//            ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//        }
//    }
//
//    //printf("Save \n");
//    string saveFilePath = "D:\\lab\\ETF\\Output\\200804\\";
//    string saveFileName = "pencil1-2dResult";
//    string saveFileExt = ".jpg";
//    string fileName = saveFilePath + saveFileName + saveFileExt;
//    if (ResultImg.empty()) {
//        cout << "read fail" << endl;
//        exit(1);
//    }
//    imwrite(fileName, ResultImg);
//
//    cout << fileName << endl;
//    waitKey(0);
//    //printf("Saved \n");
//
//    return 0;
//}
