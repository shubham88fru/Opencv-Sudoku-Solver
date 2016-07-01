//self made header files
#include "Sudoku.h"

//the trainig image
const string trainImg = "train6.png";
//the sudoku img to be solved
const string SudokuImg = "SudokuPaper2.jpg";

//set explicitly to true when you want to generate the training data for OCR
bool GenrateOCRData = false;

//set explicitly to true when you want to Train the KNN algorithm
//set this to true only when you have generated the data and copied it in the right folder
bool TrainAndTest = false;

//load the sudoku from a sudoku image in the computer
bool loadSudokuImg = true;
//grab sudoku from the camerafeed
bool captureSudokuFromCamera = false;

int main()
{
	OCR recognizeDigits;
	

	if (GenrateOCRData == true){
		
		//generate the OCR data
		recognizeDigits.genData(trainImg);
	}
	
	if (TrainAndTest == true){
	
		///recognize the digits
		//recognizeDigits.Train();
	}
	
	Sudoku sudoku;
	
	if (loadSudokuImg == true){
		
		//get the sudoku image to be solved
		sudoku.getSudoku(SudokuImg);
		//process the loaded sudoku
		sudoku.processSudoku(loadSudokuImg);

		sudoku.sendDigitsToOCR();

		sudoku.printInputSudoku();

		bool isSolved = sudoku.Solve();

		if (isSolved == true){
		
			sudoku.printSolvedSudoku();
			
			sudoku.overlayResult();
		}
		else {
			
			cout << "The provided Sudoku can't be Solved!!!!\n";
			system("PAUSE");
		}

	}
	else if (captureSudokuFromCamera == true){

		VideoCapture cap;

		cap.open(0);

		if (!cap.isOpened()){

			cout << "couldnt open the webcam\nExiting the program........";

			return 0;
		}
		
		while (1){
		
			sudoku.captureSudoku(cap);

			sudoku.processSudoku(loadSudokuImg);
			
			
		}


	}


	return 0;
}