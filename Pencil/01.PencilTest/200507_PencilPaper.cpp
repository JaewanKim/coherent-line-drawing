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
//int main(/*int argc, char* argv[]*/) {
//
//    int n = 9;
//    const int width = pow(2, n) + 1;
//    const int height = pow(2, n) + 1;
//
//    //int min = stoi(argv[1]);
//    //int max = stoi(argv[2]);
//    //int alpha = stoi(argv[3]);
//
//    //int standard = stoi(argv[4]);
//    //int offset = stoi(argv[5]);
//    //int paperMin = stoi(argv[6]);
//    //int paperMax = stoi(argv[7]);
//    //int jittered = stoi(argv[8]);
//
//    Paper p;
//
//    Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);
//
//    for (int min = 0; min < 256; min += 32) {
//        for (int max = 0; max < 256; max += 32) {
//            for (int alpha = 160; alpha <= 160; alpha += 32) {
//                for (int standard = 0; standard < 256; standard += 32) {
//                    for (int offset = 0; offset < 256; offset += 32) {
//                        for (int paperMin = 0; paperMin < 256; paperMin += 32) {
//                            for (int paperMax = 0; paperMax < 256; paperMax += 32) {
//                                for (int jittered = 0; jittered <= 72; jittered += 8) {
//
//                                    //printf("Pencil \n");
//                                    string PencilFilePath = "D:\\lab\\ETF\\Output\\200503\\test3_6\\";
//                                    char charMin[3], charMax[3], charAlpha[3];
//                                    sprintf(charMin, "%d", min);
//                                    sprintf(charMax, "%d", max);
//                                    sprintf(charAlpha, "%d", alpha);
//                                    string stringMin(charMin);
//                                    string stringMax(charMax);
//                                    string stringAlpha(charAlpha);
//                                    string PencilFileName = "min" + stringMin + "_max" + stringMax + "_alpha" + stringAlpha + "_length200_weightFlag2_sigma100.0";
//                                    string PencilFileExt = ".jpg";
//                                    string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
//                                    Mat PencilField = imread(PencilFile, CV_8UC1);
//                                    if (PencilField.empty()) continue;
//
//                                    //printf("Paper \n");
//                                    string PaperFilePath = "D:\\lab\\ETF\\Output\\200503\\test4_paper\\";
//                                    char charStandard[3], charOffset[3], charPaperMin[3], charPaperMax[3], charJittered[3];
//                                    sprintf(charStandard, "%d", standard);
//                                    sprintf(charOffset, "%d", offset);
//                                    sprintf(charPaperMin, "%d", paperMin);
//                                    sprintf(charPaperMax, "%d", paperMax);
//                                    sprintf(charJittered, "%d", jittered);
//                                    string stringStandard(charStandard);
//                                    string stringOffset(charOffset);
//                                    string stringPaperMin(charPaperMin);
//                                    string stringPaperMax(charPaperMax);
//                                    string stringJittered(charJittered);
//                                    string PaperFileName = "standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
//                                    string PaperFileExt = ".jpg";
//                                    string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
//                                    Mat PaperField = imread(PaperFile, CV_8UC1);
//                                    if (PaperField.empty()) continue;
//
//                                    for (int j = 0; j < height; j++) {
//                                        for (int i = 0; i < width; i++) {
//                                            float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
//                                            float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
//                                            //float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
//                                            float weight = altitude * p.peak(pigment) + (1.f - altitude) * p.valley(pigment);
//                                            int value = (255 * weight) + 0.5f;
//                                            if (value > 255)    value = 255;
//                                            else if (value < 0) value = 0;
//                                            ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//                                            //result_paper.at<unsigned char>(j, i) = (unsigned char)(weight * 255);
//                                        }
//                                    }
//
//                                    //printf("Save \n");
//                                    string saveFilePath = "D:\\lab\\ETF\\Output\\200507\\length200_weightFlag2_sigma100.0\\";
//                                    string saveFileName = "min" + stringMin + "_max" + stringMax + "_alpha" + stringAlpha + "__standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
//                                    string saveFileExt = ".jpg";
//                                    string fileName = saveFilePath + saveFileName + saveFileExt;
//                                    if (ResultImg.empty()) {
//                                        cout << "read fail" << endl;
//                                        exit(1);
//                                    }
//                                    imwrite(fileName, ResultImg);
//                                    cout << fileName << endl;
//                                    waitKey(0);
//                                    //printf("Saved \n");
//
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//
//    return 0;
//}
