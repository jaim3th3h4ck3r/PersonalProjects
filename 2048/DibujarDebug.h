#ifndef DIBUJARDEBUG_H
#define DIBUJARDEBUG_H

#define DEBUG_FILE "debug.log"

#include <iostream>
#include <fstream>

class DibujarFunciones;

enum ERR_CODE {
	BAD_SETCASILLAS = 1
};

class DibujarDebug {
public:
	DibujarDebug(DibujarFunciones* dibujar_param);
	void writeDebug(int caller, int fila_columna);
	void writeError(int error, int caller, int param1 = 0, int param2 = 0, int param3 = 0);

private:
	std::ofstream dbgFile;
	DibujarFunciones* dibujar;
};


#endif // DIBUJARDEBUG_H
