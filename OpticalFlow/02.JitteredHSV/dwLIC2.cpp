
//#include "pch.h"
#include "dwLIC2.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <random>
#include <ppl.h>
using namespace cv;
using namespace std;
using namespace concurrency;

random_device rn;
mt19937_64 rnd(rn());
uniform_int_distribution<int> range(0, 255);

void dwLIC2::setNoiseField2(int x, int y, unsigned char value) {
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1)) {
		std::cerr << "index error in setNoiseField" << std::endl;
		return;
	}
	else {
		int index = m_nWidth * y + x;
		m_pNoiseField[index] = value;
	}
	m_noiseList.push_back(dwNoise(x, y));
}

void dwLIC2::setBlackNoiseField(int x, int y, int threshold) {
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1)) {
		std::cerr << "index error in setNoiseField" << std::endl;
		return;
	}
	int index = m_nWidth * y + x;
	m_pNoiseField[index] = (range(rnd) > threshold) ? (unsigned char)0 : (unsigned char)255;

	m_noiseList.push_back(dwNoise(x, y));
}

void dwLIC2::setPaperField(int x, int y) {
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1)) {
		std::cerr << "index error in setNoiseField" << std::endl;
		return;
	}
	int index = m_nWidth * y + x;
	//m_pPaperField[index] = range(rnd) % 2;
	m_pPaperField[index] = range(rnd) / 255.f;
}

void dwLIC2::getJitteredSeedPoint(int x, int y, int space) {
	if (x < 0 || x >(m_nWidth - 1) || y < 0 || y >(m_nHeight - 1)) {
		std::cerr << "index error in getJitteredSeedPoint" << std::endl;
		return;
	}
	if (space <= 0) {
		std::cerr << "space shoulde be greater than 0 in getJitteredSeedPoint" << std::endl;
		return;
	}
	int index = m_nWidth * y + x;
	m_pNoiseField[index] = (unsigned char)(rand() % 256);
	if (x % space == 0 && y % space == 0) {
		if (x == 0 || x == m_nWidth || y == 0 || y == m_nHeight)
			return;
		space = (int)sqrt(space);
		int randx = randomRange(-space, space);
		int randy = randomRange(-space, space);
		m_noiseList.push_back(dwNoise(x + randx, y + randy));
	}
}

void dwLIC2::drawSeedStraight(int length, int weightflag) {

	int nListSize = m_noiseList.size();
	parallel_for(int(0), nListSize, [&](int noise_index) {
		auto& noise = m_noiseList[noise_index];

		vector<dwNoise> m_pCurve[2];
		m_pCurve[0].clear();
		m_pCurve[1].clear();

		float x, y;
		float ori_x = x = noise.x;
		float ori_y = y = noise.y;
		float ori_dx, ori_dy;

		for (int i = 0; i < 2; ++i)
			m_pCurve[i].push_back(dwNoise(noise.x, noise.y));

		int dir = 0;
		for (int i = -1; i < 2; i += 2, ++dir) {

			int index = noise.y * m_nWidth * 2 + noise.x * 2;
			float dx = m_pFlowField[index];
			float dy = m_pFlowField[index + 1];

			x = ori_x;
			y = ori_y;
			if (!getFlowVectorInterpolated(x, y, ori_dx, ori_dy))
				continue;
			dx = (ori_dx * i);
			dy = (ori_dy * i);

			int space = (int)sqrt(length);
			int rand = randomRange(-length + space, length - space);
			int RandLength = length + rand;

			for (int j = RandLength; j > 0; --j) {
				if (((x + dx) > (m_nWidth - 1)) || ((x + dx) < 0) || ((y + dy) > (m_nHeight - 1)) || ((y + dy) < 0))
					break;
				else {
					x += dx;
					y += dy;

					m_pCurve[dir].push_back(dwNoise(x, y));
				}
			}
		}

		int tot = 0;
		int tot2 = 0.f;
		float weight;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);


				if (weightflag == 0)
					weight = 1.f;
				else if (weightflag == 1)
					weight = m_pCurve[i].size() - j;
				else if (weightflag == 2)
					weight = m_pGaussianWeight[j];
				
				//printf("%f ", weight);
				tot += m_pImageField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)] * weight;
				//tot += (m_pNoiseField[y * m_nWidth + x] * weight);
				tot2 += weight;
			}
		}
		if (tot2 != 0)
			tot /= tot2;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);

				int ori_x = m_pCurve[0][0].x;
				int ori_y = m_pCurve[0][0].y;

				int ori_color = m_pImageField[ori_y * m_nWidth + ori_x];
				int new_color = m_pImageField[y * m_nWidth + x];
				int th_color = 64;

				if ((ori_color - new_color) * (ori_color - new_color) > (th_color * th_color))
					break;

				if (tot > 255) tot = 255;
				else if (tot < 0) tot = 0;

				m_pLICField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)] = (unsigned char)tot;
			}
		}
	});
}

void dwLIC2::drawSeedCurve(int length, float degree) {

	const float theta = degree / 180.0 * M_PI;
	const float cosTheta = cos(theta);
	
	int nListSize = m_noiseList.size();
	parallel_for(int(0), nListSize, [&](int noise_index) {
		auto& noise = m_noiseList[noise_index];

		vector<dwNoise> m_pCurve[2];
		m_pCurve[0].clear();
		m_pCurve[1].clear();

		float x, y;
		float ori_x = x = noise.x;
		float ori_y = y = noise.y;
		float ori_dx, ori_dy;

		int sum = 0;
		int count = 0;
		for (int i = 0; i < 2; ++i)
			m_pCurve[i].push_back(dwNoise(noise.x, noise.y));

		int dir = 0;
		for (int i = -1; i < 2; i += 2, ++dir) {
			float dx, dy;
			float prev_dx, prev_dy;
			x = ori_x;
			y = ori_y;
			if (!getFlowVectorInterpolated(x, y, ori_dx, ori_dy))
				continue;
			dx = (ori_dx * i);
			dy = (ori_dy * i);

			if (!getFlowVectorRK4(x, y, dx, dy))
				continue;

			prev_dx = dx;
			prev_dy = dy;

			//int space = (int)sqrt(length);
			//int rand = randomRange(-space, space);
			//int RandLength = length + rand;

			for (int j = length; j > 0; --j) {
				if (((x + dx) > (m_nWidth - 1)) || ((x + dx) < 0) || ((y + dy) > (m_nHeight - 1)) || ((y + dy) < 0))
					break;
				else {
					x += dx;
					y += dy;

					if (theta == NULL)
						m_pCurve[dir].push_back(dwNoise(x, y));
					else {
						float len_dx_dy = sqrt(dx * dx + dy * dy);
						float unit_dx = dx / len_dx_dy;
						float unit_dy = dy / len_dx_dy;
						float len_dx_dy2 = sqrt(prev_dx * prev_dx + prev_dy * prev_dy);
						float unit_prev_dx = prev_dx / len_dx_dy2;
						float unit_prev_dy = prev_dy / len_dx_dy2;
						float dot = (unit_dx * unit_prev_dx + unit_dy * unit_prev_dy);
						if (dot < cosTheta)
							break;
						else
							m_pCurve[dir].push_back(dwNoise(x, y));
					}

					prev_dx = dx;
					prev_dy = dy;

					if (!getFlowVectorRK4(x, y, dx, dy))
						break;
				}
			}
		}

		//for (int i = 0; i < 2; i++) {
		//	for (int j = 0; j < m_pCurve[i].size(); j++) {
		//		if (i == 0 && j == 0)
		//			continue;
		//		int x = (int)(m_pCurve[i][j].x + 0.5f);
		//		int y = (int)(m_pCurve[i][j].y + 0.5f);

		//		m_pLICField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)] = (unsigned char)0;
		//	}
		//}

		int tot = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);
				tot += m_pImageField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)];
			}
		}
		int tot2 = m_pCurve[0].size() + m_pCurve[1].size();
		if (tot2 != 0)
			tot /= tot2;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);
				m_pLICField[int(y + 0.5f) * m_nWidth + int(x + 0.5f)] = tot;
			}
		}
	});
}

void dwLIC2::doLICForward(int weightflag) {

	int nListSize = m_noiseList.size();
	parallel_for(int(0), nListSize, [&](int noise_index) {
	//for (auto& noise : m_noiseList) {
		auto& noise = m_noiseList[noise_index];

		vector<dwNoise> m_pCurve[2];

		m_pCurve[0].clear();
		m_pCurve[1].clear();

		float x, y;
		float ori_x = x = noise.x;
		float ori_y = y = noise.y;
		float ori_dx, ori_dy;

		int sum = 0;
		int count = 0;
		for (int i = 0; i < 2; ++i)
			m_pCurve[i].push_back(dwNoise(noise.x, noise.y));

		int dir = 0;
		int idx = 2;
		for (int i = -1; i < 2; i += 2, ++dir) {
			float dx, dy;
			x = ori_x;
			y = ori_y;
			if (!getFlowVectorInterpolated(x, y, ori_dx, ori_dy))
				continue;
			dx = (ori_dx * i);
			dy = (ori_dy * i);

			if (!getFlowVectorRK4(x, y, dx, dy))
				continue;

			for (int j = m_nLength; j > 0; --j) {
				if (((x + dx) > (m_nWidth - 1)) || ((x + dx) < 0) || ((y + dy) > (m_nHeight - 1)) || ((y + dy) < 0))
					break;
				else {
					x += dx;
					y += dy;

					m_pCurve[dir].push_back(dwNoise(x, y));

					if (!getFlowVectorRK4(x, y, dx, dy))
						break;
				}
			}
		}

		float tot = 0.f;
		float totB = 0.f;
		float totG = 0.f;
		float totR = 0.f;
		float tot2 = 0.f;
		float weight;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);

				if (weightflag == 0)
					weight = 1.f;
				else if (weightflag == 1)
					weight = m_pCurve[i].size() - j;
				else if (weightflag == 2)
					weight = m_pGaussianWeight[j];

				if (m_pIMG) {
					totB += (m_pNoiseField[y * m_pIMG->widthStep + x * m_pIMG->nChannels + 0] * weight);
					totG += (m_pNoiseField[y * m_pIMG->widthStep + x * m_pIMG->nChannels + 1] * weight);
					totR += (m_pNoiseField[y * m_pIMG->widthStep + x * m_pIMG->nChannels + 2] * weight);
				}
				else {
					int ori_x = m_pCurve[0][0].x;
					int ori_y = m_pCurve[0][0].y;

					int ori_color = m_pImageField[ori_y * m_nWidth + ori_x];
					int new_color = m_pImageField[y * m_nWidth + x];
					int th_color = 128;

					if ((ori_color - new_color) * (ori_color - new_color) > (th_color * th_color))
						break;

					tot += (m_pNoiseField[y * m_nWidth + x] * weight);
				}
				tot2 += weight;
			}
		}
		if (m_pIMG) {
			if (tot2 != 0.0) {
				totB /= tot2;
				totG /= tot2;
				totR /= tot2;
			}
			m_pLICField[int(noise.y + 0.5f) * m_pIMG->widthStep + int(noise.x + 0.5f) * m_pIMG->nChannels + 0] = (unsigned char)totB;
			m_pLICField[int(noise.y + 0.5f) * m_pIMG->widthStep + int(noise.x + 0.5f) * m_pIMG->nChannels + 1] = (unsigned char)totG;
			m_pLICField[int(noise.y + 0.5f) * m_pIMG->widthStep + int(noise.x + 0.5f) * m_pIMG->nChannels + 2] = (unsigned char)totR;
		}
		else {
			if (tot2 != 0.0)
				tot /= tot2;

			m_pLICField[int(noise.y + 0.5f) * m_nWidth + int(noise.x + 0.5f)] = (unsigned char)tot;
		}
	});
	//}
}

void dwLIC2::doLICStraight(int weightflag) {

	int nListSize = m_noiseList.size();
	parallel_for(int(0), nListSize, [&](int noise_index) {
		auto& noise = m_noiseList[noise_index];

		vector<dwNoise> m_pCurve[2];

		m_pCurve[0].clear();
		m_pCurve[1].clear();

		float x, y;
		float ori_x = x = noise.x;
		float ori_y = y = noise.y;
		float ori_dx, ori_dy;

		for (int i = 0; i < 2; ++i)
			m_pCurve[i].push_back(dwNoise(noise.x, noise.y));

		int dir = 0;
		int idx = 2;
		for (int i = -1; i < 2; i += 2, ++dir) {
			float dx, dy;
			x = ori_x;
			y = ori_y;
			if (!getFlowVectorInterpolated(x, y, ori_dx, ori_dy))
				continue;
			dx = (ori_dx * i);
			dy = (ori_dy * i);

			for (int j = m_nLength; j > 0; --j) {
				if (((x + dx) > (m_nWidth - 1)) || ((x + dx) < 0) || ((y + dy) > (m_nHeight - 1)) || ((y + dy) < 0))
					break;
				else {
					x += dx;
					y += dy;

					m_pCurve[dir].push_back(dwNoise(x, y));
				}
			}
		}

		float tot = 0.f;
		float tot2 = 0.f;
		float weight;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < m_pCurve[i].size(); j++) {
				if (i == 0 && j == 0)
					continue;
				int x = (int)(m_pCurve[i][j].x + 0.5f);
				int y = (int)(m_pCurve[i][j].y + 0.5f);

				if (weightflag == 0)
					weight = 1.f;
				else if (weightflag == 1)
					weight = m_pCurve[i].size() - j;
				else if (weightflag == 2)
					weight = m_pGaussianWeight[j];

				int ori_x = m_pCurve[0][0].x;
				int ori_y = m_pCurve[0][0].y;

				//int ori_color = m_pImageField[ori_y * m_nWidth + ori_x];
				//int new_color = m_pImageField[y * m_nWidth + x];
				//int th_color = 84;

				//if ((ori_color - new_color) * (ori_color - new_color) > (th_color * th_color))
				//	break;

				tot += (m_pNoiseField[y * m_nWidth + x] * weight);
				tot2 += weight;
			}
		}
		if (tot2 != 0.0)
			tot /= tot2;

		m_pLICField[int(noise.y + 0.5f) * m_nWidth + int(noise.x + 0.5f)] = (unsigned char)tot;
	});
}

bool dwLIC2::getFlowVectorRK4(float x, float y, float& dx, float& dy) {
	if ((x < 0) || (x > (m_nWidth - 1)) || (y < 0) || (y > (m_nHeight - 1)))
		return false;

	float dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4, dx_temp, dy_temp, ori_dx, ori_dy;

	ori_dx = dx_temp = dx;
	ori_dy = dy_temp = dy;
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

bool dwLIC2::getFlowVectorInterpolated(float x, float y, float& dx, float& dy) {
	if ((x < 0) || (x > (m_nWidth - 1)) || (y < 0) || (y > (m_nHeight - 1)))
		return false;

	int nx = x;
	int ny = y;

	if (x == (m_nWidth - 1)) {
		x -= 1.f;
		--nx;
	}
	if (y == (m_nHeight - 1)) {
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
	makeVectorCoherent(dx1, dy1, dx2, dy2);

	index = (ny + 1) * m_nWidth * 2 + nx * 2;
	float dx3 = m_pFlowField[index];
	float dy3 = m_pFlowField[++index];
	makeVectorCoherent(dx1, dy1, dx3, dy3);
	float dx4 = m_pFlowField[++index];
	float dy4 = m_pFlowField[++index];
	makeVectorCoherent(dx1, dy1, dx4, dy4);

	float fx2fy2 = fx2 * fy2;
	float fx1fy2 = fx1 * fy2;
	float fx2fy1 = fx2 * fy1;
	float fx1fy1 = fx1 * fy1;
	dx = (fx2fy2 * dx1) + (fx1fy2 * dx2) + (fx2fy1 * dx3) + (fx1fy1 * dx4);
	dy = (fx2fy2 * dy1) + (fx1fy2 * dy2) + (fx2fy1 * dy3) + (fx1fy1 * dy4);

	float fSize = std::sqrt(dx * dx + dy * dy);
	if (fSize != 0)
	{
		dx /= fSize;
		dy /= fSize;
		//return false;
	}
	return true;
}
