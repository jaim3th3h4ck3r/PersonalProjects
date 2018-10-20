#include "pch.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <stack>

using namespace std;

enum DIRECTION {
	UP = 0b0001,
	DOWN = 0b0010,
	LEFT = 0b0100,
	RIGHT = 0b1000
};

struct COORD {
	bool isVisited = false;
	const bool isWall = false;
	unsigned char direction = 0b0000;
	unsigned short pathID = 0;
	COORD(bool T) :isWall(T), isVisited(T) {};
};




/*Insert all SUPPORTING VARIABLES here (ie. not critical to the algorithm)*/								 //
//___________________________________________________________________________________________________________//
//Size of padding needed to complete .bmp file																 //
unsigned int rawPadding = 0;																				 //
//name of default image file; on same folder as executable													 //
string fileDefaultName = "maze.bmp";																		 //
//Pointer to image file in memory																			 //
//Only used to load file into memory for conversion into vector array and as base of final .bmp file		 //
char* rawImageBlock;																						 //
int fileBinarySize;																						   	 //
//Vector of changed positions for speeding up image writing operations
vector<pair <unsigned int, unsigned int>> changedPos;
																											 //
/*CHILD FUNCTIONS*/																							 //
//writes vector array data to diagnostic .txt file															 //
void write_debug(int maxColumn, int maxRow);																 //
//writes vector array to final pretty .bmp image															 //
void write_image();																							 //
//___________________________________________________________________________________________________________//



//Size of maze pixels to navigate
unsigned int mazeSizeHorizontal, mazeSizeVertical;

//2D vector, used for positioning
//Global because writing functions use it for writing data
//First value: column/x-coordinate
//Second value: row/y-coordinate
//REMEMBER: [0][0] is UPPER LEFT corner
vector < vector <COORD> > mazeVector;

//Vector for pathID verification
//If a certain pathID is a dead end, it's vector coordinate is set to true
vector <bool> isPathIDDeadEnd{ false };


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
					mazeVector[a].push_back(COORD(true));
				} else {
					mazeVector[a].push_back(COORD(false));
				}
			}
		}

		write_debug(mazeSizeHorizontal - 1, mazeSizeVertical - 1);
		std::system("pause");
	}
	// 2D vector complete
	// Start solving algorithm

	/*
	The new algorithm is based on simply 'filling up' the maze until we find the exit.

	The algorithm works by simply marking its neighbors as being visited, and then moving 
		onto said neighbors until it hits a wall. Each pixel is of a COORD structure, and
		the only prefilled data is the isWall value for walls; everything else is empty.
		
	Each time the algorithm hits a wall, it places its current position on a stack and then
		goes on and fills it to one side of the maze with a different pathID. If it is
		a dead end, then the algorith returns to the last coordinate on the stack; if that
		is a dead end too, it'll pop that value off the stack and read the previous value,
		and so forth until it finds a suitable point to continue.
	*/

	//first: column/x-coordinate
	//second: row/y-coordinate

	//unsigned short iterations;
	unsigned short currentPathID;
	bool hasNeighbors;
	bool isFinished = false;
	deque < pair <unsigned short, unsigned short> > currentCoord;
	stack < pair <unsigned short, unsigned short> > recoveryPoint;



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
	currentPathID = 1;
	isPathIDDeadEnd.push_back(true);

	while (!isFinished) {

		const unsigned short iterations = (unsigned short) currentCoord.size();
		hasNeighbors = false;

		for (int i = 0; i < iterations;i++) {

			//DOWN CYCLE
			//Checks if coordinate below is a wall AND if the direction is DOWN (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if (currentCoord[i].first < mazeSizeVertical - 1) {
				if ((mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isWall) && (mazeVector[currentCoord[i].first][currentCoord[i].second].direction & DOWN) == DOWN) {
					recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
					isPathIDDeadEnd.push_back(false);
					currentPathID++;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second });

					//Now checks if vector below is visited
					//If it is, it pushes it onto the currentCoord vector, then edits its properties
				} else if (!mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isVisited) {
					currentCoord.push_back({ currentCoord[i].first + 1, currentCoord[0].second });
					mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isVisited = true;
					mazeVector[currentCoord[i].first + 1][currentCoord[i].second].pathID = currentPathID;
					mazeVector[currentCoord[i].first + 1][currentCoord[i].second].direction = DOWN;
					hasNeighbors = true;

					changedPos.push_back({ currentCoord[i].first + 1, currentCoord[i].second });
				}
			}
			

			//UP CYCLE
			//Checks if coordinate above is a wall AND if the direction is UP (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if (currentCoord[i].first > 0) {
				if ((mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isWall) && (mazeVector[currentCoord[i].first][currentCoord[i].second].direction & UP) == UP) {
					recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
					isPathIDDeadEnd.push_back(false);
					currentPathID++;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second });

					//Now checks if vector above is visited
					//If it is, it pushes it onto the currentCoord vector, then edits its properties
				}
				else if (!mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited) {
					currentCoord.push_back({ currentCoord[0].first - 1, currentCoord[0].second });
					mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited = true;
					mazeVector[currentCoord[i].first - 1][currentCoord[i].second].pathID = currentPathID;
					mazeVector[currentCoord[i].first - 1][currentCoord[i].second].direction = UP;
					hasNeighbors = true;

					changedPos.push_back({ currentCoord[i].first - 1, currentCoord[i].second });

				}
			}
			

			//RIGHT CYCLE
			//Checks if coordinate to the right is a wall AND if the direction is RIGHT (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if (currentCoord[i].second < mazeSizeHorizontal - 1) {
				if ((mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isWall) && (mazeVector[currentCoord[i].first][currentCoord[i].second].direction & RIGHT) == RIGHT) {
					recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
					isPathIDDeadEnd.push_back(false);
					currentPathID++;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second });

					//Now checks if vector to the right is visited
					//If it is, it pushes it onto the currentCoord vector, then edits its properties
				}
				else if (!mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isVisited) {
					currentCoord.push_back({ currentCoord[i].first, currentCoord[i].second + 1 });
					mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isVisited = true;
					mazeVector[currentCoord[i].first][currentCoord[i].second + 1].pathID = currentPathID;
					mazeVector[currentCoord[i].first][currentCoord[i].second + 1].direction = RIGHT;
					hasNeighbors = true;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second + 1 });

				}
			}
			

			//LEFT CYCLE
			//Checks if coordinate to the left is a wall AND if the direction is LEFT (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if (currentCoord[i].second > 0) {
				if ((mazeVector[currentCoord[i].first][currentCoord[i].second - 1].isWall) && (mazeVector[currentCoord[i].first][currentCoord[i].second].direction & LEFT) == LEFT) {
					recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
					isPathIDDeadEnd.push_back(false);
					currentPathID++;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second });

					//Now checks if vector above is visited
					//If it is, it pushes it onto the currentCoord vector, then edits its properties
				}else if (!mazeVector[currentCoord[i].first][currentCoord[i].second - 1].isVisited) {
					currentCoord.push_back({ currentCoord[i].first, currentCoord[i].second - 1});
					mazeVector[currentCoord[i].first][currentCoord[i].second - 1].isVisited = true;
					mazeVector[currentCoord[i].first][currentCoord[i].second - 1].pathID = currentPathID;
					mazeVector[currentCoord[i].first][currentCoord[i].second - 1].direction = LEFT;
					hasNeighbors = true;

					changedPos.push_back({ currentCoord[i].first, currentCoord[i].second - 1 });

				}
			}

			//Checks for finished status: if the coordinate either below or to the right is the size of the maze, that is the exit
			if (currentCoord[i].first >= mazeSizeHorizontal - 1 || currentCoord[i].second >= mazeSizeVertical - 1) {
				isFinished = true;
				break;
			}

		}

		//Special routine if it doesn't have neighbors, going back to the last recovery point
		//and loading that into currentCoord, then resetting the while loop
		if (!hasNeighbors) {
			write_debug(mazeSizeVertical, mazeSizeHorizontal);
			isPathIDDeadEnd[currentPathID] = true;
			currentCoord.clear();
			currentCoord.push_back(recoveryPoint.top());
			recoveryPoint.pop();
			break;
		} else {
			write_debug(50, 40);
		}

		currentCoord.erase(currentCoord.begin(), currentCoord.begin() + (iterations - 1));
	}

	std::cout << "Finished!\n";

	write_debug(mazeSizeVertical - 1, mazeSizeHorizontal - 1);
	write_image();

	std::system("pause");
	return 0;
}

void write_debug(int maxColumn, int maxRow) {
	//Yes, I know the code looks like crap. Don't judge.
	//I wrote this two  years ago (save for a couple changes here and there)
	//Good thing is, it just works fine
	//(except when the maze size is above 1000 in one dimension, in which case it just breaks down lol)

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

	// Format: (unused), R, G, B
	char* write_head;


	for (int i = changedPos.size() - 1; i >= 0; i--) {
		write_head = rawImageBlock + static_cast<unsigned char> (*(rawImageBlock + 10))
								   + (changedPos[i].first * (mazeSizeHorizontal * 3 + rawPadding))
								   + (changedPos[i].second * 3);
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
			*(write_head) = 0xFF;
			write_head++;
		}
		else if (mazeVector[changedPos[i].first][changedPos[i].second].isVisited) {
			*(write_head) = 0x00;
			write_head++;
			*(write_head) = 0xFF;
			write_head++;
			*(write_head) = 0x00;
			write_head++;
		}
		else {
			*(write_head) = 0xFF;
			write_head++;
			*(write_head) = 0xFF;
			write_head++;
			*(write_head) = 0xFF;
			write_head++;
		}
	}
	changedPos.clear();

	result.seekp(0, ios::beg);
	result.write(rawImageBlock, fileBinarySize);
	return;
}