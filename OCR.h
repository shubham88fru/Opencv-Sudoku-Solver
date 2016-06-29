#pragma once

//opencv header files
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2\ml\ml.hpp>

#include <iostream>

using namespace cv;
using namespace std;
//using namespace ml;

class OCR
{
public:
	
	OCR();
	void genData(string trainImg);
	void Train(Mat img);
	void Recognize( Mat img);
	Mat classificationNumsTrain;
	Mat imageAsFlattenedFloatTrain;
	~OCR();

private:
	
	KNearest knearest;

	Mat floatimg;
	Mat _test;
	Mat _trainingImage;
	Mat _grayImg;
	Mat _blurrImg;
	Mat _threshImg;
	Mat _threshClone;
	Mat _classificationNums;
};

