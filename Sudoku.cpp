#include "Sudoku.h"

//stores the minimum vote that a line must have to be considered as a line.
int minVote;

//named windows
const string CameraFeed = "Original Feed";
const string sudokuWindow = "Sudoku";
//const string threshSudokuWindow1 = "Binary";
//const string threshSudokuWindow2 = "BinarySudoku";
const string allignedSudokuWindow = "allignedSudoku";
const string CannySudokuWindow = "Canny";


Sudoku::Sudoku()
{

}

//get the sudoku image stored in the computer
void Sudoku::getSudoku(string imgName)
{
	_sudokuImgName = imgName;

	//read the sudoku image
	_Sudoku = imread(_sudokuImgName, CV_LOAD_IMAGE_UNCHANGED);

	//check if the image was loaded successfully
	if (_Sudoku.empty()){
	
		cout << "could not load the image\n\n";
		cout << "Exiting the program now.......";

		exit(1);
	}

	imshow(CameraFeed, _Sudoku);


}

//capture the sudoku from the webcam
void Sudoku::captureSudoku(VideoCapture cap)
{
	cap >> _Sudoku;
	
	//flip the the camerafeed by 180 degree about the y axis
	//because my webacm was showing fliped image
	flip(_Sudoku, _Sudoku, 1);
	
	waitKey(20);

	namedWindow(CameraFeed, 1);

	//imshow(CameraFeed, _Sudoku);
	

}


/////////////////////////////This is the image processing part////////////////////////////////////


//perform various processings
void Sudoku::processSudoku(bool loadByImg)
{

	//vector to store the hough lines
	vector <Vec2f> Lines;
	
	//if the user want to load the sudoku from a saved image
	if (loadByImg == true){

		//minVote = 350;
		minVote = 220;
		
		//threshold the image so that we can find the contours and thereby extract the sudoku
		_threshold(_Sudoku,_threshSudoku);

		//extract the sudoku frome the image by assuming 
		//that the largest contour in the iamge is of the sudoku grid
		_extractSudoku(_threshSudoku);
		
		//once again threshold the ROI(i.e sudoku box) so that we can find the 
		//hough lines
		_threshold(_ROISudoku, _threshROISudoku);
	
		/*_showBinarySudoku = true;
		if (_showBinarySudoku == true){
			namedWindow(threshSudokuWindow2, 1);
			imshow(threshSudokuWindow2, _threshROISudoku);
		}*/

		//find the hough lines,merge and draw them
		_findHoughLines(_threshROISudoku, Lines);
		_mergeHoughLines(&Lines);
		//_drawLines(Lines);

		//detct the edges(left edge,top edge etc)
		_allignSudoku(Lines);

		_threshold(_allignedSudoku, _threshAllignedSudoku);

		dilate(_threshAllignedSudoku, _threshAllignedSudoku, getStructuringElement(MORPH_RECT, Size(1, 1)), Point(-1, -1), 1);
		//erode(_threshAllignedSudoku, _threshAllignedSudoku, getStructuringElement(MORPH_RECT, Size(2, 2)), Point(-1, -1), 4);
		
		namedWindow("PerformOCR", 1);
		imshow("PerformOCR", _threshAllignedSudoku);
		//sendDigitsToOCR();
		//find the contours to extract the region of interest
		//_extractSudoku(_threshSudoku);

	}
	//if the user wants to capture the sudoku from the camera feed(webcam)
	else{
	
		minVote = 240;

		_canny();

		//find the hough lines
		_findHoughLines(_cannySudoku,Lines);

		//_extractSudoku(_cannySudoku);
	}
	//convert the image to binary;
	//_threshold();
}

void Sudoku::_canny()
{
	//perform some preprocessing
	cvtColor(_Sudoku, _graySudoku, CV_BGR2GRAY);
	blur(_graySudoku, _blurrSudoku, Size(3, 3));
	//GaussianBlur(_graySudoku, _blurrSudoku, Size(3, 3),0);
	Canny(_blurrSudoku, _cannySudoku, 30, 60, 3);
	
	if (_showCannySudoku == true ){
		namedWindow(CannySudokuWindow, WINDOW_AUTOSIZE);
		imshow(CannySudokuWindow, _cannySudoku);
	}
}

void Sudoku::_threshold(Mat &img, Mat &threshimg)
{
	Mat _graySudoku;
	Mat _blurrSudoku;
	
	//perform some preprocessing
	cvtColor(img, _graySudoku, CV_BGR2GRAY);
	GaussianBlur(_graySudoku, _blurrSudoku, Size(7, 7), 0);
	adaptiveThreshold(_blurrSudoku, threshimg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 3, 2);

	//dilate if need be

	//Mat kernel = (Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
	//dilate(_threshSudoku, _threshSudoku, kernel);
	
	/*if (_showBinaryFeed == true){
		namedWindow(threshSudokuWindow1, 1);
		imshow(threshSudokuWindow1, _threshSudoku);
	}*/

}

void Sudoku::_findHoughLines(Mat img, vector <Vec2f> &Lines)
{

	HoughLines(img, Lines, 1, CV_PI / 180, minVote, 0, 0);
	
	/*draw the lines before merging ,just to test whether the algorithm is merging the lines to desired level*/

	//for (int i = 0; i < Lines.size(); i++){

	//	float rho = Lines[i][0];
	//	float theta = Lines[i][1];

	//	double COS = cos(theta);
	//	double SINE = sin(theta);

	//	double x0 = rho*COS;
	//	double y0 = rho*SINE;

	//	Point pt1, pt2;

	//	pt1.x = cvRound(x0 - (2000 * SINE));
	//	pt1.y = cvRound(y0 + (2000 * COS));
	//	pt2.x = cvRound(x0 + (2000 * SINE));
	//	pt2.y = cvRound(y0 - (2000 * COS));

	//	line(_Sudoku, pt1, pt2, Scalar(0, 0, 255), 1, CV_AA);
	//}
	//Mat test = _Sudoku.clone();
	//namedWindow("test", 1);
	//imshow("test", test);

	

}

//we will try and merge the nearby lines so that they fuse together to a common lines
//this will reduce the number of unwanted lines.
void Sudoku::_mergeHoughLines(vector <Vec2f> *lines)
{
	//iterartor to iterarte through the vector of lines
	vector <Vec2f>::iterator it1;
	
	for (it1 = lines->begin(); it1 != lines->end(); it1++){
		
		//we will mark the lines which have been merged,by setting the rho and theta values to possibly the 
		//impossible values,this is very necessary because we need to have a way to recognize the lines that 
		//have been merged and no longer exist.
		if ((*it1)[0] == 0 || (*it1)[1] == -100){
			
			continue;
		}

		float rho1 = (*it1)[0];
		float theta1 = (*it1)[1];

		Point point11, point12;
		
		if (((*it1)[1] > (CV_PI * 45 / 180)) && ((*it1)[1] < (CV_PI * 135 / 180))){
			
			point11.x = 0;
			point11.y = (rho1 / sin(theta1));

			point12.x = _ROISudoku.size().width;
			point12.y = (rho1 / sin(theta1)) - (_ROISudoku.size().width/tan(theta1));
		}
		else{
			
			point11.y = 0;
			point11.x = (rho1 / cos(theta1));

			point12.y = _ROISudoku.size().height;
			point12.x = (rho1 / cos(theta1)) - (_ROISudoku.size().height *tan(theta1));
		}

		vector<Vec2f>::iterator it2;
		
		for (it2 = lines->begin(); it2 != lines->end(); it2++){
		
			Point point21, point22;
			

			if ((*it1) == (*it2)){
				
				continue;
			}

			if ((fabs(((*it2)[0] - (*it1)[0])) < 20) && (fabs(((*it2)[1] - (*it1)[1])) < CV_PI*20/180)){
			
				
				float rho2 = (*it2)[0];
				float theta2 = (*it2)[1];

				if (((*it2)[1] > (CV_PI * 45 / 180)) && ((*it2)[1] < (CV_PI * 135 / 180))){

					point21.x = 0;
					point21.y = (rho2 / sin(theta2));

					point22.x = _ROISudoku.size().width;
					point22.y = (rho2 / sin(theta2)) - (_ROISudoku.size().width / tan(theta2));
				}
				else{

					point21.y = 0;
					point21.x = (rho2 / cos(theta2));

					point22.y = _ROISudoku.size().height;
					point22.x = (rho2 / cos(theta2)) - (_ROISudoku.size().height * tan(theta2));
				}

				if ((double)((point11.x - point21.x)*(point11.x - point21.x) + (point11.y - point21.y)*(point11.y - point21.y)) < 64*64 && (double)((point12.x - point22.x)*(point12.x - point22.x) + (point12.y - point22.y)*(point12.y - point22.y)) <64*64){
				
					cout << "Merging....\n";

					(*it1)[0] = ((*it1)[0] + (*it2)[0]) / 2.0;
					(*it1)[1] = ((*it1)[1] + (*it2)[1]) / 2.0;

					(*it2)[0] = 0;
					(*it2)[1] = -100;
				}
			}
		}
	}

}

void Sudoku::_allignSudoku(const vector <Vec2f> &lines)
{
	/*	We will first detect the edges of the sudoku to find the point of intersection (i.e the corners of the sudoku)*/

	//the edges
	Vec2f topEdge = Vec2f(1000, 1000);          //double topXintercept = 0;		double topYintercept = 10000;
	Vec2f bottomEdge = Vec2f(-1000, -1000);		//double bottomXintercept = 0;	double bottomYintercept = 0;
	Vec2f leftEdge = Vec2f(1000, 1000);			double leftXintercept = 10000;	double leftYintercept = 0;
	Vec2f rightEdge = Vec2f(-1000, -1000);		double rightXintercept = 0;		double rightYintercept = 0;

	for (int i = 0; i < lines.size(); i++){
		
		Vec2f currentLine = lines[i];
		
		float rho = currentLine[0];
		float theta = currentLine[1];
		
		float xinter = currentLine[0] / cos(currentLine[1]);
		float yinter = currentLine[0] / sin(currentLine[1]);
		
		if (rho == 0 && theta == -100){
			
			continue;
		}
		
		//check the horizontal lines
		if ((theta > CV_PI * 60 / 180) && (theta < CV_PI * 120 / 180)){
			
			if (rho < topEdge[0]){
			
				topEdge = currentLine;
			}

			if (rho > bottomEdge[0]){

				bottomEdge = currentLine;
			}
		}

		else if ((theta < CV_PI * 10 / 180) || (theta > CV_PI * 170 / 180)){
		
			if (xinter > rightXintercept){

				rightEdge = currentLine;
				rightXintercept = xinter;
			}
			if (xinter <= leftXintercept){

				leftEdge = currentLine;
				leftXintercept = xinter;
			}
		}
	}

	/*Now that we have the edges,we can just print them out on our image to check if they are correct*/
	
	/*_drawEdges(topEdge, Scalar(255, 0, 0));
	_drawEdges(bottomEdge, Scalar(0, 255, 0));
	_drawEdges(leftEdge, Scalar(0, 0, 255));
	_drawEdges(rightEdge, Scalar(0, 0, 0));*/

	//imshow(CameraFeed, _Sudoku);
	//imshow(sudokuWindow, _ROISudoku);

	/* We will now find the corners by finding the points of intersection of the edges
	obviously the corners will be the point of intersection of the edges.
	also we will use the two point form of a line to find the point of intersection.
	*/
	
	//finding the points of intersection

	//first of all we need two points on each line.
	Point top1, top2, bottom1, bottom2, left1, left2, right1, right2;

	//for two points on the vertical edges
	//it is neccessary to check if a line does not have 
	//slope infinity because if we dont and go ahead to points trivially, our program may crash
	if (leftEdge[1] != 0){
	
		left1.x = 0;								left1.y = leftEdge[0] / sin(leftEdge[1]);
		left2.x = _ROISudoku.size().width;		    left2.y = left1.y - (left2.x / tan(leftEdge[1]));
	}
	else{
	
		left1.y = 0;								left1.x = leftEdge[0] / cos(leftEdge[1]);
		left2.y = _ROISudoku.size().height;			left2.x = left1.x - (left2.y * tan(leftEdge[1]));
	}

	if (rightEdge[1] != 0){

		right1.x = 0;								right1.y = rightEdge[0] / sin(rightEdge[1]);
		right2.x = _ROISudoku.size().width;			right2.y = right1.y - (right2.x / tan(rightEdge[1]));
	}
	else{

		right1.y = 0;								right1.x = rightEdge[0] / cos(rightEdge[1]);
		right2.y = _ROISudoku.size().height;		right2.x = right1.x - (right2.y * tan(rightEdge[1]));
	}

	top1.x = 0;										top1.y = topEdge[0] / sin(topEdge[1]);
	top2.x = _ROISudoku.size().width;				top2.y = top1.y - (top2.x / tan(topEdge[1]));

	bottom1.x = 0;									bottom1.y = bottomEdge[0] / sin(bottomEdge[1]);
	bottom2.x = _ROISudoku.size().width;			bottom2.y = bottom1.y - (bottom2.x / tan(bottomEdge[1]));

	//leftA is "A" in the equation Ax+By=C. and so on.
	double leftA = left2.y - left1.y;
	double leftB = left1.x - left2.x;   
	double leftC = leftB*left1.y + leftA*left1.x; //cout << leftC << endl;

	double rightA = right2.y - right1.y;
	double rightB = right1.x - right2.x;
	double rightC = rightB*right1.y + rightA*right1.x;

	double topA = top2.y - top1.y;  //cout << topA << endl;
	double topB = top1.x - top2.x;	cout << topB<< endl;
	double topC = topB*top1.y + topA*top1.x;

	double bottomA = bottom2.y - bottom1.y;
	double bottomB = bottom1.x - bottom2.x;
	double bottomC = bottomB*bottom1.y + bottomA*bottom1.x;

	//the deterninants
	double topLeftDet = topA*leftB - topB*leftA; //cout << topLeftDet << endl;
	double topRightDet = topA*rightB - topB*rightA;
	double bottomLeftDet = bottomA*leftB - bottomB*leftA;
	double bottomRightDet = bottomA*rightB - bottomB*rightA;

	//actual points of inersection
	Point topLeft, topRight, bottomLeft, bottomRight;

	topLeft.x = (topC*leftB - topB*leftC) / topLeftDet;
	topLeft.y = (topA*leftC - topC*leftA) / topLeftDet;
	//circle(_ROISudoku, Point(topLeft.x, topLeft.y), 6, Scalar(0, 0, 255), -1, CV_AA);
	//cout << topLeft.x<<endl;

	topRight.x = (topC*rightB - topB*rightC) / topRightDet;
	topRight.y = (topA*rightC - topC*rightA) / topRightDet;
	//circle(_ROISudoku, Point(topRight.x, topRight.y), 6, Scalar(0, 255, 0), -1, CV_AA);
	//cout << topRight.x << endl;

	bottomRight.x = (bottomC*rightB - bottomB*rightC) / bottomRightDet;
	bottomRight.y = (bottomA*rightC - bottomC*rightA) / bottomRightDet;
	//circle(_ROISudoku, Point(bottomRight.x, bottomRight.y), 6, Scalar(255, 0, 0), -1, CV_AA);
	//cout << bottomRight.x << endl;

	bottomLeft.x = (bottomC*leftB - bottomB*leftC) / bottomLeftDet;
	bottomLeft.y = (bottomA*leftC - bottomC*leftA) / bottomLeftDet;
	//circle(_ROISudoku, Point(bottomLeft.x, bottomLeft.y), 6, Scalar(255, 255, 0), -1, CV_AA);
	//cout << bottomLeft.x << endl;

	//we will now determine the longest edge..
	int maxLength = ((topRight.x - topLeft.x)*(topRight.x - topLeft.x)) + ((topRight.y - topLeft.y)*(topRight.y - topLeft.y));
	 
	int length = ((topRight.x - bottomRight.x)*(topRight.x - bottomRight.x) + (topRight.y - bottomRight.y)*(topRight.y - bottomRight.y));
	if (length > maxLength)
		maxLength = length;
	length = ((bottomRight.x - bottomLeft.x)*(bottomRight.x - bottomLeft.x)) + ((bottomRight.y - bottomLeft.y)*(bottomRight.y - bottomLeft.y));
	if (length > maxLength)
		maxLength = length;
	length = ((bottomLeft.x - topLeft.x)*(bottomLeft.x - topLeft.x)) + ((bottomLeft.y - topLeft.y)*(bottomLeft.y - topLeft.y));
	if (length > maxLength)
		maxLength = length;

	maxLength = sqrt(maxLength);

	//in order to allign the sudoku in the image we need to perform warpperspective 
	Point2f originalPoints[4], mappedPoints[4];
	
	originalPoints[0] = topLeft;			mappedPoints[0] = Point(0, 0);
	originalPoints[1] = topRight;			mappedPoints[1] = Point(maxLength - 1, 0);
	originalPoints[2] = bottomRight;		mappedPoints[2] = Point(maxLength - 1, maxLength - 1);
	originalPoints[3] = bottomLeft;			mappedPoints[3] = Point(0, maxLength - 1);

	_allignedSudoku = Mat(Size(maxLength, maxLength), CV_8UC1);
	
	warpPerspective(_ROISudoku, _allignedSudoku, cv::getPerspectiveTransform(originalPoints, mappedPoints), Size(maxLength, maxLength));
	
	namedWindow(allignedSudokuWindow, 1);
	
	imshow(allignedSudokuWindow, _allignedSudoku);

	//if (_showSudoku == true){

	//	//create a window to display the sudoku
	//	namedWindow(sudokuWindow, 1);
	//	imshow(sudokuWindow,_ROISudoku);
	//}
}

void Sudoku::_drawEdges(const Vec2f &edge,Scalar rgb)
{
	if (edge[1] != 0){
		
		float m = -(1 / tan(edge[1]));
		float c = (edge[0] / sin(edge[1]));

		line(_ROISudoku, Point(0, c), Point(_ROISudoku.size().width, m*_ROISudoku.size().width + c), rgb, 2, CV_AA);
	}
	else
		line(_ROISudoku, Point(edge[0], 0), Point(edge[0], _ROISudoku.size().height), rgb, 2, CV_AA);
}

void Sudoku::_drawLines(vector <Vec2f> &lines)
{
	for (int i = 0; i < lines.size(); i++){

		if (lines[i][0] == 0 || lines[i][1] == -100){
		
			cout << "Merged Lines....\n";
			continue;
		
		}

		float rho = lines[i][0];
		float theta = lines[i][1];

		double COS = cos(theta);
		double SINE = sin(theta);

		double x0 = rho*COS;
		double y0 = rho*SINE;

		Point pt1, pt2;

		pt1.x = cvRound(x0 - (2000 * SINE));
		pt1.y = cvRound(y0 + (2000 * COS));
		pt2.x = cvRound(x0 + (2000 * SINE));
		pt2.y = cvRound(y0 - (2000 * COS));

		line(_ROISudoku, pt1, pt2, Scalar(255, 0, 0), 1, CV_AA);
	}

	//imshow(CameraFeed, _Sudoku);
}

void Sudoku::_extractSudoku(Mat img)
{	
	Mat copy = img.clone();
	
	vector < vector<Point> > contours;
	vector <Vec4i> hierarchy;
	
	int largestContourIndex;

	findContours(copy, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	double maxContourArea = contourArea(contours[0], false);
	
	for (int i = 0; i < contours.size(); i++){
		
		double area = contourArea(contours[i]);
		//if (area >= 1000){
		//
		//	Rect BoundingRect = boundingRect(contours[i]);

		//	rectangle(_Sudoku, BoundingRect, Scalar(0, 255, 0), 1);
		//}

		if (area >= maxContourArea){
		
			maxContourArea = area;
			largestContourIndex = i;

		}

	}
	
	Rect BoundingRect = boundingRect(contours[largestContourIndex]);
	//rectangle(_Sudoku, BoundingRect, Scalar(0, 255, 0), 2);
	
	_ROISudoku = _Sudoku(BoundingRect);
	//_ROISudoku = copy(BoundingRect);
	//drawContours(_Sudoku, contours, largestContourIndex, Scalar(0, 0, 255), 2);

	//if (_showSudoku == true){

	//	//create a window to display the sudoku
	//	namedWindow(sudokuWindow, 1);
	//	imshow(sudokuWindow,_ROISudoku);
	//}
}

void Sudoku::sendDigitsToOCR()
{
	
	
	/*if (_cell.isContinuous()){
	
		cout << "_Cell1 continuos" << endl;
	}
	else cout << "_cell1 continous" << endl;*/

	int cellLength = floor((float)_allignedSudoku.size().width / 9);

	//cout << cellLength << endl;
	//cout << _allignedSudoku.size().width;
	
	cout << "Recognizing the Digits....\n";
	cout << "Please wait....\n";
	
	Rect rect = Rect((1* cellLength + 6), 6 + (0 * cellLength), cellLength-6 , cellLength-6);
	_cell = Mat(_threshAllignedSudoku, rect);
	Moments momentum = moments(_cell, false);
	cout << momentum.m00 << endl;
	//_cell.copyTo(newimg);
	//if (newimg.isContinuous()){
	//
	//	cout << "new is continous \n";
	//}
	//else cout << "new is not cont\n";
	//if (_cell.isContinuous()){
	//
	//	cout << "_cell2 is continous";
	//}
	//else cout << "not2 continous";
	//if (_threshAllignedSudoku.isContinuous()){

	//	cout << "thres is continous";
	//}
	//else cout << "thresh not continous"<<endl;
	//*newimg = _cell.clone();
	
	//Mat resized;
	//resize(_cell, resized, Size(20, 30));
	imshow("test", _cell);
	//imshow("resize", resized);
	for (int i = 0; i < 9; i++){
	
		for (int j = 0; j < 9; j++){
			
			Rect rect = Rect((j * cellLength + 6), 6 + (i * cellLength), cellLength - 6, cellLength - 6);
			_cell = Mat(_threshAllignedSudoku, rect);
			//_cell.copyTo(newimg);
			//_cell = Mat(_allignedSudoku, rect);
			Moments moment = moments(_cell, false);
			if (moment.m00 >=1000){
				//_ocr.Train(newimg);
				_preProcessCell();

				//_sudokuDigits[i][j] = _Digit;
				_sudokuDigits[i][j] = _Digit;

				//cout << " ";
				//cout << "1"<<" ";
			}
			else
				
				_sudokuDigits[i][j] = 0;
				//cout << "." << " ";

			/*char ip = waitKey(0);
			if (ip != 'q'){
				exit(1);
			}*/
		}
		//cout << endl;

	}
	//waitKey(0);
}

void Sudoku::_preProcessCell()
{
	Mat newimg;
	Mat resizedNew;
	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;

	Mat cellClone = _cell.clone();
	findContours(cellClone, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	///Moments moment = moments()
	
	for (int i = 0; i < contours.size(); i++){
	
		Rect rect = boundingRect(contours[i]);
		if (rect.x >= 2 && rect.x <= 22 &&contourArea(contours[i])>=20/*&& rect.y >= 2 */)/*&& rect.y <= 18*/{
		
			newimg = _cell(rect);
			resize(newimg, resizedNew, Size(20, 30));
			_Digit=_ocr.Train(resizedNew);
			//imshow("testing", resizedNew);
			/*if (resizedNew.isContinuous()){
			
				cout << "resize is cont";
			}
			else cout << "resized is not";*/
		}
		else
		{
			cout << contourArea(contours[i]);
			//_sudokuDigits[i][j] = 0;
			resize(_cell, resizedNew, Size(20, 30));
			imshow("problem", resizedNew);
		}
	}
	
	
	
	
	
}



////////////////////////Solving the Sudoku/////////////////////////////////////////////////
void Sudoku::printInputSudoku(){

	cout << "---------------------\n";
	for (int i = 0; i < 9; i++){
	
		for (int j = 0; j < 9; j++){

			cout << _sudokuDigits[i][j]<<" ";
			
			if ((j + 1) % 3 == 0){
			
				cout << "|";
			}

			
		}
		cout << endl;

		if ((i + 1) % 3 == 0){

			cout << "---------------------\n";
		}
	}

	//waitKey(0);
}

bool Sudoku::Solve()
{
	int row, col;

	if (!_positionEmpty(row, col)){
		
		return true;
	}

	for (int digit = 1; digit <= 9; digit++){
		
		if (_canBePlaced(row, col, digit)){
		
		
			_sudokuDigits[row][col] = digit;

			if (Solve()){

				return true;
			}

			_sudokuDigits[row][col] = 0;
		}
	}

	return false;
}

bool Sudoku::_positionEmpty(int &row,int &column)
{
	for (row = 0; row < 9; row++){
	
		for (column = 0; column < 9; column++){

			if (_sudokuDigits[row][column] == 0){
			
				return true;
			}
		}
	}
	return false;
}

bool Sudoku::_canBePlaced(int row, int column, int digit)
{
	if ((!_digitInRow(row, column, digit)) && (!_digitInColumn(row, column, digit)) && (!_digitIn3by3Box(row, column, digit))){
	
		return true;
	}
	return false;
	//return !_digitInRow(row, column, digit) && !_digitInColumn(row, column, digit) && !_digitIn3by3Box(row - row % 3, column - column % 3, digit);
}

bool Sudoku::_digitInRow(int row, int column, int digit)
{
	for (int j = 0; j < 9; j++){
	
		if (_sudokuDigits[row][j] == digit){

			return true;
		}
	}
	return false;
}

bool Sudoku::_digitInColumn(int row, int column, int digit)
{
	for (int i = 0; i < 9; i++){

		if (_sudokuDigits[i][column] == digit){

			return true;
		}
	}

	return false;
}

bool  Sudoku::_digitIn3by3Box(int row, int column, int digit)
{
	int startRow = row - row % 3;
	int startColumn = column - column % 3;

	for (int i = startRow; i < (startRow + 3); i++){

		for (int j = startColumn; j < (startColumn + 3); j++){

			if (_sudokuDigits[i][j] == digit){

				return true;
			}
		}
	}
	return false;
}

void Sudoku::printSolvedSudoku()
{

	cout << "---------------------\n";
	for (int i = 0; i < 9; i++){

		for (int j = 0; j < 9; j++){

			cout << _sudokuDigits[i][j] << " ";

			if ((j + 1) % 3 == 0){

				cout << "|";
			}


		}
		cout << endl;

		if ((i + 1) % 3 == 0){

			cout << "---------------------\n";
		}
	}

	waitKey(0);
}