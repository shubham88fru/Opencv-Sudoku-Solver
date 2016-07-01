#pragma once



#include "OCR.h"


//#include <string>


class Sudoku
{
public:
	//constructor
	Sudoku();
	
	///////////////////Functions for image processing//////////////////////////

	//grab the sudoku image
	void getSudoku(string imgName);
	void processSudoku(bool loadByImg);
	void sendDigitsToOCR(/*OCR recogDigit*/);
	void printInputSudoku();
	void printSolvedSudoku();
	void overlayResult();

	//capture the sudoku from the webacam 
	void captureSudoku(VideoCapture cap);
	
	///////////////////functions to solve he extracted sudoku/////////////////////
	//we will use the bactracking algorithm to  solve the sudoku

	bool Solve();

private:
	
	OCR _ocr;
	
	
	int _Digit;
	
	int _sudokuDigits[9][9];

	///////////////////Functions for image processing//////////////////////////

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

	///////////////////functions to solve he extracted sudoku/////////////////////
	//we will use the bactracking algorithm to  solve the sudoku

	bool _canBePlaced(int row, int column, int digit);
	bool _positionEmpty(int &row,int &column);
	bool _digitInRow(int row, int column, int digit);
	bool _digitInColumn(int row, int column, int digit);
	bool _digitIn3by3Box(int startrow, int startcolumn, int digit);
};

