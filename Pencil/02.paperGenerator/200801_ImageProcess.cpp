
#include "200801_ImageProcess.h"

Mat ImgProcess::gaussian_blur(const Mat& src, int ksize, float sigma) {
	Mat result = src.clone();
	double exp = 0.f;
	vector<vector<double>> gaussian_kernel;
	gaussian_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 0.f));

	for (int i = 0; i < 2 * ksize + 1; i++) {
		for (int j = 0; j < 2 * ksize + 1; j++) {
			exp = -((pow(ksize - i, 2) + pow(ksize - j, 2)) / (2.f * sigma * sigma));
			gaussian_kernel[i][j] = (1 / (6.283184 * (2.f * sigma * sigma))) * pow(2.718282, exp);
		}
	}

	int height = src.rows;
	int width = src.cols;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			double blur = 0.f;
			double B_blur = 0.f;
			double G_blur = 0.f;
			double R_blur = 0.f;

			double sum = 0.f;

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (src.channels() == 3) {
						B_blur += (src.at<Vec3b>(r, c)[0] * gaussian_kernel[i][j]);
						G_blur += (src.at<Vec3b>(r, c)[1] * gaussian_kernel[i][j]);
						R_blur += (src.at<Vec3b>(r, c)[2] * gaussian_kernel[i][j]);
					}
					else if (src.channels() == 1)
						blur += (src.at<unsigned char>(r, c) * gaussian_kernel[i][j]);

					sum += gaussian_kernel[i][j];
				}
			}

			if (src.channels() == 3) {
				result.at<Vec3b>(h, w)[0] = (B_blur / sum);
				result.at<Vec3b>(h, w)[1] = (G_blur / sum);
				result.at<Vec3b>(h, w)[2] = (R_blur / sum);
			}
			else if (src.channels() == 1)
				result.at<unsigned char>(h, w) = (unsigned char)(blur / sum);
		}
	}

	return result;
}

Mat ImgProcess::bilateral_filter(const Mat& src, const Mat& colRef_src, int ksize, double dist_sigma, double color_sigma) {
	Mat result = src.clone();

	long double exp = 0.f;
	vector<vector<double>> gaussian_kernel;
	vector<vector<double>> bilateral_kernel;
	gaussian_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 0.f));

	for (int i = 0; i < 2 * ksize + 1; i++) {
		for (int j = 0; j < 2 * ksize + 1; j++) {
			exp = -((pow(ksize - i, 2) + pow(ksize - j, 2)) / (2.f * dist_sigma * dist_sigma));
			gaussian_kernel[i][j] = (1 / (6.283184 * (2.f * dist_sigma * dist_sigma))) * pow(2.718282, exp);
		}
	}

	int height = src.rows;
	int width = src.cols;

	parallel_for(int(0), height, [&](int h) {

		//for (int h = 0; h < height; h++) {
		vector<vector<double>> bilateral_kernel;
		for (int w = 0; w < width; w++) {

			bilateral_kernel.clear();
			bilateral_kernel.assign(2 * ksize + 1, vector<double>(2 * ksize + 1, 1.f));

			long double blur = 0.f;
			long double B_blur = 0.f;
			long double G_blur = 0.f;
			long double R_blur = 0.f;

			int center_B, center_G, center_R, center_C;
			int new_B, new_G, new_R, new_C;

			long double weight = 0.f;
			long double color_diff = 0.f;

			if (colRef_src.channels() == 3) {
				center_B = colRef_src.at<Vec3b>(h, w)[0];
				center_G = colRef_src.at<Vec3b>(h, w)[1];
				center_R = colRef_src.at<Vec3b>(h, w)[2];
			}
			else
				center_C = colRef_src.at<unsigned char>(h, w);

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (colRef_src.channels() == 3) {
						new_B = colRef_src.at<Vec3b>(r, c)[0];
						new_G = colRef_src.at<Vec3b>(r, c)[1];
						new_R = colRef_src.at<Vec3b>(r, c)[2];
						color_diff = sqrt((new_B - center_B) * (new_B - center_B) + (new_G - center_G) * (new_G - center_G) + (new_R - center_R) * (new_R - center_R));
					}
					else {
						new_C = colRef_src.at<unsigned char>(r, c);
						color_diff = sqrt((new_C - center_C) * (new_C - center_C));
					}

					exp = -((pow(color_diff, 2)) / (2.f * color_sigma * color_sigma));
					bilateral_kernel[i][j] = (1 / (6.283184 * (2.f * color_sigma * color_sigma))) * pow(2.718282, exp);
				}
			}

			for (int i = 0; i < 2 * ksize + 1; i++) {
				for (int j = 0; j < 2 * ksize + 1; j++) {
					int r = h - ksize + j;
					int c = w - ksize + i;

					if (r < 0 || r >= height || c < 0 || c >= width)
						continue;

					if (src.channels() == 3) {
						B_blur += (src.at<Vec3b>(r, c)[0] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
						G_blur += (src.at<Vec3b>(r, c)[1] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
						R_blur += (src.at<Vec3b>(r, c)[2] * gaussian_kernel[i][j] * bilateral_kernel[i][j]);
					}
					else if (src.channels() == 1)
						blur += (src.at<unsigned char>(r, c) * gaussian_kernel[i][j] * bilateral_kernel[i][j]);

					weight += gaussian_kernel[i][j] * bilateral_kernel[i][j];
				}
			}
			if (weight != 0.f) {
				if (src.channels() == 3) {
					result.at<Vec3b>(h, w)[0] = (unsigned char)(B_blur / weight);
					result.at<Vec3b>(h, w)[1] = (unsigned char)(G_blur / weight);
					result.at<Vec3b>(h, w)[2] = (unsigned char)(R_blur / weight);
				}
				else if (src.channels() == 1)
					result.at<unsigned char>(h, w) = (unsigned char)(blur / weight);
			}
		}
		});

	return result;
}

Mat ImgProcess::alpha_blending(const Mat& src1, const Mat& src2, int alpha) {
	Mat result = src1.clone();

	int height = result.rows;
	int width = result.cols;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {

			if (src1.channels() == 3 && src2.channels() == 3) {
				result.at<Vec3b>(h, w)[0] = src2.at<Vec3b>(h, w)[0] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[0] * alpha / 100;
				result.at<Vec3b>(h, w)[1] = src2.at<Vec3b>(h, w)[1] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[1] * alpha / 100;
				result.at<Vec3b>(h, w)[2] = src2.at<Vec3b>(h, w)[2] * (100 - alpha) / 100 + src1.at<Vec3b>(h, w)[2] * alpha / 100;
			}
			else if (src1.channels() == 1 && src2.channels() == 1)
				result.at<unsigned char>(h, w) = src2.at<unsigned char>(h, w) * (100 - alpha) / 100 + src1.at<unsigned char>(h, w) * alpha / 100;
		}
	}

	return result;
}