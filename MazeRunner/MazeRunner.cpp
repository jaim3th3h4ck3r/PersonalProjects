#include "pch.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <stack>

using namespace std;

struct COORD {
	bool isVisited = false;
	bool isWall = false;
	unsigned int pathID;
};

enum DIRECTION {
	UP		= 0b0001,
	DOWN	= 0b0010,
	LEFT	= 0b0100,
	RIGHT	= 0b1000
};


/*Insert all SUPPORTING VARIABLES here (ie. not critical to the algorithm)*/
//___________________________________________________________________________________________________________//
//Size of padding needed to complete .bmp file																 //
unsigned int rawPadding = 0;																				 //
//name of default image file; on same folder as executable													 //
string fileDefaultName = "maze.bmp";																		 //
//Pointer to image file in memory																			 //
//Only used to load file into memory for conversion into vector array and as base of final .bmp file		 //
char* rawImageBlock;																						 //
																											 //
/*CHILD FUNCTIONS*/																							 //
//writes vector array data to diagnostic .txt file															 //
void write_debug();																							 //
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
vector <bool> isPathIDDeadEnd;


int main(void){

	unsigned int fileBinarySize = 0;
	char* rawImagePointer;

	unsigned short iterations;
	short direction;
	unsigned short currentPathID;
	bool hasNeighbors;

	std::cout << "Enter file name: ";
	cin >> fileDefaultName;

	//Read image file and load it into memblock
	ifstream maze(fileDefaultName, ios::in | ios::binary | ios::ate);
	fileBinarySize = (unsigned int) maze.tellg();
	rawImageBlock = new char[fileBinarySize];
	maze.seekg(0, ios::beg);
	maze.read(rawImageBlock, fileBinarySize);
	maze.close();

	//Read the number of horizontal pixels from header, 18 bytes from start of file
	rawImagePointer = rawImageBlock + 18;
	mazeSizeHorizontal = 256 + (unsigned int) *(rawImagePointer);

	//Read the number of vertical pixels from header, 22 bytes from start of file
	rawImagePointer = rawImageBlock + 22;
	mazeSizeVertical = 256 + (unsigned int) *(rawImagePointer);

	//Parse memblock into int vector array
	rawImagePointer = rawImageBlock + (int) *(rawImageBlock + 10);					// Set pointer to start of image data
	fileBinarySize = fileBinarySize - (int) *(rawImageBlock + 10);					// Set new filesize to only size of image data
	rawPadding = (3 * mazeSizeHorizontal) % 4;

	//Resize coor to match 
	mazeVector.resize(mazeSizeVertical - 1);

	//Vector-writing loops
	//Row loop = y-coordinate
	for (unsigned int a = 0; a < mazeSizeVertical; a++) {
		mazeVector[a].resize(mazeSizeHorizontal - 1);

		//Column loop = x-coordinate
		for (unsigned int b = 0; b < mazeSizeHorizontal; b++) {

			//Location of data in order
			int rawOffset = a * mazeSizeHorizontal + b;
			if (*(rawImagePointer + (3 * rawOffset) + (a * rawPadding)) == 0) {
				mazeVector[a][b].isWall = true;
			}
		}
	}

	write_debug();

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
	deque < pair <unsigned int, unsigned int> > currentCoord;
	stack < pair <unsigned int, unsigned int> > recoveryPoint;

	bool isFinished = false;

	//Tries to find an entry point on the upper and left edges of the image
	for (unsigned int x = 0, y = 0; x < mazeSizeHorizontal - 1 || y < mazeSizeVertical - 1;) {
		if (!mazeVector[x][0].isWall) {
			currentCoord.push_back({ x, 1 });
			direction = DOWN;

			for (int i = x; !mazeVector[i][0].isWall; i++)
				mazeVector[i][0].isWall = true;

			break;
		} else if (!mazeVector[0][y].isWall) {
			currentCoord.push_back({ 1, y });
			direction = RIGHT;

			for (int i = y; !mazeVector[0][y].isWall; i++)
				mazeVector[0][i].isWall = true;

			break;
		}

		if (x < mazeSizeHorizontal - 1)
			x++;
		if (y < mazeSizeVertical - 1)
			y++;
	}

	currentPathID = 0;
	// Keep going until finished
	while (!isFinished) {
		iterations = (unsigned short) currentCoord.size();

		hasNeighbors = false;

		for (int i = 0; i < iterations;i++) {

			//DOWN CYCLE
			//Checks if coordinate above is a wall AND if the direction is DOWN (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if ((mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isWall) && (direction & DOWN) == DOWN) {
				recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
				currentPathID++;

			//Now checks if vector above is visited
			//If it is, it pushes it onto the currentCoord vector, then edits its properties
			} else if (!mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isVisited) {
				currentCoord.push_back({ currentCoord[i].first + 1, currentCoord[0].second });
				mazeVector[currentCoord[i].first + 1][currentCoord[i].second].isVisited = true;
				mazeVector[currentCoord[i].first + 1][currentCoord[i].second].pathID = currentPathID;
				hasNeighbors = true;

			}

			//UP CYCLE
			//Checks if coordinate above is a wall AND if the direction is UP (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if ((mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isWall) && (direction & UP) == UP) {
				recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
				currentPathID++;

			//Now checks if vector above is visited
			//If it is, it pushes it onto the currentCoord vector, then edits its properties
			}
			else if (!mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited) {
				currentCoord.push_back({ currentCoord[0].first - 1, currentCoord[0].second });
				mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited = true;
				mazeVector[currentCoord[i].first - 1][currentCoord[i].second].pathID = currentPathID;
				hasNeighbors = true;

			}

			//RIGHT CYCLE
			//Checks if coordinate above is a wall AND if the direction is DOWN (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if ((mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isWall) && (direction & RIGHT) == RIGHT) {
				recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
				currentPathID++;

			//Now checks if vector above is visited
			//If it is, it pushes it onto the currentCoord vector, then edits its properties
			} else if (!mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isVisited) {
				currentCoord.push_back({ currentCoord[i].first, currentCoord[i].second + 1 });
				mazeVector[currentCoord[i].first][currentCoord[i].second + 1].isVisited = true;
				mazeVector[currentCoord[i].first][currentCoord[i].second + 1].pathID = currentPathID;
				hasNeighbors = true;

			}

			//LEFT CYCLE
			//Checks if coordinate above is a wall AND if the direction is UP (for preventing false negatives eg. when going along a wall)
			//And if so pushes the coordinates onto the stack for later retrieval
			if ((mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isWall) && (direction & LEFT) == LEFT) {
				recoveryPoint.push({ currentCoord[i].first, currentCoord[i].second });
				currentPathID++;

			//Now checks if vector above is visited
			//If it is, it pushes it onto the currentCoord vector, then edits its properties
			} else if (!mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited) {
				currentCoord.push_back({ currentCoord[i].first - 1, currentCoord[i].second });
				mazeVector[currentCoord[i].first - 1][currentCoord[i].second].isVisited = true;
				mazeVector[currentCoord[i].first - 1][currentCoord[i].second].pathID = currentPathID;
				hasNeighbors = true;

			}

			//Checks for finished status: if the coordinate either below or to the right is the size of the maze, that is the exit
			if (currentCoord[i].first + 1 >= mazeSizeHorizontal || currentCoord[i].second + 1 >= mazeSizeVertical) {
				isFinished = true;
				break;
			}

		}

		//Special routine if it doesn't have neighbors, going back to the last recovery point
		//and loading that into currentCoord, then resetting the while loop
		if (!hasNeighbors) {
			isPathIDDeadEnd[currentPathID] = true;
			currentCoord.clear();
			currentCoord.push_back(recoveryPoint.top());
			recoveryPoint.pop();
			break;
		}

		currentCoord.erase(currentCoord.begin(), currentCoord.begin() + (iterations - 1));
	}

	std::cout << "Finished!";
	write_debug();
	write_image();
	return 0;
}





void write_debug() {
	ofstream debug("debug.txt", ios::out | ios::ate);
	for (unsigned int i = 0; i < (mazeSizeVertical); i++) {									// Row loop
		for (unsigned int j = 0; j < (mazeSizeHorizontal); j++) {								// Column loop
			debug << abs(mazeVector[i][j].isWall);
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

	//Set new filesize to only size of image data
	int header_size = *(rawImageBlock + 10);

	//Write header from input file to output file
	result.write(rawImageBlock, header_size);

	//Start image writing
	int write_head = 0;
	result.seekp(0, ios::end);
	char pad_data = 0;

	//Row loop = y-coordinate
	for (unsigned int x = 0; x < (mazeSizeVertical); x++) {

		//Column loop = x-coordinate
		for (unsigned int y = 0; y < (mazeSizeHorizontal); y++) {
			if (mazeVector[x][y].isWall) {
				write_head = 0x000000;
			}
			else if (isPathIDDeadEnd[mazeVector[x][y].pathID]) {
				write_head = 0xFF0000;
			}
			else if (mazeVector[x][y].isVisited) {
				write_head = 0x00FF00;
			}
			else {
				write_head = 0x000000;
			}
			result.write((char*)&write_head, 3);
		}
		result.write(&pad_data, rawPadding);
	}
	return;
}