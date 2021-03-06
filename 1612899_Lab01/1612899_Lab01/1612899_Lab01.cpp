#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace std;
using namespace cv;

bool isGrayImage(const Mat &image, float threshold = 0.001)
{
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if (abs(image.at<Vec3b>(y, x)[0] - image.at<Vec3b>(y, x)[1]) <= threshold
				|| abs(image.at<Vec3b>(y, x)[0] - image.at<Vec3b>(y, x)[2]) <= threshold
				|| abs(image.at<Vec3b>(y, x)[1] - image.at<Vec3b>(y, x)[2]) <= threshold)
				return false;
		}
	}
	return true;
}

Mat rbgToGray(const Mat image) {
	Mat gray_image = image.clone();

	// The algorithm linear: Y = B * 0.0722 + G * 0.7152 + R * 0.2126
	// Blue - Green - Red
	for (int y = 0; y < gray_image.rows; y++) {
		for (int x = 0; x < gray_image.cols; x++) {
			float yLinear = gray_image.at<Vec3b>(y, x)[0] * 0.0722 +
				gray_image.at<Vec3b>(y, x)[1] * 0.7152 +
				gray_image.at<Vec3b>(y, x)[2] * 0.2126;

			gray_image.at<Vec3b>(y, x)[0] = yLinear;
			gray_image.at<Vec3b>(y, x)[1] = yLinear;
			gray_image.at<Vec3b>(y, x)[2] = yLinear;
		}
	}
	return gray_image;
}

Mat adjustContract(Mat image, float alpha) {
	/*
		The parameters α>0 is often called the gain parameter;
		this parameter is said to control contrast.
			g(x)=αf(x)
	*/
	Mat new_image = image.clone();
	for (int y = 0; y < new_image.rows; y++) {
		for (int x = 0; x < new_image.cols; x++) {
			for (int c = 0; c < new_image.channels(); c++) {
				new_image.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(alpha*new_image.at<Vec3b>(y, x)[c]);
			}
		}
	}
	return new_image;
}

Mat adjustBrightness(Mat image, float beta) {
	/*
		The parameters β is  often called bias parameter;
		this parameters is said to control brightness.
			g(x)=f(x)+β
	*/
	Mat new_image = image.clone();
	for (int y = 0; y < new_image.rows; y++) {
		for (int x = 0; x < new_image.cols; x++) {
			for (int c = 0; c < new_image.channels(); c++) {
				new_image.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(new_image.at<Vec3b>(y, x)[c] + beta);
			}
		}
	}
	return new_image;
}

Mat toNegativeImage(const Mat& image) {
	Mat negativeImage = image.clone();
	for (int y = 0; y < negativeImage.rows; y++) {
		for (int x = 0; x < negativeImage.cols; x++) {
			negativeImage.at<Vec3b>(y, x)[0] = 255 - negativeImage.at<Vec3b>(y, x)[0];
			negativeImage.at<Vec3b>(y, x)[1] = 255 - negativeImage.at<Vec3b>(y, x)[1];
			negativeImage.at<Vec3b>(y, x)[2] = 255 - negativeImage.at<Vec3b>(y, x)[2];
		}
	}
	return negativeImage;
}

Mat transformByLog(const Mat& image, float constant = 1) {
	Mat new_image = image.clone();
	// newValuePixel = c* Log(currentValuePixel + 1);
	for (int y = 0; y < new_image.rows; y++) {
		for (int x = 0; x < new_image.cols; x++) {
			for (int c = 0; c < new_image.channels(); c++) {
				new_image.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(constant *log2(1 + new_image.at<Vec3b>(y, x)[c]));
			}
		}
	}
	return new_image;
}

Mat transformByGamma(const Mat& image, float fGamma = 1) {
	////pow((float)(pixel/ 255.0), fGamma) * 255.0f	
	uchar* p = new uchar[256];
	for (int i = 0; i < 256; ++i)
		p[i] = saturate_cast<uchar>(pow(i / 255.0, fGamma) * 255.0);

	Mat new_image = image.clone();

	switch (new_image.channels())
	{
	case 1:
	{
		MatIterator_<uchar> it, end;
		for (it = new_image.begin<uchar>(), end = new_image.end<uchar>(); it != end; ++it)
			*it = p[(*it)];
		break;
	}
	case 3:
	{
		MatIterator_<Vec3b> it, end;
		for (it = new_image.begin<Vec3b>(), end = new_image.end<Vec3b>(); it != end; ++it)
		{
			(*it)[0] = p[((*it)[0])];
			(*it)[1] = p[((*it)[1])];
			(*it)[2] = p[((*it)[2])];
		}
		break;
	}
	}

	return new_image;
}

void calColorHistogram(const Mat&image, vector<int> & b_hist, vector<int> & g_hist, vector<int> & r_hist)
{
	/// B - G - R
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			int b_indexBin = image.at<Vec3b>(y, x)[0];
			int g_indexBin = image.at<Vec3b>(y, x)[1];
			int r_indexBin = image.at<Vec3b>(y, x)[2];
			b_hist[b_indexBin]++;
			g_hist[g_indexBin]++;
			r_hist[r_indexBin]++;
		}
	}
}

void calGrayHistogram(const Mat&image, vector<int> & hist)
{
	/// B - G - R
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if (image.channels() == 1)
				hist[image.at<uchar>(y, x)]++;
			else
				hist[image.at<Vec3b>(y, x)[0]]++;
		}
	}
}



Mat drawHistogramColorImage(Mat histImage, const int& histSize, const int&histWeight, const int&histHeight,
	const vector<int>& b_hist, const vector<int>& g_hist, const vector<int>& r_hist)
{
	normalize(b_hist, b_hist, 0, histHeight, NORM_MINMAX, -1, Mat());
	normalize(g_hist, g_hist, 0, histHeight, NORM_MINMAX, -1, Mat());
	normalize(r_hist, r_hist, 0, histHeight, NORM_MINMAX, -1, Mat());

	int bin_w = cvRound((double)histWeight / histSize);
	for (int i = 1; i < histSize; ++i)
	{
		line(histImage, Point(bin_w*(i - 1), histHeight - b_hist[i - 1]),
			Point(bin_w*(i), histHeight - b_hist[i]),
			Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), histHeight - g_hist[i - 1]),
			Point(bin_w*(i), histHeight - (g_hist[i])),
			Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), histHeight - r_hist[i - 1]),
			Point(bin_w*(i), histHeight - (r_hist[i])),
			Scalar(0, 0, 255), 2, 8, 0);
	}
	return histImage;
}

Mat drawHistogramGrayImage(Mat histImage, const int& histSize, const int&histWeight, const int&histHeight,
	const vector<int>& hist)
{
	normalize(hist, hist, 0, histHeight, NORM_MINMAX, -1, Mat());
	int bin_w = cvRound((double)histWeight / histSize);
	for (int i = 1; i < histSize; ++i)
	{
		line(histImage, Point(bin_w*(i - 1), histHeight - hist[i - 1]),
			Point(bin_w*(i), histHeight - hist[i]),
			Scalar(255, 255, 0), 2, 8, 0);
	}
	return histImage;
}


void histogramOfImage(const Mat& image, int histSize = 256, int histWeight = 512, int histHeight = 400)
{

	vector<int> b_hist(histSize, 0);
	vector<int> g_hist(histSize, 0);
	vector<int> r_hist(histSize, 0);
	calColorHistogram(image, b_hist, g_hist, r_hist);

	Mat histImage(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage = drawHistogramColorImage(histImage, histSize, histWeight, histHeight, b_hist, g_hist, r_hist);

	imshow("The histogram ", histImage);
}

void histogramOfGrayImage(const Mat& image, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	vector<int> hist(histSize, 0);
	calGrayHistogram(image, hist);

	Mat histImage(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage = drawHistogramGrayImage(histImage, histSize, histWeight, histHeight, hist);

	imshow("The histogram gray image", histImage);
}


double calDiffHistogram(const int& histSize, const vector<int>& b_hist1, const vector<int>& g_hist1, const vector<int>& r_hist1,
	const vector<int>& b_hist2, const vector<int>& g_hist2, const vector<int>& r_hist2)
{
	double res = 0;
	for (int i = 0; i < histSize; ++i)
	{
		double b = b_hist1[i] - b_hist2[i];
		double g = g_hist1[i] - g_hist2[i];
		double r = r_hist1[i] - r_hist2[i];
		res += (double)pow(b + r + g, 2) / (double)(b_hist1[i] + g_hist1[i] + r_hist1[i]);
	}
	return res;
}

double calDiffHistogramOfGrayImage(const int& histSize, const vector<int>& hist1, const vector<int>& hist2)
{
	double res = 0;
	for (int i = 0; i < histSize; ++i)
	{
		res += (double)pow(hist1[i] - hist2[i], 2) / (double)(hist1[i]);
	}
	return res;
}

void compareHistogramImage(const Mat& image1, const Mat & image2, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	// calculate histogram for image 1 
	vector<int> b_hist1(histSize, 0);
	vector<int> g_hist1(histSize, 0);
	vector<int> r_hist1(histSize, 0);
	calColorHistogram(image1, b_hist1, g_hist1, r_hist1);
	Mat histImage1(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage1 = drawHistogramColorImage(histImage1, histSize, histWeight, histHeight, b_hist1, g_hist1, r_hist1);

	// calculate histogram for image 2
	vector<int> b_hist2(histSize, 0);
	vector<int> g_hist2(histSize, 0);
	vector<int> r_hist2(histSize, 0);
	calColorHistogram(image2, b_hist2, g_hist2, r_hist2);
	Mat histImage2(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage2 = drawHistogramColorImage(histImage2, histSize, histWeight, histHeight, b_hist2, g_hist2, r_hist2);

	double result = calDiffHistogram(histSize, b_hist1, g_hist1, r_hist1, b_hist2, g_hist2, r_hist2);
	cout << "Compare following Chi-Square metric: " << result << endl;
	imshow("The histogram of image 1 ", histImage1);
	imshow("The histogram of image 2", histImage2);

}

void compareHistogramGrayImage(const Mat& image1, const Mat & image2, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	// calculate histogram for image 1 
	vector<int> hist1(histSize, 0);
	calGrayHistogram(image1, hist1);
	Mat histImage1(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage1 = drawHistogramGrayImage(histImage1, histSize, histWeight, histHeight, hist1);

	// calculate histogram for image 2

	vector<int> hist2(histSize, 0);
	calGrayHistogram(image2, hist2);
	Mat histImage2(histHeight, histWeight, CV_8UC3, Scalar(0, 0, 0));
	histImage2 = drawHistogramGrayImage(histImage2, histSize, histWeight, histHeight, hist2);

	double result = calDiffHistogramOfGrayImage(histSize, hist1, hist2);
	cout << "Compare following Chi-Square metric: " << result << endl;
	imshow("The histogram of gray image 1 ", histImage1);
	imshow("The histogram of gray image 2", histImage2);

}

float pixelRound(int value, int histSize)
{
	return round(value*(histSize / 256))*(256 / histSize);
}

Mat quantizationIamge(const Mat & image, int histSize = 256)
{
	/// B - G - R
	Mat newImage = image.clone();
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if (image.channels() == 1)
			{
				newImage.at<uchar>(y, x) = pixelRound(image.at<uchar>(y, x), histSize);
			}
			else
			{
				newImage.at<Vec3b>(y, x)[0] = pixelRound(image.at<Vec3b>(y, x)[0], histSize);
				newImage.at<Vec3b>(y, x)[1] = pixelRound(image.at<Vec3b>(y, x)[1], histSize);
				newImage.at<Vec3b>(y, x)[2] = pixelRound(image.at<Vec3b>(y, x)[2], histSize);
			}
		}
	}
	return newImage;
}

void histogramQuantizationColorImage(const Mat& image, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	Mat newImage = quantizationIamge(image, histSize);
	imshow("After quantization: ", newImage);
	//histogramOfImage(image, histSize, histWeight, histHeight);
}

void compareHistogramQuantization(const Mat& image1, const Mat & image2, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	Mat newImage1 = quantizationIamge(image1, histSize);
	Mat newImage2 = quantizationIamge(image2, histSize);
	compareHistogramImage(newImage1, newImage2, histSize, histWeight, histHeight);
}

void histogramQuantizationGrayImage(const Mat&image, int histSize = 256, int histWeight = 512, int histHeight = 400)
{
	Mat grayImage = image;
	if (grayImage.channels() != 1)
	{
		grayImage = rbgToGray(grayImage);
	}

	quantizationIamge(grayImage);
	histogramOfGrayImage(image, histSize, histWeight, histHeight);
}



int main(int argc, char** argv)
{
	cout << "Hello! 1612899" << endl;

	if (argc < 2) {
		cout << "The program can not open and show image." << endl;
		return -1;
	}

	//const char * fname = "C:\\Users\\Truong Hoang\\Pictures\\dora.jpg";

	char * fname = argv[1];										// Read file name from command line
	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	imshow("The original image", image);						//	Show image in screen 
	
	if (strcmp(argv[2], "--g") == 0) {
		if (isGrayImage(image))
		{
			cout << "only color image can use this function." << endl;
		}
		imshow("color to gray image", rbgToGray(image));
	}
	else if (strcmp(argv[2], "--c") == 0) {
		float alpha = atof(argv[3]);
		imshow("constract image", adjustContract(image, alpha));
	}
	else if (strcmp(argv[2], "--b") == 0) {
		float beta = atof(argv[3]);
		imshow("brightness image", adjustBrightness(image, beta));
	}
	else if (strcmp(argv[2], "--n") == 0) {
		imshow("negative image", toNegativeImage(image));
	}
	else if (strcmp(argv[2], "--lt") == 0) {
		float constant = atof(argv[3]);
		imshow("transform by log", transformByLog(image, constant));
	}
	else if (strcmp(argv[2], "--gt") == 0) {
		float fgamma = atof(argv[3]);
		imshow("transform by gamma", transformByGamma(image, fgamma));
	}
	else if (strcmp(argv[2], "--hi") == 0) {
		histogramOfImage(image);
	}
	else if (strcmp(argv[2], "--cmphi") == 0)
	{
		if (argc > 3)
		{
			const char * fname2 = argv[3];
			Mat image2 = imread(fname2, IMREAD_COLOR);
			if (image2.empty())
			{
				cout << "Can not open image!" << endl;
				return -1;
			}
			imshow("The 2nd image", image2);
			compareHistogramImage(image, image2);
		}
		else cout << "Can not open image 2. Make right path for image 2" << endl;
	}
	else if (strcmp(argv[2], "--hiqc") == 0) {
		int histSize = atoi(argv[3]);
		histogramQuantizationColorImage(image, histSize);
	}
	else if (strcmp(argv[2], "--hiqg") == 0) {
		int histSize = atoi(argv[3]);
		histogramQuantizationGrayImage(image, histSize);
	}
	else if (strcmp(argv[2], "--cphiqc") == 0)
	{
		if (argc > 3)
		{
			const char * fname2 = argv[3];
			int histSize = atoi(argv[4]);
			Mat image2 = imread(fname2, IMREAD_COLOR);
			if (image2.empty())
			{
				cout << "Can not open image!" << endl;
				return -1;
			}
			imshow("The 2nd image", image2);

			if (image2.channels() == 1 && image.channels() == 1)
			{
				compareHistogramGrayImage(image, image2, histSize);
			}
			else if (image2.channels() == 3 && image.channels() == 3)
			{
				compareHistogramQuantization(image, image2, histSize);
			}
			else cout << "Two images are not same channel." << endl;
		}
		else cout << "Can not open image 2. Make right path for image 2" << endl;
	}

	waitKey(0);
	return 0;

}
