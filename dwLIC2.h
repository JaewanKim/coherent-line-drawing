
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

	//friend class dwVoronoi;
	float m_fStepLength;
	int m_nWidth;
	int m_nHeight;
	int m_nLength;
	float* m_pFlowField = NULL;
	//float * m_pLICField = NULL;
	//unsigned char* m_pNoiseField = NULL;

	std::vector<dwNoise> m_noiseList;
	bool getFlowVectorInterpolated(float x, float y, float& dx, float& dy);
	bool getFlowVectorRK4(float x, float y, float& dx, float& dy, int direction);
	void makeVectorCoherent(float ori_dx, float ori_dy, float& coh_dx, float& coh_dy)
	{
		if ((ori_dx * coh_dx + ori_dy * coh_dy) < 0)
		{
			coh_dx = -coh_dx;
			coh_dy = -coh_dy;
		}
	}

	void initFlowField()
	{
		if (m_nWidth && m_nHeight && !m_pFlowField)
		{
			m_pFlowField = new float[m_nWidth * m_nHeight * 2];
		}
		memset(m_pFlowField, 0, m_nWidth * m_nHeight * 2 * sizeof(float));
	}

	void initNoiseField()
	{
		if (m_nWidth && m_nHeight && !m_pNoiseField)
		{
			m_pNoiseField = new unsigned char[m_nWidth * m_nHeight];
		}
		memset(m_pNoiseField, 0, m_nWidth * m_nHeight * sizeof(unsigned char));
	}

	void initLICField()
	{
		if (m_nWidth && m_nHeight && !m_pLICField)
		{
			m_pLICField = new float[m_nWidth * m_nHeight];
		}
		memset(m_pLICField, 0, m_nWidth * m_nHeight * sizeof(float));
	}

public:
	dwLIC2() :dwLIC2(0, 0) {}
	dwLIC2(int nWidth, int nHeight, int nLength = 200, float fStepLength = 1.0f)
		:m_nWidth(nWidth), m_nHeight(nHeight), m_nLength(nLength), m_fStepLength(fStepLength) {}
	~dwLIC2()
	{
		if (m_pFlowField)
			delete[] m_pFlowField;
		if (m_pNoiseField)
			delete[] m_pNoiseField;
		if (m_pLICField)
			delete[] m_pLICField;

		if (m_pCurve)
			delete[] m_pCurve;
		if (m_pGaussianWeight)
			delete[] m_pGaussianWeight;
	}
	float* m_pLICField;
	float* m_pCurve;
	float* m_pGaussianWeight;
	unsigned char* m_pNoiseField;
	void setFlowField(int x, int y, float dx, float dy);
	void setNoiseField(float x, float y, unsigned char value);
	void doLICForward();
	void init();
	void getGaussianWeight(int lentgh, float sigma = 1.f);
};
