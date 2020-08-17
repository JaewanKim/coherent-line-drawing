
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
	int m_nChannel;
	int m_nWidthStep;
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
	void initLICField() {
		if (m_nWidth && m_nHeight && !m_pLICField) {
			printf("initLICField - %d Channel\n", m_nChannel);
			m_pLICField = new unsigned char[m_nWidth * m_nHeight * m_nChannel];
			memset(m_pLICField, 0, m_nWidth * m_nHeight * sizeof(unsigned char) * m_nChannel);
		}
	}
	void initNoiseField() {
		if (m_nWidth && m_nHeight && !m_pNoiseField) {
			printf("initNoiseField - %d Channel\n", m_nChannel);
			m_pNoiseField = new unsigned char[m_nWidth * m_nHeight * m_nChannel];
			memset(m_pNoiseField, 0, m_nWidth * m_nHeight * sizeof(unsigned char) * m_nChannel);
		}
	}
	void initGaussianWeight() {
		if (m_nLength && !m_pGaussianWeight)
			m_pGaussianWeight = new float[m_nLength];
		memset(m_pGaussianWeight, 0, m_nLength * sizeof(float));
		getGaussianWeight(m_nLength, 5.0);
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

public:
	dwLIC2() :dwLIC2(0, 0, 0, 0) {}
	dwLIC2(int nWidth, int nHeight, int nChannel, int nWidthStep, int nLength = 30, float fStepLength = 1.0f)
		:m_nWidth(nWidth), m_nHeight(nHeight), m_nChannel(nChannel), m_nWidthStep(nWidthStep), m_nLength(nLength), m_fStepLength(fStepLength) {}
	~dwLIC2() {
		if (m_pFlowField)
			delete[] m_pFlowField;
		if (m_pNoiseField)
			delete[] m_pNoiseField;
		if (m_pLICField)
			delete[] m_pLICField;

		if (m_pGaussianWeight)
			delete[] m_pGaussianWeight;
	}
	unsigned char* m_pLICField;
	unsigned char* m_pNoiseField;
	void init() {
		initFlowField();
		initLICField();
		initNoiseField();
		initGaussianWeight();
	};
	void setFlowField(int x, int y, float dx, float dy);
	void setNoiseField(int x, int y, unsigned char v);
	void setNoiseField2(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void setBlackNoiseField(int x, int y, int threshold);
	void doLICForward(int weightflag);
	void doLICStraight(int weightflag);
};