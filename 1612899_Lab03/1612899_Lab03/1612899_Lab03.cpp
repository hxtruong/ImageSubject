// 1612899_Lab03.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#define PI 3.14159265358979f

using namespace std;
using namespace cv;

float ValueInterpolation(float a, float b, float p00, float p10, float p01, float p11)
{
	return (1 - a) * (1 - b) * p00
		+ a * (1 - b) * p10
		+ b * (1 - a) * p01
		+ a * b * p11;
}

void ScaleInterpolation(const Mat& originImage, Mat& desImage, int rowScale, int colScale)
{
	for (int y = 0; y < desImage.rows; y++) {
		for (int x = 0; x < desImage.cols; x++) {
			float xs = (float)x / (float)colScale;
			float ys = (float)y / (float)rowScale;
			int l = trunc(xs);
			int k = trunc(ys);
			float a = xs - l;
			float b = ys - k;

			Vec3b pixel00, pixel01, pixel10, pixel11;
			pixel00 = originImage.at<Vec3b>(k, l);
			if (l + 1 >= originImage.cols && k + 1 >= originImage.rows)
			{
				pixel11 = originImage.at<Vec3b>(k, l);
			}
			else if (l + 1 >= originImage.cols)
			{
				pixel10 = originImage.at<Vec3b>(k, l);
				pixel11 = originImage.at<Vec3b>(k + 1, l);
			}
			else if (k + 1 >= originImage.rows)
			{
				pixel01 = originImage.at<Vec3b>(k, l);
				pixel11 = originImage.at<Vec3b>(k, l + 1);
			}
			else
			{
				pixel01 = originImage.at<Vec3b>(k + 1, l);
				pixel10 = originImage.at<Vec3b>(k, l + 1);
				pixel11 = originImage.at<Vec3b>(k + 1, l + 1);
			}

			desImage.at<Vec3b>(y, x) = Vec3b(
				(int)ValueInterpolation(a, b, pixel00[0], pixel10[0], pixel01[0], pixel11[0]),
				(int)ValueInterpolation(a, b, pixel00[1], pixel10[1], pixel01[1], pixel11[1]),
				(int)ValueInterpolation(a, b, pixel00[2], pixel10[2], pixel01[2], pixel11[2])
			);

		}
	}
}

Mat Scale(const Mat& originImage, int rowScale, int colScale)
{
	//Mat desImage = Mat::zeros(originImage.rows*rowScale, originImage.cols*colScale, CV_8U);
	Mat desImage(originImage.rows*rowScale, originImage.cols*colScale, CV_8UC3, Scalar(0, 0, 0));
	// initialize a scale image
	for (int y = 0; y < originImage.rows; y++) {
		for (int x = 0; x < originImage.cols; x++) {
			desImage.at<Vec3b>(y*rowScale, x*colScale) = originImage.at<Vec3b>(y, x);
		}
	}

	// interpolation the image
	ScaleInterpolation(originImage, desImage, rowScale, colScale);
	return desImage;
}



void RotationInterpolation(const Mat& originImage, Mat& desImage, float angle, int additionWidth = 0)
{
	float alpha = angle * (PI / 180);

	// Get pivot point of image
	int xPivot = originImage.cols / 2 - 1;
	int yPivot = originImage.rows / 2 - 1;
	Mat resImage(desImage.rows, desImage.cols, CV_8UC3, Scalar(0, 0, 0));
	for (int y = 0; y < desImage.rows; y++) {
		for (int x = 0; x < desImage.cols; x++) {
			float xr = (x - xPivot)*cos(alpha) + (y - yPivot)*sin(alpha) + xPivot - additionWidth / 2;
			float yr = -(x - xPivot)*sin(alpha) + (y - yPivot)*cos(alpha) + yPivot - additionWidth / 2;

			if (xr < 0 || yr < 0) continue;

			int l = trunc(xr);
			int k = trunc(yr);
			float a = xr - l;
			float b = yr - k;

			Vec3b pixel00, pixel01, pixel10, pixel11;
			if (k >= originImage.rows || l >= originImage.cols) continue;
			pixel00 = originImage.at<Vec3b>(k, l);
			if (l + 1 >= originImage.cols && k + 1 >= originImage.rows)
			{
				pixel11 = originImage.at<Vec3b>(k, l);
			}
			else if (l + 1 >= originImage.cols)
			{
				pixel10 = originImage.at<Vec3b>(k, l);
				pixel11 = originImage.at<Vec3b>(k + 1, l);
			}
			else if (k + 1 >= originImage.rows)
			{
				pixel01 = originImage.at<Vec3b>(k, l);
				pixel11 = originImage.at<Vec3b>(k, l + 1);
			}
			else
			{
				pixel01 = originImage.at<Vec3b>(k + 1, l);
				pixel10 = originImage.at<Vec3b>(k, l + 1);
				pixel11 = originImage.at<Vec3b>(k + 1, l + 1);
			}

			resImage.at<Vec3b>(y, x) = Vec3b(
				(int)ValueInterpolation(a, b, pixel00[0], pixel10[0], pixel01[0], pixel11[0]),
				(int)ValueInterpolation(a, b, pixel00[1], pixel10[1], pixel01[1], pixel11[1]),
				(int)ValueInterpolation(a, b, pixel00[2], pixel10[2], pixel01[2], pixel11[2])
			);

		}
	}
	desImage = resImage;
}


Mat Rotation(const Mat& originImage, float angle) {
	float alpha = angle * (PI / 180);
	int xPivot = originImage.cols / 2 - 1;
	int yPivot = originImage.rows / 2 - 1;
	Mat desImage(originImage.rows, originImage.cols, CV_8UC3, Scalar(0, 0, 0));
	for (int y = 0; y < originImage.rows; y++) {
		for (int x = 0; x < originImage.cols; x++) {
			int xr = xPivot + cos(alpha)*(x - xPivot) - sin(alpha)*(y - yPivot);
			int yr = yPivot + sin(alpha)*(x - xPivot) + cos(alpha)*(y - yPivot);
			if (xr >= originImage.cols || yr >= originImage.rows
				|| xr < 0 || yr < 0) continue;
			desImage.at<Vec3b>(yr, xr) = originImage.at<Vec3b>(y, x);
		}
	}

	// interpolation the rotation image;
	RotationInterpolation(originImage, desImage, angle);

	return desImage;

}


Mat RotationN(const Mat& originImage, float angle) {
	float alpha = angle * (PI / 180);

	// Get pivot point of image
	int xPivot = originImage.cols / 2 - 1;
	int yPivot = originImage.rows / 2 - 1;
	int additionWidth = round(sqrt(xPivot*xPivot + yPivot * yPivot));
	Mat desImage(originImage.rows + additionWidth, originImage.cols + additionWidth, CV_8UC3, Scalar(0, 0, 0));
	for (int y = 0; y < originImage.rows; y++) {
		for (int x = 0; x < originImage.cols; x++) {
			int xr = xPivot + cos(alpha)*(x - xPivot) - sin(alpha)*(y - yPivot) + additionWidth / 2;
			int yr = yPivot + sin(alpha)*(x - xPivot) + cos(alpha)*(y - yPivot) + additionWidth / 2;
			if (xr >= desImage.cols || yr >= desImage.rows
				|| xr < 0 || yr < 0) continue;
			desImage.at<Vec3b>(yr, xr) = originImage.at<Vec3b>(y, x);
		}
	}

	// interpolation the rotation image;
	RotationInterpolation(originImage, desImage, angle, additionWidth);

	return desImage;

}


int main(int argc, char** argv)
{
	char * fname = argv[1];										// Read file name from command line
	//const char * fname = "C:\\Users\\hxtruong\\Pictures\\dora.jpg";


	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}

	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	imshow("The original image", image);						//	Show image in screen 

	if (argc < 4)			// fileImageName command parameter
	{
		cout << "Not enough parameter." << endl;
		return 0;
	}
	if (strcmp(argv[2], "-rotate") == 0)
	{
		float angle = atof(argv[3]);
		imshow("Rotation image", Rotation(image, angle));
	}
	else if (strcmp(argv[2], "-rotateN") == 0)
	{
		float angle = atof(argv[3]);
		imshow("Rotation image with full width", RotationN(image, angle));
	}
	else  if (strcmp(argv[2], "-scale") == 0)
	{
		if (argc < 5) {
			cout << "Not enough parameter." << endl;
			return 0;
		}
		float rowScale = atof(argv[3]);
		float colScale = atof(argv[4]);
		imshow("Scale image", Scale(image, rowScale, colScale));
	}
	waitKey(0);
	return 0;
}

