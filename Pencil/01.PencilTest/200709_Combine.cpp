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
//        //for (int min = 0; min <= 0; min += 32) {                                                // min == 0
//        //    for (int max = 248; max <= 248; max += 32) {                                        // max == 248
//        //        for (int alpha = 0; alpha <= 128; alpha += 32) {                                // alpha <= 128
//                for (int n = 1; n < 16; n++) {
//                    for (int standard = 128; standard <= 128; standard += 16) {                  // standard around 128
//                        for (int offset = 0; offset <= 128; offset += 32) {                      // offset
//                            for (int paperMin = 0; paperMin <= 256; paperMin += 16) {
//                                for (int paperMax = 0; paperMax <= 256; paperMax += 16) {
//                                    for (int jittered = 16; jittered <= 16; jittered += 16) {     // jittered <= 64
//
//                                        printf("Pencil \n");
//                                        string PencilFilePath = "D:\\lab\\ETF\\Output\\200713\\2\\2_Pencil_01~\\";
//                                        char charN[5];
//                                        sprintf(charN, "%d", n);
//                                        string stringN(charN);
//                                        string PencilFileName = stringN;
//                                        string PencilFileExt = ".jpg";
//                                        string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
//                                        Mat PencilField = imread(PencilFile, CV_8UC1);
//                                        if (PencilField.empty()) {
//                                            cout << "no pencil" << endl;
//                                            continue;
//                                        }
//
//                                        //const double angle = 45.f;   //시계반대방향으로 45도 회전
//                                        //const double scale = 2.f;
//                                        //Mat rotMat = getRotationMatrix2D(Point2f(PencilField.rows / 2, PencilField.cols / 2), angle, scale);
//                                        //warpAffine(PencilField, PencilField, rotMat, PencilField.size());
//
//                                        printf("Paper \n");
//                                        //string PaperFilePath = "D:\\lab\\ETF\\Output\\200702\\2dRecursivePaper(n=11-15)\\";
//                                        string PaperFilePath = "D:\\lab\\ETF\\Output\\200701\\Paper (standard 96 128 160 _ jittered 16)\\";
//                                        //string PaperFilePath = "D:\\lab\\ETF\\Output\\200713\\2\\Paper(2)\\";
//                                        char charStandard[5], charOffset[5], charPaperMin[5], charPaperMax[5], charJittered[5];
//                                        sprintf(charStandard, "%d", standard);
//                                        sprintf(charOffset, "%d", offset);
//                                        sprintf(charPaperMin, "%d", paperMin);
//                                        sprintf(charPaperMax, "%d", paperMax);
//                                        sprintf(charJittered, "%d", jittered);
//                                        string stringStandard(charStandard);
//                                        string stringOffset(charOffset);
//                                        string stringPaperMin(charPaperMin);
//                                        string stringPaperMax(charPaperMax);
//                                        string stringJittered(charJittered);
//                                        string PaperFileName = "standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
//                                        string PaperFileExt = ".jpg";
//                                        string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
//                                        Mat PaperField = imread(PaperFile, CV_8UC1);
//                                        cout << PaperFile << endl;
//                                        if (PaperField.empty()) {
//                                            cout << "no paper" << endl;
//                                            continue;
//                                        }
//
//                                        resize(PencilField, PencilField, Size(width, height));
//
//                                        for (int j = 0; j < height; j++) {
//                                            for (int i = 0; i < width; i++) {
//                                                float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
//                                                float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
//                                                //float weight = ((atan(50.f * ((NoiseField.at<unsigned char>(j, i) / 255.f) - 0.5f) + 15) / (M_PI_2)) + 1) * 0.5f;
//                                                float weight = altitude * p.peak(pigment) + (1.f - altitude) * p.valley(pigment);
//                                                int value = (255 * weight) + 0.5f;
//                                                if (value > 255)    value = 255;
//                                                else if (value < 0) value = 0;
//                                                ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
//                                                //result_paper.at<unsigned char>(j, i) = (unsigned char)(weight * 255);
//                                            }
//                                        }
//
//                                        //printf("Save \n");
//                                        string saveFilePath = "D:\\lab\\ETF\\Output\\200713\\2\\PencilPaper(2)\\";
//                                        string saveFileName = stringN + "__standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
//                                        string saveFileExt = ".jpg";
//                                        string fileName = saveFilePath + saveFileName + saveFileExt;
//                                        if (ResultImg.empty()) {
//                                            cout << "read fail" << endl; 
//                                            exit(1);
//                                        }
//                                        string savePencilPath = "D:\\lab\\ETF\\Output\\200713\\2\\Pencil\\";
//                                        string savePencilFile = savePencilPath + PencilFileName + saveFileExt;
//                                        string savePaperPath = "D:\\lab\\ETF\\Output\\200713\\2\\Paper(2)\\";
//                                        string savePaperFile = savePaperPath + PaperFileName + saveFileExt;
//
//                                        imwrite(savePencilFile, PencilField);
//                                        imwrite(savePaperFile, PaperField);
//                                        imwrite(fileName, ResultImg);
//
//                                        cout << fileName << endl;
//                                        waitKey(0);
//                                        //printf("Saved \n");
//
//                                    }
//                                }
//                            }
//                        }
//                    }
//    }
//    //            }
//    //        }
//    //    }
//    //}
//
//
//    return 0;
//}
