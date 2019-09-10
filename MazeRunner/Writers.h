#pragma once

/* 
File with all external writing functions separate
Keep the main MazeRunner.cpp file neater
*/

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

		char* write_head = rawImageBlock + static_cast<unsigned char> (*(rawImageBlock + 10)) + (iter.first * (mazeNumberRows * 3 + rawPadding)) + (iter.second * 3);

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
	FG_GREY			= 0x0007,
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