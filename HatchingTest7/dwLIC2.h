
#pragma once
#include <opencv2\core\core_c.h>
#include <corecrt_math_defines.h>

struct dwNoise
{
	float x;
	float y;
	dwNoise(float i, float j) :x(i), y(j) {}
};

class dwLIC2
{
	static constexpr float asixth = 1.f / 6.f;

	float m_fStepLength;
	int m_nWidth;
	int m_nHeight;
	int m_nLength;
	IplImage* m_pIMG = NULL;
	float* m_pFlowField = NULL;
	float* m_pGaussianWeight = NULL;

	std::vector<dwNoise> m_noiseList;
	bool getFlowVectorInterpolated(float x, float y, float& dx, float& dy);
	bool getFlowVectorRK4(float x, float y, float& dx, float& dy);
	void makeVectorCoherent(float ori_dx, float ori_dy, float& coh_dx, float& coh_dy) {
		if ((ori_dx * coh_dx + ori_dy * coh_dy) < 0) {
			coh_dx = -coh_dx;
			coh_dy = -coh_dy;
		}
	}
	void initFlowField() {
		if (m_nWidth && m_nHeight && !m_pFlowField)
			m_pFlowField = new float[m_nWidth * m_nHeight * 2];
		memset(m_pFlowField, 0, m_nWidth * m_nHeight * 2 * sizeof(float));
	}
	void initNoiseField() {
		if (m_nWidth && m_nHeight && !m_pNoiseField) {
			if (m_pIMG) {
				printf("initNoiseField - 3 Channel\n");
				m_pNoiseField = new unsigned char[m_nWidth * m_nHeight * m_pIMG->nChannels];
				memset(m_pNoiseField, 0, m_nWidth * m_nHeight * sizeof(unsigned char) * m_pIMG->nChannels);
			}
			else {
				printf("initNoiseField - 1 Channel\n");
				m_pNoiseField = new unsigned char[m_nWidth * m_nHeight];
				memset(m_pNoiseField, 0, m_nWidth * m_nHeight * sizeof(unsigned char));
			}
		}
	}
	void initImageField() {
		if (m_nWidth && m_nHeight && !m_pImageField) {
			//if (m_pIMG) {
			printf("initImageField - 3 Channel\n");
			m_pImageField = new unsigned char[m_nWidth * m_nHeight * 3];
			memset(m_pImageField, 0, m_nWidth * m_nHeight * sizeof(unsigned char) * 3);
			//}
			//else {
			//	printf("initImageField - 1 Channel\n");
			//	m_pImageField = new unsigned char[m_nWidth * m_nHeight];
			//	memset(m_pImageField, 0, m_nWidth * m_nHeight * sizeof(unsigned char));
			//}
		}
	}
	void initLICField() {
		if (m_nWidth && m_nHeight && !m_pLICField) {
			if (m_pIMG) {
				printf("initLICField - 3 Channel\n");
				m_pLICField = new unsigned char[m_nWidth * m_nHeight * m_pIMG->nChannels];
				memset(m_pLICField, 255, m_nWidth * m_nHeight * sizeof(unsigned char) * m_pIMG->nChannels);
			}
			else {
				printf("initLICField - 1 Channel\n");
				m_pLICField = new unsigned char[m_nWidth * m_nHeight];
				memset(m_pLICField, 255, m_nWidth * m_nHeight * sizeof(unsigned char));
			}
		}
	}
	void initPaperField() {
		if (m_nWidth && m_nHeight && !m_pPaperField) {
			printf("initLICField - 1 Channel\n");
			m_pPaperField = new float[m_nWidth * m_nHeight];
			memset(m_pPaperField, 0, m_nWidth * m_nHeight * sizeof(float));
		}
	}
	void initGaussianWeight() {
		if (m_nLength && !m_pGaussianWeight)
			m_pGaussianWeight = new float[m_nLength];
		memset(m_pGaussianWeight, 0, m_nLength * sizeof(float));
		getGaussianWeight(m_nLength, 2.0);
	}
	void getGaussianWeight(int lentgh, float sigma) {
		float r, s = 2.0 * sigma * sigma;

		float sum = 0.0;
		int y = 1;
		for (int x = 0; x <= lentgh; x++) {
			r = sqrt(x * x + y * y);
			m_pGaussianWeight[x] = (exp(-(r * r) / s)) / (M_PI * s);
			sum += m_pGaussianWeight[x];
		}
		for (int i = 0; i <= lentgh; ++i) {
			m_pGaussianWeight[i] /= sum;
		}
	}
	int randomRange(int n1, int n2) {
		return (int)(rand() % (n2 - n1 + 1)) + n1;
	}

public:
	dwLIC2() :dwLIC2(0, 0) {}
	dwLIC2(int nWidth, int nHeight, int nLength = 30, float fStepLength = 1.0f, IplImage* m_pImg = NULL)
		:m_nWidth(nWidth), m_nHeight(nHeight), m_nLength(nLength), m_fStepLength(fStepLength), m_pIMG(m_pImg) {}
	~dwLIC2() {
		if (m_pFlowField)
			delete[] m_pFlowField;
		if (m_pNoiseField)
			delete[] m_pNoiseField;
		if (m_pImageField)
			delete[] m_pImageField;
		if (m_pLICField)
			delete[] m_pLICField;
		if (m_pPaperField)
			delete[] m_pPaperField;

		//if (m_pCurve)
		//	delete[] m_pCurve;
		if (m_pGaussianWeight)
			delete[] m_pGaussianWeight;
	}
	unsigned char* m_pLICField;
	unsigned char* m_pNoiseField;
	unsigned char* m_pImageField;
	float* m_pPaperField;

	void init() {
		initFlowField();
		initLICField();
		initNoiseField();
		initImageField();
		initGaussianWeight();
		initPaperField();
	};
	void setFlowField(int x, int y, float dx, float dy);
	void setNoiseField(int x, int y);
	void setBlackNoiseField(int x, int y, int threshold);
	void setPaperField(int x, int y);
	void getJitteredSeedPoint(int x, int y, int space);
	void drawSeedStraight(int length, int weightflag);
	void drawSeedCurve(int length, float degree = NULL);
	void doLICForward(int weightflag = NULL);
	void doLICStraight(int weightflag = NULL);
	bool isImgExist() {
		if (m_pIMG) return true;
		else return false;
	};
};
