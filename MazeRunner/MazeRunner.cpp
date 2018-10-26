#include "pch.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <conio.h>

#include <string>
#include <vector>
#include <deque>

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
//_____________________________________________________________________________________________________________
//Size of padding needed to complete .bmp file																 //
unsigned int rawPadding = 0;																				 //
//name of default image file; on same folder as executable													 //
std::wstring fileDefaultName = L"maze_inv.bmp";																 //
//Pointer to image file in memory																			 //
//Only used to load file into memory for conversion into vector array and as base of final .bmp file		 //
char* rawImageBlock;																						 //
int fileBinarySize;																						   	 //
//Vector of changed positions for speeding up image writing operations										 //
std::vector<std::pair <unsigned int, unsigned int>> changedPos;												 //
//String for error throwing																					 //
std::string errorString;																					 //
//Mutex and condition variable
std::mutex debug;
std::condition_variable isDebugComplete;
																											 //
/*CHILD FUNCTIONS*/																							 //
//writes vector array data to diagnostic .txt file															 //
void write_debug(unsigned int maxColumn = 256, unsigned int maxRow = 256);									 //
//writes vector array to final pretty .bmp image															 //
void write_image();																							 //
//Writes vector to second console																			 //
void write_console(const unsigned short maxColumn, const unsigned short maxRow);								 //
//___________________________________________________________________________________________________________//



//Size of maze pixels to navigate
unsigned short mazeNumberRows, mazeNumberColumns;

//2D vector, used for positioning
//Global because writing functions use it for writing data
//First value: column/x-coordinate
//Second value: row/y-coordinate
//REMEMBER: [0][0] is [UPPER][LEFT] corner
std::vector < std::vector <COORD_MAZE> > mazeVector;

//Vector for bad pathID verification
//If a certain pathID is a dead end, it's vector coordinate is set to true
std::vector <bool> isPathIDDeadEnd{ true };

int wmain(int argc, wchar_t** argv){
	{
		char* rawImagePointer;

		//Read image file and load it into memblock
		bool isFileGood;
		std::ifstream maze;

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
		mazeNumberRows = 256 + static_cast<unsigned int> (*(rawImagePointer));

		//Read the number of vertical pixels from header, 22 bytes from start of file
		rawImagePointer = rawImageBlock + 22;
		mazeNumberColumns = 256 + static_cast<unsigned int>(*(rawImagePointer));

		//Parse memblock into int vector array
		rawImagePointer = rawImageBlock + static_cast<int>(*(rawImageBlock + 10));					// Set pointer to start of image data
		rawPadding = (3 * mazeNumberRows) % 4;

		//Resize coor to match maze dimensions
		try {
			mazeVector.resize(mazeNumberColumns);

			//Vector-writing loops
			//Row loop = y-coordinate
			for (unsigned int a = 0; a < mazeNumberColumns; a++) {

				mazeVector[a].reserve(mazeNumberRows);

				//Column loop = x-coordinate
				for (unsigned int b = 0; b < mazeNumberRows; b++) {

					//Location of data in order
					int rawOffset = a * mazeNumberRows + b;
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
		write_debug(mazeNumberColumns, mazeNumberRows);
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

	bool isFinished = false;
	bool hasNeighborPos, hasNeighborNeg;
	bool hasNeighborNormal;
	unsigned int DIR = 0;
	unsigned short iterations;
	signed int firstModifier = 0, secondModifier = 0;
	std::vector < std::pair <unsigned short, unsigned short> > currentCoord;
	std::vector < std::pair <unsigned short, unsigned short> > newCoord;

	//Recovery queue for when a deadend is reached
	//Higher priority (ie. wall hit events) gets pushed onto the front
	//Lower priority (ie. everuthing else) gets pushed onto the back
	std::deque < std::pair <unsigned short, unsigned short> > recoveryPoint;

	std::chrono::steady_clock::time_point solveTimeBegin = std::chrono::steady_clock::now();

	//Tries to find an entry point on the upper and left edges of the image
	for (unsigned short x = 0, y = 0; x < mazeNumberRows - 1 || y < mazeNumberColumns - 1;) {
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

		if (x < mazeNumberRows - 1) {
			x++;
		}
		if (y < mazeNumberColumns - 1) {
			y++;
		}
	}

	std::thread printDebug (write_console, mazeNumberColumns, mazeNumberRows);
	printDebug.detach();

	try{

		isPathIDDeadEnd.emplace_back(false);

		while (!isFinished) {

			std::unique_lock<std::mutex> lock(debug);
			isDebugComplete.wait(lock);

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

			iterations = static_cast<unsigned short> (currentCoord.size());

			hasNeighborNormal = false;

			for (auto& iter : currentCoord) {

				hasNeighborPos = false;
				hasNeighborNeg = false;

				DIR = 0;

				for (int j = 4; j > 0; j--) {

					//Conditional DIR assignment
					//After every normal loop iteration DIR is reset to 0
					if (DIR == 0) {
						DIR = 1 << (j - 1);
					}
					
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
						default:
							errorString = "Bad DIR value: " + std::to_string(DIR) + "\n4D travel not allowed";
							throw std::out_of_range(errorString);
					}

					//Protection 'if' -- checks for invalid vector coordinates, throws an exception if one of them is
					if (static_cast<signed int>(iter.first) + firstModifier >= static_cast<signed int>(mazeNumberColumns)
							|| static_cast<signed int>(iter.second) + secondModifier >= static_cast<signed int>(mazeNumberRows)) { 
						errorString = "Bad maze coordinates: (" + std::to_string(iter.first + firstModifier) + ", " 
							+ std::to_string(iter.second + secondModifier) + ")\nRemember: The Minotaur must not escape. Do not undermine our efforts.";
						throw std::out_of_range(errorString);
					}
					
					if (static_cast<signed int>(iter.first + firstModifier) >= static_cast<signed int>(0)
							&& static_cast<signed int>(iter.second + secondModifier) >= static_cast<signed int>(0)) {
						//Checks if wall is hit
						if ((mazeVector[iter.first + firstModifier][iter.second + secondModifier].isWall)
							&& (mazeVector[iter.first][iter.second].direction & DIR) == DIR) {

							if ((DIR & VERTICAL) != 0) {
								if (iter.second < mazeNumberRows - 1 && !mazeVector[iter.first][iter.second + 1].isWall) {
									mazeVector[iter.first][iter.second + 1].isVisited = true;
									mazeVector[iter.first][iter.second + 1].direction = RIGHT;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[iter.first][iter.second + 1].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborPos = true;

								}
								if (iter.second > 0 && !mazeVector[iter.first][iter.second - 1].isWall) {
									mazeVector[iter.first][iter.second - 1].isVisited = true;
									mazeVector[iter.first][iter.second - 1].direction = LEFT;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[iter.first][iter.second - 1].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborNeg = true;
								}

								//Conditional loading into currentCoord and recoveryPoint
								//Only one is loaded into currentCoord; if both upper and lower exist,
								//lower is instead loaded into recoveryPoint
								newCoord.clear();
								if (hasNeighborNeg && hasNeighborPos) {
									newCoord.emplace_back(iter.first, iter.second + 1);
									recoveryPoint.emplace_front(iter.first, iter.second - 1);
								} else if (hasNeighborPos) {
									newCoord.emplace_back(iter.first, iter.second + 1);
								} else if (hasNeighborNeg) {
									newCoord.emplace_back(iter.first, iter.second - 1);
								}

							} else if ((DIR & HORIZONTAL) != 0) {
								if (iter.first < mazeNumberColumns - 1 && !mazeVector[iter.first + 1][iter.second].isWall) {
									mazeVector[iter.first + 1][iter.second].isVisited = true;
									mazeVector[iter.first + 1][iter.second].direction = DOWN;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[iter.first + 1][iter.second].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborPos = true;

								}
								if (iter.first > 0 && !mazeVector[iter.first - 1][iter.second].isWall) {
									mazeVector[iter.first - 1][iter.second].isVisited = true;
									mazeVector[iter.first - 1][iter.second].direction = UP;
									isPathIDDeadEnd.emplace_back(false);
									mazeVector[iter.first - 1][iter.second].pathID = static_cast<unsigned short>(isPathIDDeadEnd.size() - 1);
									hasNeighborNeg = true;
								}

								//Conditional loading into currentCoord and recoveryPoint
								//Only one is loaded into currentCoord; if both upper and lower exist,
								//lower is instead loaded into recoveryPoint
								newCoord.clear();
								if (hasNeighborNeg && hasNeighborPos) {
									newCoord.emplace_back(iter.first + 1, iter.second);
									recoveryPoint.emplace_front(iter.first - 1, iter.second);
								} else if (hasNeighborPos) {
									newCoord.emplace_back(iter.first + 1, iter.second);
								} else if (hasNeighborNeg) {
									newCoord.emplace_back(iter.first - 1, iter.second);
								}
							}

							changedPos.emplace_back(iter.first, iter.second);

							//Special routine: if the point we're checking doesn't have neighbors, sets that pathID as a dead end.
							//then clears the vectors and writes the new coordinate to newCoord
							if (!(hasNeighborPos || hasNeighborNeg)) {
								isPathIDDeadEnd[mazeVector[iter.first][iter.second].pathID] = true;
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
						} else if (!(mazeVector[iter.first + firstModifier][iter.second + secondModifier].isVisited)) {
							newCoord.emplace_back(iter.first + firstModifier, iter.second + secondModifier);
							mazeVector[iter.first + firstModifier][iter.second + secondModifier].isVisited = true;
							mazeVector[iter.first + firstModifier][iter.second + secondModifier].pathID 
								= mazeVector[iter.first][iter.second].pathID;
							mazeVector[iter.first + firstModifier][iter.second + secondModifier].direction = static_cast<unsigned char>(DIR);

							changedPos.emplace_back(iter.first + firstModifier, iter.second + secondModifier);

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
					isPathIDDeadEnd[mazeVector[iter.first][iter.second].pathID] = true;
					currentCoord.clear();
					newCoord.clear();
					newCoord.push_back(recoveryPoint.front());
					recoveryPoint.pop_front();
				} else {
					DIR = 0;
				}

				//Checks for finished status: if the coordinate either below or to the right is the size of the maze, that is the exit
				if (iter.first >= mazeNumberRows - 1 || iter.second >= mazeNumberColumns - 1) {
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

void write_debug(const unsigned int maxColumn, const unsigned int maxRow) {
	//Yes, I know the code looks like crap. Don't judge. I wrote this two  years ago (save for a couple changes here and there)
	//Good thing is, it just works fine (except when the maze size is above 1000 in one dimension, in which case it just breaks down lol)

	std::ofstream debugFile;

	debugFile.open("debug.log", std::ios::out | std::ios::trunc | std::ios::beg);

	for (int i = -4; (i < 0?(static_cast<unsigned int>(i) > maxRow): (static_cast<unsigned int>(i) < maxRow)); i++) {
		if (i < -1) {
			debugFile << "row ";
			for (unsigned int j = 0; j < maxColumn; j++) {
				debugFile << (j / static_cast<int>(pow(10,(abs(i) - 2)))) % 10;
			}
		} else if (i == -1) {
			debugFile << "col";
			for (unsigned int j = 0; j < maxColumn; j++) {
				debugFile << '_';
			}
		} else {
			for (int j = -2; (j < 0 ? (static_cast<unsigned int>(j) > maxColumn) : (static_cast<unsigned int>(j) < maxColumn)); j++) {
				if (j == -2) {
					debugFile << std::setw(3) << i;
				} else if (j == -1) {
					debugFile << '|';
				} else {
					if (mazeVector[i][j].isWall) {
						debugFile << 'W';
					} else if (mazeVector[i][j].isVisited) {
						debugFile << mazeVector[i][j].pathID % 10;
					} else {
						debugFile << 'E';
					}
				}
			}
		}
		debugFile << '\n';
	}
	debugFile.close();
}

void write_image() {

	//Create new file
	std::wstring newname = fileDefaultName;
	newname.append(L".result.bmp");
	std::ofstream result(newname, std::ios::out | std::ios::binary);

	for (auto& iter : changedPos) { 

		char* write_head = rawImageBlock + static_cast<unsigned char> (*(rawImageBlock + 10))
						+ (iter.first * (mazeNumberRows * 3 + rawPadding)) + (iter.second * 3);

		if (mazeVector[iter.first][iter.second].isWall) {
			write_head[0] = 0x00;
			write_head[1] = 0x00;
			write_head[2] = 0x00;
		}
		else if (isPathIDDeadEnd[mazeVector[iter.first][iter.second].pathID]) {
			write_head[0] = 0x00;
			write_head[1] = 0x00;
			write_head[2] = static_cast<unsigned char>(0xFF);
		}
		else if (mazeVector[iter.first][iter.second].isVisited) {
			write_head[0] = 0x00;
			write_head[1] = static_cast<unsigned char>(0xFF);
			write_head[2] = 0x00;
		}
		else {
			write_head[0] = static_cast<unsigned char>(0xFF);
			write_head[1] = static_cast<unsigned char>(0xFF);
			write_head[2] = static_cast<unsigned char>(0xFF);
		}
		write_head += 3;
	}

	changedPos.clear();

	result.seekp(0, std::ios::beg);
	result.write(rawImageBlock, fileBinarySize);
}

enum PIXEL_TYPE
{
	PIXEL_SOLID = 0x2588,
	PIXEL_THREEQUARTERS = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591,
};

enum COLOUR
{
	FG_BLACK		= 0x0000,
	FG_DARK_BLUE    = 0x0001,	
	FG_DARK_GREEN   = 0x0002,
	FG_DARK_CYAN    = 0x0003,
	FG_DARK_RED     = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW  = 0x0006,
	FG_GREY			= 0x0007, // Thanks MS :-/
	FG_DARK_GREY    = 0x0008,
	FG_BLUE			= 0x0009,
	FG_GREEN		= 0x000A,
	FG_CYAN			= 0x000B,
	FG_RED			= 0x000C,
	FG_MAGENTA		= 0x000D,
	FG_YELLOW		= 0x000E,
	FG_WHITE		= 0x000F,
	BG_BLACK		= 0x0000,
	BG_DARK_BLUE	= 0x0010,
	BG_DARK_GREEN	= 0x0020,
	BG_DARK_CYAN	= 0x0030,
	BG_DARK_RED		= 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW	= 0x0060,
	BG_GREY			= 0x0070,
	BG_DARK_GREY	= 0x0080,
	BG_BLUE			= 0x0090,
	BG_GREEN		= 0x00A0,
	BG_CYAN			= 0x00B0,
	BG_RED			= 0x00C0,
	BG_MAGENTA		= 0x00D0,
	BG_YELLOW		= 0x00E0,
	BG_WHITE		= 0x00F0,
};

void write_console(const unsigned short maxColumn, const unsigned short maxRow) {
	//Yes, I know the code looks like crap. Don't judge. I wrote this two  years ago (save for a couple changes here and there)
	//Good thing is, it just works fine (except when the maze size is above 1000 in one dimension, in which case it just breaks down lol)

	HANDLE newConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT rectConsole = { 0, 0, static_cast<short>(maxColumn), static_cast<short>(maxRow) };
	SetConsoleWindowInfo(newConsoleHandle, true, &rectConsole);
	COORD coordConsole = { static_cast<short>(maxColumn), static_cast<short>(maxRow) };
	if (!SetConsoleScreenBufferSize(newConsoleHandle, coordConsole)) {
		std::wcerr << L"ERROR IN SetConsoleScreenBufferSize";
	}
	if (!SetConsoleActiveScreenBuffer(newConsoleHandle)) {
		std::wcerr << L"ERROR IN SetConsoleActiveScreenBuffer";
	}
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 3;
	cfi.dwFontSize.Y = 3;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"Consolas");
	if (!SetCurrentConsoleFontEx(newConsoleHandle, false, &cfi)) {
		std::wcerr << L"SetCurrentConsoleFontEx";
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(newConsoleHandle, &csbi))
		std::wcerr << L"GetConsoleScreenBufferInfo";
	if (static_cast<signed int>(maxColumn) > csbi.dwMaximumWindowSize.Y)
		std::wcerr << L"Screen Height / Font Height Too Big";
	if (static_cast<signed int>(maxRow) > csbi.dwMaximumWindowSize.X)
		std::wcerr << "Screen Width / Font Width Too Big";
	rectConsole = { 0, 0, (short)maxRow - 1, (short)maxColumn - 1 };
	if (!SetConsoleWindowInfo(newConsoleHandle, TRUE, &rectConsole))
		std::wcerr << L"SetConsoleWindowInfo";
	auto* bufferConsole = new CHAR_INFO[maxRow * maxColumn];
	memset(bufferConsole, 0, sizeof(CHAR_INFO) * maxRow * maxColumn);

	while (true) {

		for (int x = 0; static_cast<unsigned int>(x) < maxRow; x++) {
			for (int y = 0; static_cast<unsigned int>(y) < maxColumn; y++) {
				if (mazeVector[y][x].isWall) {
					bufferConsole[y * maxRow + x].Char.UnicodeChar = PIXEL_QUARTER;
					bufferConsole[y * maxRow + x].Attributes = FG_WHITE | BG_GREY;
				} else if (mazeVector[y][x].isVisited) {
					bufferConsole[y * maxRow + x].Char.UnicodeChar = PIXEL_SOLID;
					bufferConsole[y * maxRow + x].Attributes = (mazeVector[y][x].pathID % 14) + 1;
				} else {
					bufferConsole[y * maxRow + x].Char.UnicodeChar = PIXEL_QUARTER;
					bufferConsole[y * maxRow + x].Attributes = FG_GREY | BG_BLACK;
				}
			}
		}
		WriteConsoleOutput(newConsoleHandle, bufferConsole, { (short)maxColumn, (short)maxRow }, { 0,0 }, &rectConsole);
		
		switch (int c = _getch()) {
			case 102:
				write_debug();
		}
		_getch();

		isDebugComplete.notify_all();
	}


}