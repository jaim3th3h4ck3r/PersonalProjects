﻿#include "pch.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <stack>

/*
Includes olcConsoleGameEngine for drawing map to console. (Yes, I'm that lazy. I'll learn Win32 consoles later.)
Copyright (C) 2018 Javidx9
Source available here:
https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
*/
//#include <olcConsoleGameEngine.h>

using namespace std;

enum DIRECTION {
	UP		= 0b0001,
	DOWN	= 0b0010,
	LEFT	= 0b0100,
	RIGHT	= 0b1000,

	VERTICAL	= 0b0011,
	HORIZONTAL	= 0b1100
};

struct COORD_MAZE {
	bool isVisited = false;
	const bool isWall = false;
	unsigned char direction = 0b0000;
	unsigned short pathID = 1;
	explicit COORD_MAZE(bool T) :isWall(T), isVisited(T) {};
};




/*Insert all SUPPORTING VARIABLES here (ie. not critical to the algorithm)*/								 
//_____________________________________________________________________________________________________________
//Size of padding needed to complete .bmp file																 //
unsigned int rawPadding = 0;																				 //
//name of default image file; on same folder as executable													 //
string fileDefaultName = "maze.bmp";																		 //
//Pointer to image file in memory																			 //
//Only used to load file into memory for conversion into vector array and as base of final .bmp file		 //
char* rawImageBlock;																						 //
int fileBinarySize;																						   	 //
//Vector of changed positions for speeding up image writing operations										 //
vector<pair <unsigned int, unsigned int>> changedPos;														 //
																											 //
/*CHILD FUNCTIONS*/																							 //
//writes vector array data to diagnostic .txt file															 //
void write_debug_full();
void write_debug(unsigned int maxColumn, unsigned int maxRow);												 //
//writes vector array to final pretty .bmp image															 //
void write_image();																							 //
//Writes vector to console buffer																			 //
class olcConsoleGameEngine;																					 //
void write_console(olcConsoleGameEngine* console); 															 //
//___________________________________________________________________________________________________________//



//Size of maze pixels to navigate
unsigned int mazeSizeHorizontal, mazeSizeVertical;

//2D vector, used for positioning
//Global because writing functions use it for writing data
//First value: column/x-coordinate
//Second value: row/y-coordinate
//REMEMBER: [0][0] is UPPER LEFT corner
vector < vector <COORD_MAZE> > mazeVector;

//Vector for pathID verification
//If a certain pathID is a dead end, it's vector coordinate is set to true
vector <bool> isPathIDDeadEnd{ true };

int main(void){
	{
		char* rawImagePointer;


		//Read image file and load it into memblock
		bool isFileGood;
		ifstream maze;

		do {
			isFileGood = true;

			std::cout << "Enter file name: ";
			cin >> fileDefaultName;

			maze.open(fileDefaultName, ios::in | ios::binary | ios::ate);
			if (!maze.is_open() || maze.bad()) {
				std::cerr << "Error opening file. Try again." << std::endl;
				isFileGood = false;
			}
		} while (!isFileGood);

		fileBinarySize = (int) maze.tellg();
		rawImageBlock = new char[fileBinarySize];
		maze.seekg(0, ios::beg);
		maze.read(rawImageBlock, fileBinarySize);
		maze.close();

		//Read the number of horizontal pixels from header, 18 bytes from start of file
		rawImagePointer = rawImageBlock + 18;
		mazeSizeHorizontal = 256 + static_cast<unsigned int> (*(rawImagePointer));

		//Read the number of vertical pixels from header, 22 bytes from start of file
		rawImagePointer = rawImageBlock + 22;
		mazeSizeVertical = 256 + static_cast<unsigned int>(*(rawImagePointer));

		//Parse memblock into int vector array
		rawImagePointer = rawImageBlock + static_cast<int>(*(rawImageBlock + 10));					// Set pointer to start of image data
		rawPadding = (3 * mazeSizeHorizontal) % 4;

		//Resize coor to match maze dimensions
		mazeVector.resize(mazeSizeVertical);
		if (mazeSizeVertical > mazeVector.capacity())
			return -10;

		//Vector-writing loops
		//Row loop = y-coordinate
		for (unsigned int a = 0; a < mazeSizeVertical; a++) {
			
			mazeVector[a].reserve(mazeSizeHorizontal);
			if (mazeSizeHorizontal > mazeVector[a].capacity())
				return -11;

			//Column loop = x-coordinate
			for (unsigned int b = 0; b < mazeSizeHorizontal; b++) {

				//Location of data in order
				int rawOffset = a * mazeSizeHorizontal + b;
				if (*(rawImagePointer + (3 * rawOffset) + (a * rawPadding)) == 0) {
					mazeVector[a].push_back(COORD_MAZE(true));
				} else {
					mazeVector[a].push_back(COORD_MAZE(false));
				}
			}
		}
	}
	// 2D vector complete
	// Start solving algorithm

	/*
	The new algorithm is based on simply 'filling up' the maze until we find the exit.

	The algorithm works by simply marking its neighbors as being visited, and then moving 
		onto said neighbors until it hits a wall. Each pixel is of a COORD_MAZE structure, and
		the only prefilled data is the isWall value for walls; everything else is empty.
		
	Each time the algorithm hits a wall, it places its current position on a stack and then
		goes on and fills it to one side of the maze with a different pathID. If it is
		a dead end, then the algorith returns to the last coordinate on the stack; if that
		is a dead end too, it'll pop that value off the stack and read the previous value,
		and so forth until it finds a suitable point to continue.
	*/

	write_image();

	//unsigned short iterations;
	bool hasNeighbors = false;
	bool isFinished = false;
	int firstModifier, secondModifier;
	deque < pair <unsigned short, unsigned short> > currentCoord;

	//Tries to find an entry point on the upper and left edges of the image
	for (unsigned int x = 0, y = 0; x < mazeSizeHorizontal - 1 || y < mazeSizeVertical - 1;) {
		if (!mazeVector[x][0].isWall) {
			currentCoord.push_back({ x, 0 });
			mazeVector[x][0].direction = DOWN;
			mazeVector[x][0].isVisited = true;
			break;

		} else if (!mazeVector[0][y].isWall) {
			currentCoord.push_back({ 0, y });
			mazeVector[0][y].direction = RIGHT;
			mazeVector[0][y].isVisited = true;
			break;
		}

		if (x < mazeSizeHorizontal - 1)
			x++;
		if (y < mazeSizeVertical - 1)
			y++;
	}

	//since write_debug prints 0 for walls and it also prints pathID for other parts, pathID = 0 is not used 
	unsigned short iterationsPerm;
	
	while (!isFinished) {

		const unsigned short iterations = static_cast<unsigned short> (currentCoord.size());
		iterationsPerm = iterations;

		for (int i = 0; i < iterations;i++) {

			hasNeighbors = false;

			for (int DIR, j = 4; j > 0; j--) {
				DIR = 1 << (j-1);
				switch (DIR) {
					case UP:
						firstModifier = -1;
						secondModifier = 0;
						break;
					case DOWN:
						firstModifier = 1;
						secondModifier = 0;
						break;
					case LEFT:
						firstModifier = 0;
						secondModifier = -1;
						break;
					case RIGHT:
						firstModifier = 0;
						secondModifier = 1;
						break;
				}

				//Protection if -- checks for invalid vector coordinates
				if (static_cast<unsigned int>(currentCoord[i].first + firstModifier) < mazeSizeVertical 
						&& currentCoord[i].first + firstModifier >= 0 && static_cast<unsigned int>(currentCoord[i].second + secondModifier) < mazeSizeHorizontal
						&& currentCoord[i].second + secondModifier >= 0) {

					//Checks if wall is hit
					if ((mazeVector[currentCoord[i].first + firstModifier][currentCoord[i].second + secondModifier].isWall)
						&& (mazeVector[currentCoord[i].first][currentCoord[i].second].direction & DIR) == DIR) {

						//Only pushes back isPathIDDeadEnd vector if there isn't enough space
						while(static_cast<unsigned short>(isPathIDDeadEnd.size()) <= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID + 2)
							isPathIDDeadEnd.push_back(false);

						if ((DIR & VERTICAL) != 0) {
							if (currentCoord[i].second < mazeSizeHorizontal - 1 && !mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isWall) {
								mazeVector[currentCoord[i].first][currentCoord[i].second + 1].direction = RIGHT;
								mazeVector[currentCoord[i].first][currentCoord[i].second + 1].pathID
									= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID + 1;
								currentCoord.insert(currentCoord.begin() + iterations, {currentCoord[i].first, currentCoord[i].second + 1});
							}
							if (currentCoord[i].second > 0 && !mazeVector[currentCoord[i].first][currentCoord[i].second - 1].isWall) {
								mazeVector[currentCoord[i].first][currentCoord[i].second - 1].direction = LEFT;
								mazeVector[currentCoord[i].first][currentCoord[i].second - 1].pathID
									= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID + 2;
								currentCoord.insert(currentCoord.begin() + iterations, {currentCoord[i].first, currentCoord[i].second - 1});
							}

						} else if ((DIR & HORIZONTAL) != 0) {
							if (currentCoord[i].first < mazeSizeVertical - 1 && !mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isWall) {
								mazeVector[currentCoord[i].first + 1][currentCoord[i].second].direction = DOWN;
								mazeVector[currentCoord[i].first + 1][currentCoord[i].second].pathID
									= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID + 1;
								currentCoord.insert(currentCoord.begin() + iterations, {currentCoord[i].first + 1, currentCoord[i].second});
							}
							if (currentCoord[i].first > 0 && !mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isWall) {
								mazeVector[currentCoord[i].first - 1][currentCoord[i].second].direction = UP;
								mazeVector[currentCoord[i].first - 1][currentCoord[i].second].pathID
									= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID + 2;
								currentCoord.insert(currentCoord.begin() + iterations, {currentCoord[i].first - 1, currentCoord[i].second});
							}
						}
						
						changedPos.push_back({ currentCoord[i].first, currentCoord[i].second });

					//Now checks if vector below is visited IF AND ONLY IF it did not hit a wall (wall-specific routine below)
					//If it is, it pushes it onto the currentCoord vector, then edits its properties
					} else if (!(mazeVector[currentCoord[i].first + firstModifier][currentCoord[i].second + secondModifier].isVisited)) {
						currentCoord.push_back({ currentCoord[i].first + firstModifier, currentCoord[i].second + secondModifier });
						mazeVector[currentCoord[i].first + firstModifier][currentCoord[i].second + secondModifier].isVisited = true;
						mazeVector[currentCoord[i].first + firstModifier][currentCoord[i].second + secondModifier].pathID 
							= mazeVector[currentCoord[i].first][currentCoord[i].second].pathID;
						mazeVector[currentCoord[i].first + firstModifier][currentCoord[i].second + secondModifier].direction = DIR;
						hasNeighbors = true;

						changedPos.push_back({ currentCoord[i].first + firstModifier, currentCoord[i].second + secondModifier });
					}

				}
			}

			//Checks for finished status: if the coordinate either below or to the right is the size of the maze, that is the exit
			if (currentCoord[i].first >= mazeSizeHorizontal - 1 || currentCoord[i].second >= mazeSizeVertical - 1) {
				isFinished = true;
				break;
			}
			
			//Special routine: if the point we're checking doesn't have neighbors, sets that pathID as a dead end
			//Also writes debug for good measure
			if (!hasNeighbors)
				isPathIDDeadEnd[mazeVector[currentCoord[i].first][currentCoord[i].second].pathID] = true;

		}

		currentCoord.erase(currentCoord.begin(), currentCoord.begin() + iterations);
	}

	std::cout << "Finished!\n";

	write_debug(mazeSizeVertical - 1, mazeSizeHorizontal - 1);
	write_image();

	std::system("pause");
	return 0;
}

void write_debug_full() {
	
}

void write_debug(unsigned int maxColumn, unsigned int maxRow) {
	//Yes, I know the code looks like crap. Don't judge. I wrote this two  years ago (save for a couple changes here and there)
	//Good thing is, it just works fine (except when the maze size is above 1000 in one dimension, in which case it just breaks down lol)

	ofstream debug("debug.log", ios::out | ios::beg);
	
	for (int i = -4; (i < 0?(static_cast<unsigned int>(i) > maxRow): (static_cast<unsigned int>(i) < maxRow)); i++) {
		if (i < -1) {
			debug << "row ";
			for (unsigned int j = 0; j < maxColumn; j++) {
				debug << (j / static_cast<int>(pow(10,(abs(i) - 2)))) % 10;
			}
		} else if (i == -1) {
			debug << "col";
			for (unsigned int j = 0; j < maxColumn; j++) {
				debug << '_';
			}
		} else {
			for (int j = -2; (j < 0 ? (static_cast<unsigned int>(j) > maxColumn) : (static_cast<unsigned int>(j) < maxColumn)); j++) {
				if (j == -2) {
					debug << std::setw(3) << i;
				} else if (j == -1) {
					debug << '|';
				} else {
					if (mazeVector[i][j].isWall) {
						debug << 'W';
					} else if (mazeVector[i][j].isVisited) {
						debug << mazeVector[i][j].pathID % 10;
					} else {
						debug << 'E';
					}
				}
			}
		}
		debug << '\n';
	}
	return;
}

void write_image() {

	//Create new file
	string newname = fileDefaultName;
	newname.append(".result.bmp");
	ofstream result(newname, ios::out | ios::binary);

	


	for (int i = changedPos.size() - 1; i >= 0; i--) { 

		char *write_head = rawImageBlock + static_cast<unsigned char> (*(rawImageBlock + 10))
						+ (changedPos[i].first * (mazeSizeHorizontal * 3 + rawPadding)) + (changedPos[i].second * 3);

		if (mazeVector[changedPos[i].first][changedPos[i].second].isWall) {
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = 0x00;
			write_head++;
		}
		else if (isPathIDDeadEnd[mazeVector[changedPos[i].first][changedPos[i].second].pathID]) {
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = static_cast<unsigned char>(0xFF);
			write_head++;
		}
		else if (mazeVector[changedPos[i].first][changedPos[i].second].isVisited) {
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = static_cast<unsigned char>(0xFF);
			write_head++;
			*(write_head) = 0x00;
			write_head++;
		}
		else {
			*(write_head) = static_cast<unsigned char>(0xFF);
			write_head++;
			*(write_head) = static_cast<unsigned char>(0xFF);
			write_head++;
			*(write_head) = static_cast<unsigned char>(0xFF);
			write_head++;
		}
	}
	changedPos.clear();

	result.seekp(0, ios::beg);
	result.write(rawImageBlock, fileBinarySize);
	return;
}
