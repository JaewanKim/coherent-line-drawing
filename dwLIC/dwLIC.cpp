
#include "dwLIC.h"
#include <opencv2/opencv.hpp>

const float dwLIC::LOWPASS_FILTR_LENGTH = 10.0f;
const float dwLIC::LINE_SQUARE_CLIP_MAX = 100000.0f;
const float dwLIC::VECTOR_COMPONENT_MIN = 0.050000f;

dwLIC::~dwLIC(void)
{
	if (pNoise)
		delete[] pNoise;
	if (pNoiseB)
		delete[] pNoiseB;
	if (pNoiseG)
		delete[] pNoiseG;
	if (pNoiseR)
		delete[] pNoiseR;
	if (pVectr)
		delete[] pVectr;
}

void dwLIC::MakeWhiteNoise()
{
	pNoise = new unsigned char[n_xres * n_yres];
	for (int j = 0; j < n_yres; j++)
	{
		for (int i = 0; i < n_xres; i++)
		{
			int  r = rand();
			r = ((r & 0xff) + ((r & 0xff00) >> 8)) & 0xff;
			pNoise[j * n_xres + i] = (unsigned char)r;
		}
	}
}

void dwLIC::NoiseFromImage(IplImage* img)
{
	pNoiseB = new unsigned char[n_xres * n_yres];
	pNoiseG = new unsigned char[n_xres * n_yres];
	pNoiseR = new unsigned char[n_xres * n_yres];

	for (int j = 0; j < n_yres; j++)
	{
		for (int i = 0; i < n_xres; i++)
		{
			pNoiseB[(n_yres - j - 1) * n_xres + i] = (unsigned char)img->imageData[j * img->widthStep + i * img->nChannels + 0];
			pNoiseG[(n_yres - j - 1) * n_xres + i] = (unsigned char)img->imageData[j * img->widthStep + i * img->nChannels + 1];
			pNoiseR[(n_yres - j - 1) * n_xres + i] = (unsigned char)img->imageData[j * img->widthStep + i * img->nChannels + 2];
		}
	}
	bNoiseImage = true;
}

///		generate box filter LUTs     ///
void dwLIC::GenBoxFiltrLUT()
{
	p_LUT0 = new float[DISCRETE_FILTER_SIZE];
	p_LUT1 = new float[DISCRETE_FILTER_SIZE];

	for (int i = 0; i < LUTsiz; i++)
	{
		p_LUT0[i] = p_LUT1[i] = i;
	}
}

void dwLIC::NormalizVectrs()
{
	for (int j = 0; j < n_yres; j++)
	{
		for (int i = 0; i < n_xres; i++)
		{
			int		index = (j * n_xres + i) << 1;
			float	vcMag = float(sqrt(double(pVectr[index] * pVectr[index] + pVectr[index + 1] * pVectr[index + 1])));

			float	scale = (vcMag == 0.0f) ? 0.0f : 1.0f / vcMag;
			pVectr[index] *= scale;
			pVectr[index + 1] *= scale;
		}
	}
}

///		flow imaging (visualization) through Line Integral Convolution     ///
void dwLIC::FlowImagingLIC()
{
	NormalizVectrs();
	if (!bNoiseImage) {
		MakeWhiteNoise();
		pOutputImg = cvCreateImage(cvSize(n_xres, n_yres), 8, 1);	// GRAY 1 channel
	}
	else
		pOutputImg = cvCreateImage(cvSize(n_xres, n_yres), 8, 3);	// RGB 3 channel

	LUTsiz = DISCRETE_FILTER_SIZE;
	GenBoxFiltrLUT();
	krnlen = LOWPASS_FILTR_LENGTH;

	int		vec_id;						///ID in the VECtor buffer (for the input flow field)
	int		advDir;						///ADVection DIRection (0: positive;  1: negative)
	int		advcts;						///number of ADVeCTion stepS per direction (a step counter)
	int		ADVCTS = int(krnlen * 3);	///MAXIMUM number of advection steps per direction to break dead loops	

	float	vctr_x;						///x-component  of the VeCToR at the forefront point
	float	vctr_y;						///y-component  of the VeCToR at the forefront point
	float	clp0_x;						///x-coordinate of CLiP point 0 (current)
	float	clp0_y;						///y-coordinate of CLiP point 0	(current)
	float	clp1_x;						///x-coordinate of CLiP point 1 (next   )
	float	clp1_y;						///y-coordinate of CLiP point 1 (next   )
	float	samp_x;						///x-coordinate of the SAMPle in the current pixel
	float	samp_y;						///y-coordinate of the SAMPle in the current pixel
	float	tmpLen;						///TeMPorary LENgth of a trial clipped-segment
	float	segLen;						///SEGment   LENgth
	float	curLen;						///CURrent   LENgth of the streamline
	float	prvLen;						///PReVious  LENgth of the streamline		
	float	W_ACUM;						///ACcuMulated Weight from the seed to the current streamline forefront
	float	texVal;						///TEXture VALue
	float	texValB;					///TEXture VALue
	float	texValG;					///TEXture VALue
	float	texValR;					///TEXture VALue
	float	smpWgt;						///WeiGhT of the current SaMPle
	float	t_acum[2];					///two ACcUMulated composite Textures for the two directions, perspectively
	float	t_acumB[2];					///two ACcUMulated composite Textures for the two directions, perspectively
	float	t_acumG[2];					///two ACcUMulated composite Textures for the two directions, perspectively
	float	t_acumR[2];					///two ACcUMulated composite Textures for the two directions, perspectively
	float	w_acum[2];					///two ACcUMulated Weighting values   for the two directions, perspectively
	float* wgtLUT = NULL;				///WeiGhT Look Up Table pointing to the target filter LUT
	float	len2ID = (DISCRETE_FILTER_SIZE - 1) / krnlen;	///map a curve LENgth TO an ID in the LUT

	///for each pixel in the 2D output LIC image///
	for (int j = 0; j < n_yres; j++)
	{
		for (int i = 0; i < n_xres; i++)
		{
			///init the composite texture accumulators and the weight accumulators///
			t_acum[0] = t_acum[1] = t_acumB[0] = t_acumB[1] = t_acumG[0] = t_acumG[1] = t_acumR[0] = t_acumR[1] = w_acum[0] = w_acum[1] = 0.0f;

			///for either advection direction///
			for (advDir = 0; advDir < 2; advDir++)
			{
				///init the step counter, curve-length measurer, and streamline seed///
				advcts = 0;
				curLen = 0.0f;
				clp0_x = i + 0.5f;
				clp0_y = j + 0.5f;

				///access the target filter LUT///
				wgtLUT = (advDir == 0) ? p_LUT0 : p_LUT1;

				vec_id = (int(clp0_y) * n_xres + int(clp0_x)) << 1;
				float prevVctr_x = pVectr[vec_id];
				float prevVctr_y = pVectr[vec_id + 1];

				///until the streamline is advected long enough or a tightly  spiralling center / focus is encountered///
				while (curLen < krnlen && advcts < ADVCTS)
				{
					///access the vector at the sample///
					vec_id = (int(clp0_y) * n_xres + int(clp0_x)) << 1;
					vctr_x = pVectr[vec_id];
					vctr_y = pVectr[vec_id + 1];

					if (vctr_x * prevVctr_x + vctr_y * prevVctr_y < 0)
					{
						vctr_x = -vctr_x;
						vctr_y = -vctr_y;
					}
					prevVctr_x = vctr_x;
					prevVctr_y = vctr_y;

					///in case of a critical point///
					if (vctr_x == 0.0f && vctr_y == 0.0f)
					{

						if (!bNoiseImage) {
							t_acum[advDir] = (advcts == 0) ? 0.0f : t_acum[advDir];		   ///this line is indeed unnecessary
						}
						else {
							t_acumB[advDir] = (advcts == 0) ? 0.0f : t_acumB[advDir];		   ///this line is indeed unnecessary
							t_acumG[advDir] = (advcts == 0) ? 0.0f : t_acumG[advDir];		   ///this line is indeed unnecessary
							t_acumR[advDir] = (advcts == 0) ? 0.0f : t_acumR[advDir];		   ///this line is indeed unnecessary
						}
						w_acum[advDir] = (advcts == 0) ? 1.0f : w_acum[advDir];
						break;
					}

					///negate the vector for the backward-advection case///
					vctr_x = (advDir == 0) ? vctr_x : -vctr_x;
					vctr_y = (advDir == 0) ? vctr_y : -vctr_y;

					///clip the segment against the pixel boundaries --- find the shorter from the two clipped segments///
					///replace  all  if-statements  whenever  possible  as  they  might  affect the computational speed///
					segLen = LINE_SQUARE_CLIP_MAX;
					segLen = (vctr_x < -VECTOR_COMPONENT_MIN) ? (int(clp0_x) - clp0_x) / vctr_x : segLen;
					segLen = (vctr_x > VECTOR_COMPONENT_MIN) ? (int(int(clp0_x) + 1.5f) - clp0_x) / vctr_x : segLen;
					segLen = (vctr_y < -VECTOR_COMPONENT_MIN) ?
						(((tmpLen = (int(clp0_y) - clp0_y) / vctr_y) < segLen) ? tmpLen : segLen)
						: segLen;
					segLen = (vctr_y > VECTOR_COMPONENT_MIN) ?
						(((tmpLen = (int(int(clp0_y) + 1.5f) - clp0_y) / vctr_y) < segLen) ? tmpLen : segLen)
						: segLen;

					///update the curve-length measurers///
					prvLen = curLen;
					curLen += segLen;
					segLen += 0.0004f;

					///check if the filter has reached either end///
					segLen = (curLen > krnlen) ? ((curLen = krnlen) - prvLen) : segLen;

					///obtain the next clip point///
					clp1_x = clp0_x + vctr_x * segLen;
					clp1_y = clp0_y + vctr_y * segLen;

					///obtain the middle point of the segment as the texture-contributing sample///
					samp_x = (clp0_x + clp1_x) * 0.5f;
					samp_y = (clp0_y + clp1_y) * 0.5f;

					///obtain the texture value of the sample///
					if (!bNoiseImage) {
						texVal = pNoise[int(samp_y) * n_xres + int(samp_x)];
					}
					else {
						texValB = pNoiseB[int(samp_y) * n_xres + int(samp_x)];
						texValG = pNoiseG[int(samp_y) * n_xres + int(samp_x)];
						texValR = pNoiseR[int(samp_y) * n_xres + int(samp_x)];
					}

					///update the accumulated weight and the accumulated composite texture (texture x weight)///
					W_ACUM = wgtLUT[int(curLen * len2ID)];
					smpWgt = W_ACUM - w_acum[advDir];
					w_acum[advDir] = W_ACUM;

					if (!bNoiseImage) {
						t_acum[advDir] += texVal * smpWgt;
					}
					else {
						t_acumB[advDir] += texValB * smpWgt;
						t_acumG[advDir] += texValG * smpWgt;
						t_acumR[advDir] += texValR * smpWgt;
					}

					///update the step counter and the "current" clip point///
					advcts++;
					clp0_x = clp1_x;
					clp0_y = clp1_y;

					///check if the streamline has gone beyond the flow field///
					if (clp0_x < 0.0f || clp0_x >= (n_xres - 1) || clp0_y < 0.0f || clp0_y >= (n_yres - 1))  break;
				}
			}


			///normalize the accumulated composite texture///
			///clamp the texture value against the displayable intensity range [0, 255]
			///clamp the texture value against the displayable intensity range [0, 255]
			if (!bNoiseImage) {
				texVal = (t_acum[0] + t_acum[1]) / (w_acum[0] + w_acum[1]);
				texVal = (texVal < 0.0f) ? 0.0f : texVal;
				texVal = (texVal > 255.0f) ? 255.0f : texVal;
				pOutputImg->imageData[(pOutputImg->height - j - 1) * pOutputImg->widthStep + i] = (unsigned char)texVal;
			}
			else {
				texValB = (t_acumB[0] + t_acumB[1]) / (w_acum[0] + w_acum[1]);
				texValG = (t_acumG[0] + t_acumG[1]) / (w_acum[0] + w_acum[1]);
				texValR = (t_acumR[0] + t_acumR[1]) / (w_acum[0] + w_acum[1]);
				texValB = (texValB < 0.0f) ? 0.0f : texValB;
				texValG = (texValG < 0.0f) ? 0.0f : texValG;
				texValR = (texValR < 0.0f) ? 0.0f : texValR;
				texValB = (texValB > 255.0f) ? 255.0f : texValB;
				texValG = (texValG > 255.0f) ? 255.0f : texValG;
				texValR = (texValR > 255.0f) ? 255.0f : texValR;
				pOutputImg->imageData[(pOutputImg->height - j - 1) * pOutputImg->widthStep + i * pOutputImg->nChannels + 0] = (unsigned char)texValB;
				pOutputImg->imageData[(pOutputImg->height - j - 1) * pOutputImg->widthStep + i * pOutputImg->nChannels + 1] = (unsigned char)texValG;
				pOutputImg->imageData[(pOutputImg->height - j - 1) * pOutputImg->widthStep + i * pOutputImg->nChannels + 2] = (unsigned char)texValR;
			}


		}
	}
}

/*
///		write the LIC image to a PPM file     ///
void	WriteImage2PPM(int  n_xres,  int  n_yres,  unsigned char*  pImage,  char*  f_name)
{
	FILE*	o_file;
	if(   ( o_file = fopen(f_name, "w") )  ==  NULL   )
	{
		printf("Can't open output file\n");
		return;
	}
	fprintf(o_file, "P6\n%d %d\n255\n", n_xres, n_yres);
	for(int  j = 0;  j < n_yres;  j ++)
		for(int  i = 0;  i < n_xres;  i ++)
		{
			unsigned  char	unchar = pImage[j * n_xres + i];
			fprintf(o_file, "%c%c%c", unchar, unchar, unchar);
		}
		fclose (o_file);	o_file = NULL;
}
*/
