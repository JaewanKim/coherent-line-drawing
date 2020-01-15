
//#include "pch.h"
#include "dwLIC2.h"
#include <opencv2/opencv.hpp>

void dwLIC2::init() {
	initFlowField();
	initNoiseField();
	initLICField();

	if (m_nLength && !m_pGaussianWeight)
		m_pGaussianWeight = new float[m_nLength];
	memset(m_pGaussianWeight, 0, m_nLength * sizeof(float));
	getGaussianWeight(m_nLength, 40.0);
}

void dwLIC2::getGaussianWeight(int lentgh, float sigma) {
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
		printf("%f ", m_pGaussianWeight[i]);
	}
}

void dwLIC2::setFlowField(int x, int y, float dx, float dy)
{
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1))
	{
		std::cerr << "index error in setFlowField" << std::endl;
		return;
	}
	int index = m_nWidth * 2 * y + 2 * x;
	m_pFlowField[index] = dx;
	m_pFlowField[++index] = dy;
}

void dwLIC2::setNoiseField(float x, float y, unsigned char value)
{
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1))
	{
		std::cerr << "index error in setNoiseField" << std::endl;
		return;
	}
	int nx = x + 0.5f;
	int ny = y + 0.5f;
	//int index = m_nWidth * 2 * ny + 2 * nx;
	int index = m_nWidth * ny + nx;
	m_pNoiseField[index] = value;
	m_noiseList.push_back(dwNoise(x, y));
	//if (nx % 2 == 0 && ny % 2==0)
	//    m_noiseList.push_back(dwNoise(nx, ny));
}

void dwLIC2::doLICForward()
{
	for (auto& noise : m_noiseList)
	{
		float x, y;
		float ori_x = x = noise.x;
		float ori_y = y = noise.y;
		float ori_dx, ori_dy;

		int sum = 0;
		int count = 0;

		m_pCurve = new float[2 * (2 * m_nLength + 1)];
		memset(m_pCurve, 0, (2 * (2 * m_nLength + 1)) * sizeof(float));
		m_pCurve[0] = noise.x;
		m_pCurve[1] = noise.y;
		int idx = 2;
		for (int i = -1; i < 2; i += 2)
		{
			float dx, dy;
			x = ori_x;
			y = ori_y;
			if (!getFlowVectorInterpolated(x, y, ori_dx, ori_dy))
				continue;
			dx = ori_dx;
			dy = ori_dy;
			if (!getFlowVectorRK4(x, y, dx, dy, i))
				continue;

			for (int j = m_nLength; j > 0; --j)
			{
				if (((x + dx) > (m_nWidth + 1)) || ((x + dx) < 0)
					|| ((y + dy) > (m_nHeight + 1)) || ((y + dy) < 0))
				{
					break;
				}
				else
				{
					x += dx;
					y += dy;

					m_pCurve[idx++] = x;
					m_pCurve[idx++] = y;

					//sum += m_pNoiseField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)];
					//count++;

					//m_pLICField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)] += j;
					if (!getFlowVectorRK4(x, y, dx, dy, i))
						break;
				}
			}
		}
		float tot = 0;
		float tot2 = 0;


		for (int j = m_nLength; j > 0; --j) {
			int x = (int)(m_pCurve[idx - i] + 0.5);
			int y = (int)(m_pCurve[idx - i + 1] + 0.5);
			float weight = m_pGaussianWeight[(idx - i) / 2];
			tot += (m_pNoiseField[y * m_nWidth + x] * weight);
			tot2 += weight;
		}
		for (int i = idx; i >= 2; i -= 2) {
			int x = (int)(m_pCurve[idx - i] + 0.5);
			int y = (int)(m_pCurve[idx - i + 1] + 0.5);
			float weight = m_pGaussianWeight[(idx - i) / 2];
			tot += (m_pNoiseField[y * m_nWidth + x] * weight);
			tot2 += weight;
		}
		if (tot2 != 0.0) {
			tot /= tot2;
		}
		m_pLICField[int(noise.y + 0.5f) * m_nWidth + int(noise.x + 0.5f)] = (unsigned char)tot;

		//if (count != 0)
		//	sum /= count;
		//m_pLICField[int(noise.y + 0.5f) * m_nWidth + int(noise.x + 0.5f)] = sum;

	}
}

bool dwLIC2::getFlowVectorRK4(float x, float y, float& dx, float& dy, int direction)
{
	if ((x < 0) || (x > (m_nWidth - 1)) || (y < 0) || (y > (m_nHeight - 1)))
	{
		//std::cerr << "out of index occurs in getFlowVectorRK4" << std::endl;
		return false;
	}

	float dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4, dx_temp, dy_temp, ori_dx, ori_dy;

	ori_dx = dx_temp = dx * direction;
	ori_dy = dy_temp = dy * direction;
	dx1 = m_fStepLength * dx_temp;
	dy1 = m_fStepLength * dy_temp;

	getFlowVectorInterpolated(x + 0.5f * dx1, y + 0.5f * dy1, dx_temp, dy_temp);
	makeVectorCoherent(ori_dx, ori_dy, dx_temp, dy_temp);
	dx2 = m_fStepLength * dx_temp;
	dy2 = m_fStepLength * dy_temp;

	getFlowVectorInterpolated(x + 0.5f * dx2, y + 0.5f * dy2, dx_temp, dy_temp);
	makeVectorCoherent(ori_dx, ori_dy, dx_temp, dy_temp);
	dx3 = m_fStepLength * dx_temp;
	dy3 = m_fStepLength * dy_temp;

	getFlowVectorInterpolated(x + dx3, y + dy3, dx_temp, dy_temp);
	makeVectorCoherent(ori_dx, ori_dy, dx_temp, dy_temp);
	dx4 = m_fStepLength * dx_temp;
	dy4 = m_fStepLength * dy_temp;

	dx = (dx1 + 2 * dx2 + 2 * dx3 + dx4) * asixth;
	dy = (dy1 + 2 * dy2 + 2 * dy3 + dy4) * asixth;

	return true;
}

bool dwLIC2::getFlowVectorInterpolated(float x, float y, float& dx, float& dy)
{
	if ((x < 0) || (x > (m_nWidth - 1)) || (y < 0) || (y > (m_nHeight - 1)))
	{
		//std::cerr << "out of index occurs in getFlowVectorInterpolated" << std::endl;
		return false;
	}
	int nx = x;
	int ny = y;

	if (x == (m_nWidth - 1))
	{
		x -= 1.f;
		--nx;
	}
	if (y == (m_nHeight - 1))
	{
		y -= 1.f;
		--ny;
	}

	float fx1 = x - nx;
	float fx2 = 1.f - fx1;
	float fy1 = y - ny;
	float fy2 = 1.f - fy1;

	int index = ny * m_nWidth * 2 + nx * 2;
	float dx1 = m_pFlowField[index];
	float dy1 = m_pFlowField[++index];
	float dx2 = m_pFlowField[++index];
	float dy2 = m_pFlowField[++index];

	index = (ny + 1) * m_nWidth * 2 + nx * 2;
	float dx3 = m_pFlowField[index];
	float dy3 = m_pFlowField[++index];
	float dx4 = m_pFlowField[++index];
	float dy4 = m_pFlowField[++index];

	float fx2fy2 = fx2 * fy2;
	float fx1fy2 = fx1 * fy2;
	float fx2fy1 = fx2 * fy1;
	float fx1fy1 = fx1 * fy1;
	dx = (fx2fy2 * dx1) + (fx1fy2 * dx2) + (fx2fy1 * dx3) + (fx1fy1 * dx4);
	dy = (fx2fy2 * dy1) + (fx1fy2 * dy2) + (fx2fy1 * dy3) + (fx1fy1 * dy4);

	float fSize = std::sqrt(dx * dx + dy * dy);
	dx /= fSize;
	dy /= fSize;

	return true;
}