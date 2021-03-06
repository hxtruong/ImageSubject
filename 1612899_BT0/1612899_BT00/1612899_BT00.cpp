#include "pch.h"
#include <iostream>
#include <stdlib.h>
#include "opencv2\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace std;
using namespace cv;

Mat tranformWhiteBlack(const Mat image, int threshold = ADAPTIVE_THRESH_GAUSSIAN_C) {
	/*
		To convert a image to white black image, do 2 step:
			+ convert it to grayscale
			+ then threshold it to binary ( If pixel value is greater than a threshold value,
				it is assigned one value (may be white), else it is assigned another value (may be black).
	*/
	Mat image_gray;
	cvtColor(image, image_gray, CV_BGR2GRAY);
	return Mat(image_gray > threshold);
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

int main(int argc, char** argv)
{
	cout << "Hello! 1612899" << endl;

	if (argc < 2) {
		cout << "The program can not open and show image." << endl;
		return -1;
	}

	//const char * fname = "C:\\Users\\Truong\\Pictures\\sent.png";
	char * fname = argv[1];										// Read file name from command line
	Mat image = imread(fname, IMREAD_COLOR);					// Always convert image to 3 color: red, blue, green 
	if (image.empty())											// Check if image can't read image of no data in image
	{
		cout << "Can not open image!" << endl;					//Print to screen
		return -1;
	}
	namedWindow("Display window", WINDOW_AUTOSIZE);				// Set window with name is "Display window" with autosize of window
	imshow("The original image", image);						//	Show image in screen 

	if (argc == 2 || (argc > 2 && strcmp(argv[2], "-h") == 0 || strcmp(argv[2], "--help") == 0)) {
		cout << "--help command:" << endl;
		cout << "-wb || [threshold] or --whiteblack || [threshold]: tranfrom image to white black with [threshold] or DEFAULT_THRESHOLD if not [threshold] " << endl;
		cout << "-b [alpha] or --brightness [alpha]: adjust contract with [alpha] argument " << endl;
		cout << "-c [beta] or --constract [beta]: adjust brightness wiht [beta] argument" << endl;
	}
	else if (strcmp(argv[2], "-wb") == 0 || strcmp(argv[2], "--whiteBlack") == 0) {		// Tranform to white black image
		if (argc == 4) {
			float threshold = atoi(argv[3]);
			imshow("White black image", tranformWhiteBlack(image, threshold));
		}
		else
			imshow("White black image", tranformWhiteBlack(image));
	}
	else if (strcmp(argv[2], "-c") == 0 || strcmp(argv[2], "--constract") == 0) {
		float alpha = atof(argv[3]);
		imshow("Constract image", adjustContract(image, alpha));
	}
	else if (strcmp(argv[2], "-b") == 0 || strcmp(argv[2], "--brightness") == 0) {
		float beta = atof(argv[3]);
		imshow("Brightness image", adjustBrightness(image, beta));
	}

	waitKey(0);
	return 0;

}
