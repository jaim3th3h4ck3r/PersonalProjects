#include "pch.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <time.h>

using namespace std;

//Declare global variables
unsigned int filex, filey;
unsigned int padding = 0;

//name of image file; on same folder as executable
string name = "maze.bmp";

//2D vector, used for positioning
//Global because writing functions use it for writing data
vector < vector<int> > coor(0, vector<int>(0));

// Declare child funcions
void write_debug();
void write_image();


//Pointer to image file in memory
//Only used to load file into memory for conversion into vector array
//Hence it's local
char* memblock;

int main(void) {

	unsigned int filesize = 0;
	char* sizepointer;



	unsigned int a, b;

	//int lastdirx = 1;
	//int lastdiry = 1;

	//cout << "Enter file name: ";
	//cin >> name;

	//Read image file and load it into memblock
	ifstream maze(name, ios::in | ios::binary | ios::ate);
	filesize = maze.tellg();
	memblock = new char[filesize];
	maze.seekg(0, ios::beg);
	maze.read(memblock, filesize);
	maze.close();

	//Read the number of horizontal pixels from header, 18 bytes from start of file
	sizepointer = memblock + 18;
	filex = 256 + (int)*sizepointer;

	//Read the number of vertical pixels from header, 22 bytes from start of file
	sizepointer = memblock + 22;
	filey = 256 + (int)*sizepointer;

	//Parse memblock into int vector array
	sizepointer = memblock + (int) *(memblock + 10);				// Set pointer to start of image data
	filesize = filesize - (int) *(memblock + 10);					// Set new filesize to only size of image data
	padding = (3 * filex) % 4;
	coor.resize(filey);

	//Vector-writing loops
	//Row loop = y-coordinate
	for (a = 0; a < (filey); a++) {

		//Column loop = x-coordinate
		for (b = 0; b < (filex); b++) {

			//Location of data in order
			int offset = (a*(filex)) + (b);
			coor[a].push_back(*(sizepointer + (3 * offset) + (a*padding)));
		}
	}
	write_debug();
	write_image();

	// 2D vector complete
	// Start solving algorithm
	a = 0;
	b = 0;
	srand(time(nullptr));
	//Old algorithm
	//	while (coor[a][b] == 0) {
	//		b++;
	//	}
	//	while (a < filey) {
	//		//vertical movement
	//		while (coor[a][b] != 0 && !(a >= (filey - 1))) {
	//			coor[a][b] = 2;
	//			a = a + lastdiry;
	//		}
	//		if (a >= (filey - 1)) {
	//			break;
	//		}
	//		lastdiry = lastdiry * -1;
	//		a = a + lastdiry;
	//		lastdiry = rand() % 2;
	//		lastdiry = (lastdiry * -2) + 1;
	//
	//		//Horizontal movement
	//		while (coor[a][b] != 0) {
	//			coor[a][b] = 2;
	//			b = b + lastdirx;
	//		}
	//		lastdirx = lastdirx * -1;
	//		b = b + lastdirx;
	//		lastdirx = rand() % 2;
	//		lastdirx = (lastdirx * -2) + 1;
	//	}
	cout << "Finished!";
	write_debug();
	return 0;
}


//write_debug: writes vector array data to diagnostic .txt file
void write_debug() {
	ofstream debug("debug.txt", ios::out);
	for (unsigned int i = 0; i < (filey); i++) {									// Row loop
		for (unsigned int j = 0; j < (filex); j++) {								// Column loop
			debug << abs(coor[i][j]);
		}
		debug << '\n';
	}
	return;
}

//write_image: writes vector array to final pretty .bmp image
void write_image() {
	string newname = name;
	newname.append(".result.bmp");
	ofstream result(newname, ios::out | ios::binary);

	//Set new filesize to only size of image data
	int header_size = *(memblock + 10);
	result.write(memblock, header_size);					//Write header from input file to output file

	//Start image writing
	int write_head = 0x000000;
	result.seekp(0, ios::end);
	char pad_data = 0;

	//Row loop = y-coordinate
	for (unsigned int x = 0; x < (filey); x++) {

		//Column loop = x-coordinate
		for (unsigned int y = 0; y < (filex); y++) {
			switch (coor[x][y]) {
			case 0:
				write_head = 0x000000;
				break;
			case -1:
				write_head = 0xFFFFFF;
				break;
			case 2:
				write_head = 0xFF0000;
				break;
			default:
				write_head = 0x0000FF;
				break;
			}
			result.write((char*)&write_head, 3);
		}
		result.write(&pad_data, padding);
	}
	return;
}

