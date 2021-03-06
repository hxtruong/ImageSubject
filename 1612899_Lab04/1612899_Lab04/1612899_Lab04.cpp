// 1612899_Lab04.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace std;
using namespace cv;
// Blue - Green - Red

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

vector<vector<float>> gaussianKernelArray(int height, int width, float sigma)
{
	double PI = atan(1) * 4;
	vector<vector<float>> kernelArr(height, vector<float>(width));
	double sum = 0;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			kernelArr[i][j] = exp(-(i*i + j * j) / (2 * sigma*sigma)) / (2 * PI*sigma*sigma);
			sum += kernelArr[i][j];
		}
	}
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			kernelArr[i][j] /= sum;
		}
	}
	return kernelArr;
}

void GausianFilter(const Mat& originImage, Mat &desImage, vector<vector<float>> kernelArray) {
	//Assian Gaussian Blur value of the center point.Repeating this process for all other points through image
	int kerelHeight = kernelArray.size();
	int kerelWidth = kernelArray[0].size();
	int newImageHeight = originImage.rows - kerelHeight + 1;
	int newImageWidth = originImage.cols - kerelWidth + 1;
	if (originImage.channels() == 1) {
		for (int i = 0; i < newImageHeight; i++) {
			for (int j = 0; j < newImageWidth; j++) {
				double value = 0;
				for (int h = i; h < i + kerelHeight; h++) {
					for (int w = j; w < j + kerelWidth; w++) {
						value += kernelArray[h - i][w - j] * originImage.at<uchar>(h, w);
					}
				}
				desImage.at<uchar>(i, j) = saturate_cast<uchar>(value);
			}
		}
	}
	else {
		for (int chanel = 0; chanel < 3; chanel++) {
			for (int i = 0; i < newImageHeight; i++) {
				for (int j = 0; j < newImageWidth; j++) {
					double value = 0;
					for (int h = i; h < i + kerelHeight; h++) {
						for (int w = j; w < j + kerelWidth; w++) {
							value += kernelArray[h - i][w - j] * originImage.at<Vec3b>(h, w)[chanel];
						}
					}
					desImage.at<Vec3b>(i, j)[chanel] = saturate_cast<uchar>(value);
				}
			}
		}
	}

}


void MedianFilter(const Mat& originImage, Mat &desImage, int height = 3, int width = 3) {

	int newImageHeight = originImage.rows - height + 1;
	int newImageWidth = originImage.cols - width + 1;
	if (originImage.channels() == 1) {
		for (int i = 0; i < newImageHeight; i++) {
			for (int j = 0; j < newImageWidth; j++) {
				vector<uchar> arr;
				for (int h = i; h < i + height; h++) {
					for (int w = j; w < j + width; w++) {
						arr.push_back(originImage.at<uchar>(h, w));
					}
				}
				sort(arr.begin(), arr.end());
				desImage.at<uchar>(i, j) = arr[(arr.size() - 1) / 2];
			}
		}
	}
	else {
		for (int chanel = 0; chanel < 3; chanel++) {
			for (int i = 0; i < newImageHeight; i++) {
				for (int j = 0; j < newImageWidth; j++) {
					vector<uchar> arr;
					for (int h = i; h < i + height; h++) {
						for (int w = j; w < j + width; w++) {
							arr.push_back(originImage.at<Vec3b>(h, w)[chanel]);
						}
					}
					sort(arr.begin(), arr.end());
					desImage.at<Vec3b>(i, j)[chanel] = arr[(arr.size() - 1) / 2];
				}
			}
		}
	}
}

void MeanFilter(const Mat& originImage, Mat &desImage, int height = 3, int width = 3) {
	int kHeight = (height - 1) / 2;
	int kWidth = (width - 1) / 2;
	int newImageHeight = originImage.rows - kHeight + 1;
	int newImageWidth = originImage.cols - kWidth + 1;

	if (originImage.channels() == 1) {
		for (int i = kHeight; i < newImageHeight; i++) {
			for (int j = kWidth; j < newImageWidth; j++) {
				long sum = 0;
				for (int h = i - kHeight; h < i + kHeight; h++) {
					for (int w = j - kWidth; w < j + kWidth; w++) {
						sum += originImage.at<uchar>(h, w);
					}
				}
				desImage.at<uchar>(i, j) = saturate_cast<uchar>(sum / (height*width));
			}
		}
	}
	else {
		for (int chanel = 0; chanel < 3; chanel++) {
			for (int i = kHeight; i < newImageHeight; i++) {
				for (int j = kWidth; j < newImageWidth; j++) {
					long sum = 0;
					for (int h = i - kHeight; h < i + kHeight; h++) {
						for (int w = j - kWidth; w < j + kWidth; w++) {
							sum += originImage.at<Vec3b>(h, w)[chanel];
						}
					}
					desImage.at<Vec3b>(i, j)[chanel] = saturate_cast<uchar>(sum / (height*width));
				}
			}
		}
	}
}



int main(int argc, char** argv)
{
	char * fname = argv[1];										// Read file name from command line
	//const char *fname = "C:\\Users\\hxtruong\\Pictures\\dora.jpg";
	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	/*imshow("The original image", image);		*/				//	Show image in screen 

	if (argc < 3)
	{
		cout << "Not enough parameter." << endl;
		return 0;
	}
	if (strcmp(argv[2], "--mg") == 0)
	{
		cvtColor(image, image, CV_BGR2GRAY);
		imshow("The original image", image);

		if (argc < 5) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);

		Mat desImage = image.clone();
		MeanFilter(image, desImage, height, width);
		imshow("Mean filter gray image", desImage);
	}
	else if (strcmp(argv[2], "--mc") == 0)
	{
		imshow("The original image", image);

		if (argc < 5) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);

		Mat desImage = image.clone();
		MeanFilter(image, desImage, height, width);
		imshow("Mean filter color image", desImage);
	}
	else if (strcmp(argv[2], "--meg") == 0)
	{
		cvtColor(image, image, CV_BGR2GRAY);
		imshow("The original image", image);

		if (argc < 5) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);
		Mat desImage = image.clone();
		MedianFilter(image, desImage, height, width);
		imshow("Median filter gray image", desImage);
	}
	else if (strcmp(argv[2], "--mec") == 0)
	{
		imshow("The original image", image);

		if (argc < 5) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);
		Mat desImage = image.clone();
		MeanFilter(image, desImage, height, width);
		imshow("Median filter color image", desImage);
	}
	else if (strcmp(argv[2], "--gg") == 0)
	{
		cvtColor(image, image, CV_BGR2GRAY);
		imshow("The original image", image);

		if (argc < 6) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);
		float sigma = atof(argv[5]);
		Mat desImage = image.clone();
		vector<vector<float>> kernelArray = gaussianKernelArray(height, width, sigma);
		GausianFilter(image, desImage, kernelArray);
		imshow("Gaussian filter gray image", desImage);
	}
	else if (strcmp(argv[2], "--gc") == 0)
	{
		imshow("The original image", image);
		if (argc < 6) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		int height = atoi(argv[3]);
		int width = atoi(argv[4]);
		float sigma = atof(argv[5]);
		Mat desImage = image.clone();
		vector<vector<float>> kernelArray = gaussianKernelArray(height, width, sigma);
		GausianFilter(image, desImage, kernelArray);
		imshow("Gaussian filter color image", desImage);
	}

	waitKey(0);
	return 0;
}