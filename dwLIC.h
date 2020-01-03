
/*
#pragma once

struct IplImage;
class dwLIC
{
	static const int DISCRETE_FILTER_SIZE = 2048;
	static const int SQUARE_FLOW_FIELD_SZ = 400;
	static const float LOWPASS_FILTR_LENGTH;
	static const float LINE_SQUARE_CLIP_MAX;
	static const float VECTOR_COMPONENT_MIN;


	bool bNoiseImage;
	int n_xres, n_yres;
	unsigned char* pNoise;
	int LUTsiz;
	float* p_LUT0, * p_LUT1;
	float krnlen;
	IplImage * pOutputImg;

	void MakeWhiteNoise();
	void GenBoxFiltrLUT();
	void NormalizVectrs();
public:
	dwLIC(int x, int y) { n_xres = x; n_yres = y; pVectr = new float[n_xres * n_yres * 2]; bNoiseImage = false; };
	~dwLIC(void);
	float* pVectr;
	void FlowImagingLIC();
	void NoiseFromImage(IplImage* img);
	IplImage* GetLICimage() { return pOutputImg; };
};
*/

#pragma once
#include <opencv2/opencv.hpp>
using namespace cv;

//struct IplImage;

class dwLIC
{
	static const int DISCRETE_FILTER_SIZE = 2048;
	static const int SQUARE_FLOW_FIELD_SZ = 400;
	static const float LOWPASS_FILTR_LENGTH;
	static const float LINE_SQUARE_CLIP_MAX;
	static const float VECTOR_COMPONENT_MIN;


	bool bNoiseImage;
	int n_xres, n_yres;
	unsigned char* pNoise;
	int LUTsiz;
	float* p_LUT0, * p_LUT1;
	float krnlen;
	//IplImage * pOutputImg;
	Mat* pOutputImg;

	void MakeWhiteNoise();
	void GenBoxFiltrLUT();
	void NormalizVectrs();
public:
	dwLIC(int x, int y) { n_xres = x; n_yres = y; pVectr = new float[n_xres * n_yres * 2]; bNoiseImage = false; };
	~dwLIC(void);
	float* pVectr;
	void FlowImagingLIC();
	void NoiseFromImage(Mat* img);
	Mat* GetLICimage() { return pOutputImg; };
};
