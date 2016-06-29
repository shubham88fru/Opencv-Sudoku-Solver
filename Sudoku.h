#pragma once

#include "OCR.h"

//#include <string>


class Sudoku
{
public:
	//constructors
	Sudoku();
	

	//grab the sudoku image
	void getSudoku(string imgName);
	void processSudoku(bool loadByImg);
	void sendDigitsToOCR(/*OCR recogDigit*/);

	//capture the sudoku from the webacam 
	void captureSudoku(VideoCapture cap);

private:
	OCR _ocr;
	void _threshold(Mat &img,Mat &threshimg);
	void _findHoughLines(Mat img, vector <Vec2f> &Lines);
	void _mergeHoughLines(vector <Vec2f> *lines);
	void _drawLines(vector <Vec2f> &lines);
	void _allignSudoku(const vector <Vec2f> &lines);
	void _drawEdges(const Vec2f &edge,Scalar rgb);
	void _preProcessCell();
	void _canny();
	//find the largest contour in the threshed img(largest contour will be our sudoku's outline)
	void _extractSudoku(Mat img);
	

	string _sudokuImgName;

	Mat _threshROISudoku;
	Mat _Sudoku;
	Mat _cannySudoku;
	Mat _threshSudoku;
	Mat _graySudoku;
	Mat _blurrSudoku;
	Mat _ROISudoku;
	Mat _allignedSudoku;
	Mat _threshAllignedSudoku;
	Mat _cell;

	//bools to show the windows
	//bool _showSudoku = true;
	//bool _showBinaryFeed = true;
	//bool _showBinarySudoku = false;
	bool _showCannySudoku = true;
};

