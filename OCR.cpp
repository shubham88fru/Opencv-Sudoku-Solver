#include "OCR.h"

float MIN_CONTOUR_AREA = 20;

int RESIZED_WIDTH = 20;
int RESIZED_HEIGHT = 30;

OCR::OCR()
{
}


OCR::~OCR()
{
}

void OCR::genData(string trainImg)
{
	Mat flattendFloatImg;

	vector <int>  validNums = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	
	vector <vector<Point>> contours;
	vector <Vec4i> hierarchy;

	//load the training image and check if it has loaded correctly
	_trainingImage = imread(trainImg, CV_LOAD_IMAGE_UNCHANGED);
	if (_trainingImage.empty()){
		
		cout << "couldn't load the training image\n\nExiting theProgram now.....";
		system("PAUSE");
		exit(1);
	}

	imshow("TrainingImg", _trainingImage);
	
	//perform some preprocesings
	cvtColor(_trainingImage, _grayImg, CV_BGR2GRAY);
	GaussianBlur(_grayImg, _blurrImg, Size(3, 3), 0);
	adaptiveThreshold(_blurrImg, _threshImg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 3, 2);
	imshow("threshTrain", _threshImg);

	//clone the threshold image,because the findcontour changes it
	_threshClone = _threshImg.clone();

	//find the contours
	findContours(_threshImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	
	for (int i = 0; i < contours.size(); i++){
	
		float area = contourArea(contours[i],false);
		if (area > MIN_CONTOUR_AREA){
		
			Rect rect = boundingRect(contours[i]);
			
			//adjust the heights and widths to crop the digits perfectly
			rect.y -= 1;
			rect.x -= 1;
			rect.height += 2;
			rect.width += 2;
			
			rectangle(_trainingImage, rect, Scalar(0,255, 0), 1, CV_AA);
			imshow("TrainingImg", _trainingImage);

			Mat currentDigit = _threshClone(rect);
			imshow("currentDigit", currentDigit);
			
			Mat resizedDigit;
			
			resize(currentDigit, resizedDigit, Size(RESIZED_WIDTH, RESIZED_HEIGHT));
			imshow("ResizedDigit", resizedDigit);

			int input = waitKey(0);
			if (input == 27){
			
				exit(1);
			}
			else if (find(validNums.begin(), validNums.end(),input)!=validNums.end()){
				
				_classificationNums.push_back(input);
				
				Mat floatImg;
				resizedDigit.convertTo(floatImg, CV_32FC1);
				
				Mat flattendImg;
				flattendImg = floatImg.reshape(1, 1);

				flattendFloatImg.push_back(flattendImg);
			
			}//end else if
	
		}//end if
	
	}//end for
	
	FileStorage classificationFile("classifications.xml", FileStorage::WRITE);

	if (!classificationFile.isOpened()){
	
		cout << "couldn't create the classification.xml\nExiting the program....";
		
		system("PAUSE");
		
		exit(1);
	}

	classificationFile << "classifications" << _classificationNums;
	
	classificationFile.release();

	FileStorage imagesFile("images.xml", FileStorage::WRITE);

	if (!imagesFile.isOpened()){

		cout << "couldn't create the classification.xml\nExiting the program....";
		
		system("PAUSE");
		
		exit(1);
	}
	
	imagesFile << "Images" << flattendFloatImg;
	
	imagesFile.release();

	cout << "Writing data....."<<endl<<"completed! "<<"Press any key to exit: ";


	waitKey(0);
}

/*void*/ int  OCR::Train(Mat img)
{

	
	FileStorage classificationFile("classifications.xml", FileStorage::READ);
	
	if (!classificationFile.isOpened()){

		cout << "coulnt load the file\n";
		exit(1);
	}

	classificationFile["classifications"] >> classificationNumsTrain;
	
	classificationFile.release();

	FileStorage imageFile("images.xml", FileStorage::READ);
	
	if (!imageFile.isOpened()){

		cout << "coulnt load the file\n";
		exit(1);
	}

	imageFile["Images"] >> imageAsFlattenedFloatTrain;
	
	imageFile.release();

	knearest = KNearest();
	knearest.train(imageAsFlattenedFloatTrain, classificationNumsTrain);

	//cout << "Training complete!!!!......";
	
	
	_test = img;
	//Mat resi;
	//resize(_test, resi, Size(RESIZED_WIDTH, RESIZED_HEIGHT));
	_test.convertTo(floatimg, CV_32FC1);
	Mat flattenedimg;
	flattenedimg = floatimg.reshape(1, 1);

	float character = knearest.find_nearest(flattenedimg, 1);

	//cout << char(int(character));

	char digit = char(int(character));

	switch (digit){
	
	case '0':
		return 0;
		break;

	case '1':
		return 1;
		break;

	case '2':
		return 2;
		break;

	case '3':
		return 3;
		break;

	case '4':
		return 4;
		break;

	case '5':
		return 5;
		break;

	case '6':
		return 6;
		break;

	case '7':
		return 7;
		break;

	case '8':
		return 8;
		break;

	case '9':
		return 9;
		break;

	default:
		cout << "invalid digit recognized\n";
		system("PAUSE");
		exit(1);
	}

//	_test=img.clone();
//
//	if (_test.isContinuous()){
//		cout << "\ntest is cont\n";
//
//	}
//	else cout << "test is not cont\n";
//	Mat ROIresized;
//	resize(_test, ROIresized, Size(26, 26));
//	if (ROIresized.isContinuous()){
//
//		cout << "ROIresized is cont\n";
//
//	}
//	else cout << "ROIresized not cont";
//	imshow("test", _test);
//	Mat newIMAGE;
//	ROIresized.convertTo(floatimg, CV_32FC1);
//	if (floatimg.isContinuous()){
//		cout << "\nfloat is cont\n";
//
//	}
//	else cout << "float is not cont\n";
//	floatimg.copyTo(newIMAGE);
//	if (newIMAGE.isContinuous()){
//	
//		cout << "newIMAGE is cont\n";
//
//	}
//	else cout << "newIMAGE not cont";
//	Mat flattenedimg;
//flattenedimg = floatimg.reshape(1, 1);
//
//float character = knearest.find_nearest(flattenedimg, 1);
//
//	cout << char(int(character))<<"\n";


}


void OCR::Recognize(Mat img)
{
	/*_test = img;

	_test.convertTo(floatimg, CV_32FC1);
	Mat flattenedimg;
	flattenedimg = floatimg.reshape(1, 1);

	float character = knearest.find_nearest(flattenedimg, 1);

	cout << char(int(character));*/
}