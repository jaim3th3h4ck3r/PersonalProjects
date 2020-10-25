#include "pch.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <conio.h>

#include <string>
#include <vector>
#include <deque>
#include <bitset>

#include <chrono>
#include <thread>
#include <mutex>
#include <windows.h>

#include <stdexcept>


enum DIRECTION {
	UP		= 0b0001,
	DOWN	= 0b0010,
	LEFT	= 0b0100,
	RIGHT	= 0b1000,

	VERTICAL	= 0b0011,
	HORIZONTAL	= 0b1100,

	ALL_DIR		= 0b1111
};

struct COORD_MAZE {
	bool isVisited = false;
	const bool isWall = false;
	unsigned char direction = 0b0000;
	unsigned short pathID = 1;
	explicit COORD_MAZE(bool T): isVisited(T), isWall(T) {}
};


/*Insert all SUPPORTING VARIABLES here (ie. not critical to the algorithm)*/
//Size of padding needed to complete .bmp file
unsigned int rawPadding = 0;
//name of default image file; on same folder as executable
std::wstring fileDefaultName = L"maze_inv.bmp";
//Pointer to image file in memory
//Only used to load file into memory for conversion into vector array and as base of final .bmp file
char* rawImageBlock;
int fileBinarySize;
//Vector of changed positions for speeding up image writing operations
std::vector<std::pair <unsigned int, unsigned int>> changedPos;
//String for error throwing
std::string errorString;


//Size of maze pixels to navigate
std::pair <unsigned int, unsigned int> mazeNumber;

//2D vector, used for positioning
//Global because writing functions use it for writing data
//First value: column/x-coordinate
//Second value: row/y-coordinate
//REMEMBER: [0][0] is [UPPER][LEFT] corner
std::vector < std::vector <COORD_MAZE> > mazeVector;
//Vector for bad pathID verification
//If a certain pathID is a dead end, it's vector coordinate is set to true
std::vector <bool> isPathIDDeadEnd{ true };

//Function for checking whether a certain coordinate is within the specified limits of the maze
bool isCoordValid(unsigned int firstCoord, unsigned int secondCoord) {
	return firstCoord < static_cast<signed int>(mazeNumber.first) && secondCoord < static_cast<signed int>(mazeNumber.second);
}
bool isCoordValid(std::pair <unsigned int, unsigned int> pair) {
	return pair.first < static_cast<signed int>(mazeNumber.first) && pair.second < static_cast<signed int>(mazeNumber.second);
}


// Writing functions
#include "Writers.h"



int wmain(int argc, wchar_t** argv){
	//Image loading block
	//It's in a block because I don't wanna see it while editing the solving loop :P
	{
		char* rawImagePointer;

		//Read image file and load it into memblock
		bool isFileGood;
		std::ifstream maze;

		//The file can be read as an argument or can be asked to the user
		if (argc == 1) {
			do {
				isFileGood = true;

				std::wcout << L"Enter file name: ";
				std::wcin >> fileDefaultName;

				maze.open(fileDefaultName, std::ios::in | std::ios::binary | std::ios::ate);
				if (!maze.is_open() || maze.bad()) {
					std::wcerr << L"Error opening file. Try again." << std::endl;
					isFileGood = false;
				}
			} while (!isFileGood);

		} else {
			fileDefaultName = argv[1];

			do {
				isFileGood = true;

				maze.open(fileDefaultName, std::ios::in | std::ios::binary | std::ios::ate);
				if (!maze.is_open() || maze.bad()) {
					std::wcerr << L"Error opening file. Try again." << std::endl;
					isFileGood = false;

					std::wcout << L"Enter file name: ";
					std::wcin >> fileDefaultName;
				} else {
					std::wcout << L"File: " << fileDefaultName << std::endl;
				}
				
			} while (!isFileGood);
		}

		fileBinarySize = static_cast<unsigned int>(maze.tellg());
		rawImageBlock = new char[fileBinarySize];
		maze.seekg(0, std::ios::beg);
		maze.read(rawImageBlock, fileBinarySize);
		maze.close();

		//Read the number of horizontal pixels from header, 18 bytes from start of file
		rawImagePointer = rawImageBlock + 18;
		mazeNumber.first = 256 + static_cast<unsigned int> (*(rawImagePointer));

		//Read the number of vertical pixels from header, 22 bytes from start of file
		rawImagePointer = rawImageBlock + 22;
		mazeNumber.second = 256 + static_cast<unsigned int>(*(rawImagePointer));

		//Parse memblock into int vector array
		rawImagePointer = rawImageBlock + static_cast<int>(*(rawImageBlock + 10));					//Set pointer to start of image data
		rawPadding = (3 * mazeNumber.first) % 4;

		//Resize coor to match maze dimensions
		try {
			mazeVector.resize(mazeNumber.second);

			//Vector-writing loops
			//Row loop = y-coordinate
			for (unsigned int a = 0; a < mazeNumber.second; a++) {

				mazeVector[a].reserve(mazeNumber.first);

				//Column loop = x-coordinate
				for (unsigned int b = 0; b < mazeNumber.first; b++) {

					//Location of data in order
					int rawOffset = a * mazeNumber.first + b;
					if (*(rawImagePointer + (3 * rawOffset) + (a * rawPadding)) == 0) {
						mazeVector[a].emplace_back(true);
					} else {
						mazeVector[a].emplace_back(false);
					}
				}
			}
		} catch (std::bad_alloc &e) {
			std::wcerr << L"ERROR: Bad vector allocation -- " << e.what();
			return -1;
		}

		//Write image as a test
		write_image();
	}

	//Set up the console
	cmd_console console(mazeNumber);

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

	//True if maze is solved
	bool isFinished = false;
	//Vector for checking whether a certain coordinate has a neighbor
	//Use the bitmask enum for direction (UP, DOWN, LEFT, RIGHT)
	std::bitset <4> hasNeighbor;
	//Direction of the "wave", implemented using bitmask
	unsigned int DIR = 0;
	//How many coordinates on the currentCoord vector
	unsigned int iterations;
	//When checking each coordinate on currentCoord, a subloop checks the adjacent coordinates. This is the coordinate of the adjacent coordinate
	std::pair <unsigned int, unsigned int> adjacentCoord;
	//Vector with all coordinates we're checking now; new ones are added to newCoord
	std::vector < std::pair <unsigned short, unsigned short> > currentCoord;
	std::vector < std::pair <unsigned short, unsigned short> > newCoord;

	//Recovery queue for when a dead-end is reached
	//Higher priority (ie. wall hit events) gets pushed onto the front
	//Lower priority (ie. everything else) gets pushed onto the back
	std::deque < std::pair <unsigned short, unsigned short> > recoveryPoint;

	std::chrono::steady_clock::time_point solveTimeBegin = std::chrono::steady_clock::now();

	//Tries to find an entry point on the upper and left edges of the image
	for (unsigned short x = 0, y = 0; x < mazeNumber.first - 1 || y < mazeNumber.second - 1;) {
		if (!mazeVector[x][0].isWall) {
			currentCoord.emplace_back(x, 0);
			mazeVector[x][0].direction = ALL_DIR;
			mazeVector[x][0].isVisited = true;
			x = 0xFFFF;
			y = 0xFFFF;

		} else if (!mazeVector[0][y].isWall) {
			currentCoord.emplace_back(0, y);
			mazeVector[0][y].direction = ALL_DIR;
			mazeVector[0][y].isVisited = true;
			x = 0xFFFF;
			y = 0xFFFF;
		}

		if (x < mazeNumber.first - 1) {
			x++;
		}
		if (y < mazeNumber.second - 1) {
			y++;
		}
	}

	try{

		isPathIDDeadEnd.emplace_back(false);

		//Main solving loop
		while (!isFinished) {

			//Write console then wait for input to advance
			console.write_console();
			//void) _getch();

			if (!newCoord.empty()) {
				currentCoord.swap(newCoord);
				newCoord.clear();
			}

			if (currentCoord.empty()) {
				if (recoveryPoint.empty()) {
					throw std::length_error("Queue and recovery stack are empty. Refill with premium gas only.");
				}
				currentCoord.push_back(recoveryPoint.front());
				recoveryPoint.pop_front();
			}

			iterations = static_cast <unsigned int> (currentCoord.size());

			//Runs for each of the coordinates in currentCoord,
			//loopCoord is the coordinate the loop is checking
			for (auto& loopCoord : currentCoord) {

				hasNeighbor[UP] = false;
				hasNeighbor[DOWN] = false;
				hasNeighbor[LEFT] = false;
				hasNeighbor[RIGHT] = false;

				DIR = 0;

				//Runs four times for each coordinate, one for each direction
				for (int j = 4; j > 0; j--) {

					console.write_console();

					//Conditional DIR assignment
					//After every normal loop iteration DIR is reset to 0
					if (DIR == 0) {
						DIR = 1 << (j - 1);
					}

					//Sets which neighboring tile to check based on the loopCoord tile, which is gathered from the currentCoord vector
					switch (DIR) {
						case UP:
							adjacentCoord.first = loopCoord.first - 1;
							adjacentCoord.second = loopCoord.second;
							break;
						case DOWN:
							adjacentCoord.first = loopCoord.first + 1;
							adjacentCoord.second = loopCoord.second;
							break;
						case LEFT:
							adjacentCoord.first = loopCoord.first;
							adjacentCoord.second = loopCoord.second - 1;
							break;
						case RIGHT:
							adjacentCoord.first = loopCoord.first;
							adjacentCoord.second = loopCoord.second + 1;
							break;
						default:
							errorString = "Bad DIR value: " + std::to_string(DIR) + "\n4D travel not allowed";
							throw std::out_of_range(errorString);
					}


					//Protection -- checks for invalid vector coordinates (above image pixels), throws an exception if one of them is
					if (!isCoordValid(adjacentCoord)) {
						errorString = "Bad maze coordinates: (" + std::to_string(adjacentCoord.first) + ", " + std::to_string(adjacentCoord.second) + ")\nRemember: The Minotaur must not escape. Do not undermine our efforts.";
						throw std::out_of_range(errorString);
					}
					

					if (adjacentCoord.first >= static_cast<signed int>(0) && adjacentCoord.second >= static_cast<signed int>(0)) {

						//Checks if wall is hit
						if ((mazeVector[adjacentCoord.first][adjacentCoord.second].isWall) && (mazeVector[loopCoord.first][loopCoord.second].direction & DIR) == DIR) {

							if ((DIR & VERTICAL) != 0) {
								if (loopCoord.second < mazeNumber.first - 1 && !mazeVector[loopCoord.first][loopCoord.second + 1].isWall) {
									mazeVector[loopCoord.first][loopCoord.second + 1].isVisited = true;
									mazeVector[loopCoord.first][loopCoord.second + 1].direction = RIGHT;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[loopCoord.first][loopCoord.second + 1].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborPos = true;

								}
								if (loopCoord.second > 0 && !mazeVector[loopCoord.first][loopCoord.second - 1].isWall) {
									mazeVector[loopCoord.first][loopCoord.second - 1].isVisited = true;
									mazeVector[loopCoord.first][loopCoord.second - 1].direction = LEFT;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[loopCoord.first][loopCoord.second - 1].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborNeg = true;
								}

								//Conditional loading into currentCoord and recoveryPoint
								//Only one is loaded into currentCoord; if both upper and lower exist,
								//lower is instead loaded into recoveryPoint
								newCoord.clear();
								if (hasNeighbor[UP] && hasNeighbor[DOWN]) {
									newCoord.emplace_back(loopCoord.first, loopCoord.second + 1);
									recoveryPoint.emplace_front(loopCoord.first, loopCoord.second - 1);
								} else if (hasNeighbor[UP]) {
									newCoord.emplace_back(loopCoord.first, loopCoord.second + 1);
								} else if (hasNeighbor[DOWN]) {
									newCoord.emplace_back(loopCoord.first, loopCoord.second - 1);
								}

							} else if ((DIR & HORIZONTAL) != 0) {
								if (loopCoord.first < mazeNumber.second - 1 && !mazeVector[loopCoord.first + 1][loopCoord.second].isWall) {
									mazeVector[loopCoord.first + 1][loopCoord.second].isVisited = true;
									mazeVector[loopCoord.first + 1][loopCoord.second].direction = DOWN;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[loopCoord.first + 1][loopCoord.second].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborPos = true;

								}
								if (loopCoord.first > 0 && !mazeVector[loopCoord.first - 1][loopCoord.second].isWall) {
									mazeVector[loopCoord.first - 1][loopCoord.second].isVisited = true;
									mazeVector[loopCoord.first - 1][loopCoord.second].direction = UP;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[loopCoord.first - 1][loopCoord.second].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborNeg = true;
								}

								//Conditional loading into currentCoord and recoveryPoint
								//Only one is loaded into currentCoord; if both upper and lower exist,
								//lower is instead loaded into recoveryPoint
								newCoord.clear();
								if (hasNeighborNeg && hasNeighborPos) {
									newCoord.emplace_back(loopCoord.first + 1, loopCoord.second);
									recoveryPoint.emplace_front(loopCoord.first - 1, loopCoord.second);
								} else if (hasNeighborPos) {
									newCoord.emplace_back(loopCoord.first + 1, loopCoord.second);
								} else if (hasNeighborNeg) {
									newCoord.emplace_back(loopCoord.first - 1, loopCoord.second);
								}
							}

							changedPos.emplace_back(loopCoord.first, loopCoord.second);

							//Special routine: if the point we're checking doesn't have neighbors, sets that pathID as a dead end.
							//then clears the vectors and writes the new coordinate to newCoord
							if (!(hasNeighborPos || hasNeighborNeg)) {
								isPathIDDeadEnd[mazeVector[loopCoord.first][loopCoord.second].pathID] = true;
								currentCoord.clear();
								newCoord.clear();
								newCoord.push_back(recoveryPoint.front());
								recoveryPoint.pop_front();
							} else {
								DIR = 0;
							}

							//Set up the exit conditions
							iterations = 0;
							j = 0;
							break;



							//Now checks if vector below is visited IF AND ONLY IF it did not hit a wall (wall-specific routine below)
							//If it is, it pushes it onto the currentCoord vector, then edits its properties
						} else if (!(mazeVector[adjacentCoord.first][adjacentCoord.second].isVisited)) {
							newCoord.emplace_back(adjacentCoord.first, adjacentCoord.second);
							mazeVector[adjacentCoord.first][adjacentCoord.second].isVisited = true;
							mazeVector[adjacentCoord.first][adjacentCoord.second].pathID 
								= mazeVector[loopCoord.first][loopCoord.second].pathID;
							mazeVector[adjacentCoord.first][adjacentCoord.second].direction = static_cast<unsigned char>(DIR);

							changedPos.emplace_back(adjacentCoord.first, adjacentCoord.second);

							DIR = 0;
							hasNeighborNormal = true;

						}

					}
					
					//DO NOT DELETE
					DIR = 0;

				}

				if (iterations == 0) {
					break;
				}

				//Special routine: if the point we're checking doesn't have neighbors, sets that pathID as a dead end.
				//then clears the vectors and writes the new coordinate to newCoord
				if (!hasNeighborNormal && !recoveryPoint.empty()) {
					isPathIDDeadEnd[mazeVector[loopCoord.first][loopCoord.second].pathID] = true;
					currentCoord.clear();
					newCoord.clear();
					newCoord.push_back(recoveryPoint.front());
					recoveryPoint.pop_front();
				} else {
					DIR = 0;
				}

				//Checks for finished status: if the coordinate either below or to the right is the size of the maze, that is the exit
				if (loopCoord.first >= mazeNumber.first - 1 || loopCoord.second >= mazeNumber.second - 1) {
					isFinished = true;
					break;
				}

			}

		}
		
		std::chrono::steady_clock::time_point solveTimeEnd = std::chrono::steady_clock::now();
		std::chrono::duration<long int, std::milli> solveTime = std::chrono::duration_cast<std::chrono::duration<long int, std::milli>>(solveTimeEnd - solveTimeBegin);

		std::cout << "Finished!\nTime needed to solve the maze: " << solveTime.count();

	} catch (const std::out_of_range &e) {
		std::cerr << std::endl << "ERROR: " << e.what() << std::endl << "Exiting program...\n";
	} catch (const std::length_error &e) {
		std::cerr << std::endl << "ERROR: " << e.what() << std::endl << "Exiting program...\n";
	}
	
	write_image();

	std::system("pause");
	return 0;
}