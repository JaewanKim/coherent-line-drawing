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
//    //int min = 0;
//    //int max = 248;
//    //int alpha = 128;
//    //int standard = 128;
//    //int offset = 32;
//    //int paperMin = 96;
//    //int paperMax = 160;
//    //int jittered = 24;
//
//    for (int min = 32; min < 256; min += 32) {                                                // min == 0
//        for (int max = 32; max < 256; max += 32) {                                        // max == 255
//            for (int alpha = 0; alpha <= 128; alpha += 32) {                                // alpha <= 128
//                for (int standard = 96; standard <= 160; standard += 16) {                  // standard around 128
//                    for (int offset = 0; offset <= 64; offset += 16) {                      // offset
//                        for (int paperMin = 0; paperMin < 256; paperMin += 16) {
//                            for (int paperMax = 0; paperMax < 256; paperMax += 16) {
//                                for (int jittered = 0; jittered <= 64; jittered += 8) {     // jittered <= 64
//
//                                    //printf("Pencil \n");
//                                    string PencilFilePath = "D:\\lab\\ETF\\Output\\200503\\test3_6\\";
//                                    char charMin[5], charMax[5], charAlpha[5];
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
//                                    if (PencilField.empty()) {
//                                        //cout << "no pencil" << endl;
//                                        continue;
//                                    }
//
//                                    const double angle = 45.f;   //시계반대방향으로 45도 회전
//                                    const double scale = 2.f;
//                                    Mat rotMat = getRotationMatrix2D(Point2f(PencilField.rows / 2, PencilField.cols / 2), angle, scale);
//                                    warpAffine(PencilField, PencilField, rotMat, PencilField.size());
//
//
//                                    //printf("Paper \n");
//                                    string PaperFilePath = "D:\\lab\\ETF\\Output\\200503\\test4_paper\\";
//                                    char charStandard[5], charOffset[5], charPaperMin[5], charPaperMax[5], charJittered[5];
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
//                                    if (PaperField.empty()) {
//                                        //cout << "no paper" << endl;
//                                        continue;
//                                    }
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
//                                    string saveFilePath = "D:\\lab\\ETF\\Output\\200516\\PencilPaper\\";
//                                    string saveFileName = "min" + stringMin + "_max" + stringMax + "_alpha" + stringAlpha + "__standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered + "_Rotated";
//                                    string saveFileExt = ".jpg";
//                                    string fileName = saveFilePath + saveFileName + saveFileExt;
//                                    if (ResultImg.empty()) {
//                                        cout << "read fail" << endl;
//                                        exit(1);
//                                    }
//                                    string savePencilPath = "D:\\lab\\ETF\\Output\\200516\\Pencil\\";
//                                    string savePencilFile = savePencilPath + PencilFileName + saveFileExt;
//                                    string savePaperPath = "D:\\lab\\ETF\\Output\\200516\\Paper\\";
//                                    string savePaperFile = savePaperPath + PaperFileName + saveFileExt;
//
//                                    imwrite(savePencilFile, PencilField);
//                                    imwrite(savePaperFile, PaperField);
//                                    imwrite(fileName, ResultImg);
//
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
