// 1612899_Lab02.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include <ctime>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"


#define HIST_W 256

using namespace std;
using namespace cv;

void printArr(vector<int> arr, int n = HIST_W)
{
	for (int i = 0; i < n; ++i)
	{
		cout << arr[i] << " ";
	}
	cout << endl << "------------" << endl;
}

void rbgToYCbCr(const Mat& originImage, Mat & desImage)
{
	// convert RBG image to YCbCr image 
	for (int y = 0; y < originImage.rows; ++y)
	{
		for (int x = 0; x < originImage.cols; ++x)
		{
			float b = originImage.at<Vec3b>(y, x)[0];
			float g = originImage.at<Vec3b>(y, x)[1];
			float r = originImage.at<Vec3b>(y, x)[2];

			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(0.299*r + 0.587*g + 0.114*b);
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(128 - 0.168736*r - 0.331264*g + 0.5*b);
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar> (128 + 0.5*r - 0.418688*g - 0.081312*b);
		}
	}
}

void yCbCrToRBG(const Mat& originImage, Mat& desImage)
{
	// convert YCbCr image to RBG image 
	for (int y = 0; y < originImage.rows; ++y)
	{
		for (int x = 0; x < originImage.cols; ++x)
		{
			float Y = originImage.at<Vec3b>(y, x)[0];
			float Cb = originImage.at<Vec3b>(y, x)[1];
			float Cr = originImage.at<Vec3b>(y, x)[2];

			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(Y + 1.772*(Cb - 128));
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(Y - 0.344136*(Cb - 128) - 0.714136*(Cr - 128));
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(Y + 1.402*(Cr - 128));
		}
	}
}

void rbgToGray(const Mat& originImage, Mat & desImage) {
	// The algorithm linear: Y = B * 0.0722 + G * 0.7152 + R * 0.2126
	// Blue - Green - Red
	for (int y = 0; y < originImage.rows; y++) {
		for (int x = 0; x < originImage.cols; x++) {
			float yLinear = originImage.at<Vec3b>(y, x)[0] * 0.0722 +
				originImage.at<Vec3b>(y, x)[1] * 0.7152 +
				originImage.at<Vec3b>(y, x)[2] * 0.2126;

			if (desImage.channels() == 1)
			{
				desImage.at<uchar>(y, x) = yLinear;
			}
			else
			{
				desImage.at<Vec3b>(y, x)[0] = yLinear;
				desImage.at<Vec3b>(y, x)[1] = yLinear;
				desImage.at<Vec3b>(y, x)[2] = yLinear;
			}
		}
	}
}

void calculateHist(const Mat& image, vector<int>& histogram)
{
	// calculate the no of pixels for each intensity values
	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			if (image.channels() == 0)
			{

				histogram[(int)image.at<uchar>(y, x)]++;
			}
			else
			{
				histogram[(int)image.at<Vec3b>(y, x)[0]]++;
			}
		}
	}
	printArr(histogram);
}

void equalizationHistogram(int size, const vector<int>& histogram, vector<int> & eHistogram)
{
	// equalization a vector with binW and size
	long sumHistogram = 0;
	float alpha = ((float)(HIST_W - 1) / (float)size);
	for (int i = 1; i < histogram.size(); ++i)
	{
		sumHistogram += histogram[i];
		//cout << sumHistogram << " ";
		eHistogram[i] = saturate_cast<uchar>(alpha * sumHistogram);
	}
	cout << "\n*****Scale " << alpha << " ***SUM: " << sumHistogram << "*****SIze: " << size << endl;
}

void equalizationGrayImage(const Mat& originImage, Mat & desImage)
{
	vector<int> histogram(HIST_W, 0);
	calculateHist(originImage, histogram);

	vector<int> eHistogram(HIST_W, 0);
	equalizationHistogram(originImage.rows*originImage.cols, histogram, eHistogram);
	//printArr(eHistogram);

	for (int y = 0; y < originImage.rows; y++)
	{
		for (int x = 0; x < originImage.cols; x++)
		{
			if (originImage.channels() == 1)
			{
				desImage.at<uchar>(y, x) = saturate_cast<uchar>(eHistogram[originImage.at<uchar>(y, x)]);
			}
			else
			{
				desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(eHistogram[originImage.at<Vec3b>(y, x)[0]]);
				desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(eHistogram[originImage.at<Vec3b>(y, x)[1]]);
				desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(eHistogram[originImage.at<Vec3b>(y, x)[2]]);
			}
		}
	}
	//imshow("The equalization gray image", desImage);
}

void equalizationColorImage(const Mat& originImage, Mat & desImage)
{
	Mat newImage = originImage.clone();
	rbgToYCbCr(originImage, newImage);

	vector<int> histogram(HIST_W, 0);
	calculateHist(newImage, histogram);
	//printArr(histogram);

	vector<int> eHistogram(HIST_W, 0);
	equalizationHistogram(newImage.rows*newImage.cols, histogram, eHistogram);

	for (int y = 0; y < newImage.rows; y++)
		for (int x = 0; x < newImage.cols; x++)
		{
			newImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(eHistogram[newImage.at<Vec3b>(y, x)[0]]);
		}

	yCbCrToRBG(newImage, desImage);
}

void rbgPixelToHsvPixel(float b, float g, float r, float& h, float& s, float& v) {
	float minP, maxP, delta;
	b = b / 256;
	g = g / 256;
	r = r / 256;
	minP = min(b, min(g, r));
	maxP = max(b, max(g, r));

	v = maxP;
	delta = maxP - minP;
	if (maxP != 0) {
		s = delta / maxP;
	}
	else {
		// r = g =b =0
		s = 0;
		h = -1;
		return;
	}
	if (r == maxP) {
		h = (g - b) / delta;
	}
	else if (g == maxP) {
		h = 2 + (b - r) / delta;
	}
	else {
		h = 4 + (r - g) / delta;
	}
	h *= 60;
	if (h < 0) {
		h += 360;
	}
}

void hsvPixelToRbgPixel(const float& h, const float& s, const float& v, float& b, float& g, float& r) {
	if (s <= 0.0) {
		r = v;		g = v;		b = v;
		return;
	}
	float hh = h;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	long i = (long)hh;
	float ff = hh - i;
	float p = v * (1.0 - s);
	float q = v * (1.0 - (s * ff));
	float t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		r = v;		g = t;		b = p;
		break;
	case 1:
		r = q;		g = v;		b = p;
		break;
	case 2:
		r = p;		g = v;		b = t;
		break;

	case 3:
		r = p;		g = q;		b = v;
		break;
	case 4:
		r = t;		g = p;		b = v;
		break;
	case 5:
	default:
		r = v;		g = p;		b = q;
		break;
	}
}

void rbgToHsv(const Mat& originImage, Mat& desImage) {
	for (int y = 0; y < originImage.rows; y++)
		for (int x = 0; x < originImage.cols; x++)
		{
			float b = originImage.at<Vec3b>(y, x)[0];
			float g = originImage.at<Vec3b>(y, x)[1];
			float r = originImage.at<Vec3b>(y, x)[2];
			float h, s, v;

			rbgPixelToHsvPixel(b, g, r, h, s, v);
			// h = h/2; s = s*255; v = v* 255 to suitable with [0,255]
			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(h / 2);
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(255 * s);
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(255 * v);
		}
}

void hsvToRbg(const Mat& originImage, Mat& desImage) {
	for (int y = 0; y < originImage.rows; y++)
		for (int x = 0; x < originImage.cols; x++)
		{
			float h = originImage.at<Vec3b>(y, x)[0];
			float s = originImage.at<Vec3b>(y, x)[1];
			float v = originImage.at<Vec3b>(y, x)[2];
			float b, g, r;

			hsvPixelToRbgPixel(h * 2, s / 255, v / 255, b, g, r);
			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(b * 255);
			desImage.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(g * 255);
			desImage.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(r * 255);
		}
}

void equalizationHsvImage(const Mat& originImage, Mat & desImage)
{
	vector<int> histogram(HIST_W, 0);
	calculateHist(originImage, histogram);
	//printArr(histogram);

	vector<int> eHistogram(HIST_W, 0);
	equalizationHistogram(originImage.rows*originImage.cols, histogram, eHistogram);

	for (int y = 0; y < originImage.rows; y++)
		for (int x = 0; x < originImage.cols; x++)
		{
			desImage.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(eHistogram[originImage.at<Vec3b>(y, x)[0]]);
		}
}

int main(int argc, char** argv)
{
	char * fname = argv[1];										// Read file name from command line
	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	imshow("The original image", image);						//	Show image in screen 

	if (argc < 3)
	{
		cout << "Not enough parameter." << endl;
		return 0;
	}
	if (strcmp(argv[2], "--hqgray") == 0)
	{
		// equalization gray image
		const clock_t begin_time = clock();
		Mat grayImage = image.clone();
		if (grayImage.channels() == 3)
		{
			rbgToGray(image, grayImage);
			//imshow("Gray image", grayImage);
		}
		Mat equaImage = grayImage.clone();
		equalizationGrayImage(grayImage, equaImage);
		imshow("The equalization gray image", equaImage);
		cout << "Time to run in manual algorithm: " << float(clock() - begin_time) / CLOCKS_PER_SEC;

		/*
		// Opencv equalization gray image
		const clock_t begin_time = clock();
		cvtColor(image, image, COLOR_BGR2GRAY);
		//equalize the histogram
		Mat hist_equalized_image;
		equalizeHist(image, hist_equalized_image);
		imshow("The equalization gray image by Open CV", hist_equalized_image);
		cout<<"Time to run in opencv: " << float(clock() - begin_time) / CLOCKS_PER_SEC;
		*/
	}
	else if (strcmp(argv[2], "--hqrgb") == 0)
	{
		// equalization color image
		const clock_t begin_time = clock();
		Mat equaImage = image.clone();
		equalizationColorImage(image, equaImage);
		imshow("The equalization color image", equaImage);
		cout << "Time to run in manual algorithm: " << float(clock() - begin_time) / CLOCKS_PER_SEC;
		/*
		// equalization color image by open cv
		const clock_t begin_time = clock();
		Mat hist_equalized_image;
		cvtColor(image, hist_equalized_image, COLOR_BGR2YCrCb);

		//Split the image into 3 channels; Y, Cr and Cb channels respectively and store it in a std::vector
		vector<Mat> vec_channels;
		split(hist_equalized_image, vec_channels);

		//Equalize the histogram of only the Y channel
		equalizeHist(vec_channels[0], vec_channels[0]);

		//Merge 3 channels in the vector to form the color image in YCrCB color space.
		merge(vec_channels, hist_equalized_image);

		//Convert the histogram equalized image from YCrCb to BGR color space again
		cvtColor(hist_equalized_image, hist_equalized_image, COLOR_YCrCb2BGR);
		imshow("The equalization color image", hist_equalized_image);
		cout << "Time to run in open cv: " << float(clock() - begin_time) / CLOCKS_PER_SEC;
		*/
	}
	else if (strcmp(argv[2], "--hqhsv") == 0)
	{
		// rbg to hsv then equalization -> hsv to rbg
		const clock_t begin_time = clock();
		Mat hsvImage = image.clone();
		rbgToHsv(image, hsvImage);
		//imshow("RBG To HSV ", hsvImage);
		//equalization
		equalizationHsvImage(hsvImage, hsvImage);
		Mat rbgImage = image.clone();
		hsvToRbg(hsvImage, rbgImage);
		imshow("HSV To RBG after equalization", rbgImage);
		cout << "Time to run in manual algorithm: " << float(clock() - begin_time) / CLOCKS_PER_SEC;

		/*
		const clock_t begin_time = clock();
		Mat cvHsvImage = image.clone();
		cvtColor(image, cvHsvImage, COLOR_BGR2HSV);
		//equalizationHsvImage(cvHsvImage, cvHsvImage);
		vector<Mat> vec_channels;
		split(cvHsvImage, vec_channels);
		//Equalize the histogram of only the H channel
		equalizeHist(vec_channels[0], vec_channels[0]);
		//Merge 3 channels in the vector to form the color image in HSV color space.
		merge(vec_channels, cvHsvImage);
		Mat cvRbgImage = image.clone();
		cvtColor(cvHsvImage, cvRbgImage, COLOR_HSV2BGR);
		imshow("Open cv HSV to RBG", cvRbgImage);
		cout<<"Time to run in opencv: " << float(clock() - begin_time) / CLOCKS_PER_SEC;
		*/
	}
	waitKey(0);
	return 0;
}

