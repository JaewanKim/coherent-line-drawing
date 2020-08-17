
#define _CRT_SECURE_NO_WARNINGS

#include "PencilPaperTest.h"
#include <random>
#include <vector>
#include <algorithm>
#include "dwLIC2.h"
#include "paper.h"
#include <opencv.hpp>

using namespace cv;
using namespace std;

random_device rn;
mt19937_64 rnd(rn());
uniform_int_distribution<int> range(0, 255);


int main() {

    int n = 9;
    const int width = pow(2, n) + 1;
    const int height = pow(2, n) + 1;

    Paper p;

    Mat ResultImg = Mat::zeros(Size(width, height), CV_8UC1);

    //int min = 0;
    //int max = 248;
    //int alpha = 128;
    //int standard = 128;
    //int offset = 32;
    //int paperMin = 96;
    //int paperMax = 160;
    //int jittered = 24;
    

    for (int n = 4; n < 5; n += 1) {                  // standard around 128
        for (int standard = 128; standard <= 128; standard += 16) {                  // standard around 128
            for (int offset = 0; offset <= 128; offset += 16) {                      // offset <= 128
                for (int paperMin = 0; paperMin <= 256; paperMin += 16) {
                    for (int paperMax = 0; paperMax <= 256; paperMax += 16) {
                        for (int jittered = 0; jittered <= 128; jittered += 16) {     // jittered <= 64

                            printf("Pencil \n");
                            string PencilFilePath = "D:\\lab\\ETF\\Output\\200810\\pencil\\";
                            char charN[5];
                            sprintf(charN, "%d", n);
                            string stringN(charN);
                            string PencilFileName = "pencil" + stringN;
                            string PencilFileExt = ".jpg";
                            string PencilFile = PencilFilePath + PencilFileName + PencilFileExt;
                            Mat PencilField = imread(PencilFile, CV_8UC1);
                            if (PencilField.empty()) {
                                //cout << "no pencil" << endl;
                                continue;
                            }

                            printf("Paper \n");
                            string PaperFilePath = "D:\\lab\\ETF\\Output\\200807\\2dRecursivePaper4_200711\\";
                            char charStandard[5], charOffset[5], charPaperMin[5], charPaperMax[5], charJittered[5];
                            sprintf(charStandard, "%d", standard);
                            sprintf(charOffset, "%d", offset);
                            sprintf(charPaperMin, "%d", paperMin);
                            sprintf(charPaperMax, "%d", paperMax);
                            sprintf(charJittered, "%d", jittered);
                            string stringStandard(charStandard);
                            string stringOffset(charOffset);
                            string stringPaperMin(charPaperMin);
                            string stringPaperMax(charPaperMax);
                            string stringJittered(charJittered);
                            string PaperFileName = "standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
                            string PaperFileExt = ".jpg";
                            string PaperFile = PaperFilePath + PaperFileName + PaperFileExt;
                            Mat PaperField = imread(PaperFile, CV_8UC1);
                            cout << PaperFile << endl;
                            if (PaperField.empty()) {
                                //cout << "no paper" << endl;
                                continue;
                            }

                            //resize(PencilField, PencilField, Size(width, height));

                            for (int j = 0; j < height; j++) {
                                for (int i = 0; i < width; i++) {
                                    float pigment = 1.f - (PencilField.at<unsigned char>(j, i)) / 255.f;
                                    float altitude = PaperField.at<unsigned char>(j, i) / 255.f;
                                    float weight = altitude * p.peak(pigment) + (1.f - altitude) * p.valley(pigment);
                                    int value = (255 * weight) + 0.5f;
                                    if (value > 255)    value = 255;
                                    else if (value < 0) value = 0;
                                    ResultImg.at<unsigned char>(j, i) = (unsigned char)(255 - value);
                                }
                            }

                            //printf("Save \n");
                            string saveFilePath = "D:\\lab\\ETF\\Output\\200810\\combine\\";
                            //string saveFileName = "standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
                            string saveFileName = stringN + "__standard" + stringStandard + "_offset" + stringOffset + "_min" + stringPaperMin + "_max" + stringPaperMax + "_jittered" + stringJittered;
                            string saveFileExt = ".jpg";
                            string fileName = saveFilePath + saveFileName + saveFileExt;
                            if (ResultImg.empty()) {
                                cout << "read fail" << endl;
                                exit(1);
                            }
                            string savePaperPath = "D:\\lab\\ETF\\Output\\200810\\paper\\";
                            string savePaperFile = savePaperPath + PaperFileName + saveFileExt;

                            imwrite(savePaperFile, PaperField);
                            imwrite(fileName, ResultImg);

                            cout << fileName << endl;
                            waitKey(0);
                            //printf("Saved \n");

                        }
                    }
                }
            }
        }
    }
    return 0;
}
