#include "pch.h"
#include "DibujarDebug.h"
#include "DibujarFunciones.h"


DibujarDebug::DibujarDebug(DibujarFunciones* dibujar_param) : dibujar(dibujar_param) {
	dibujar->setDebugInterface(this);

	std::ofstream* borrar = new std::ofstream;
	borrar->open(DEBUG_FILE, std::ios::out | std::ios::trunc);
	borrar->close();
	delete borrar;
}

void DibujarDebug::writeDebug(int paso, int fila_columna) {
	dbgFile.open(DEBUG_FILE, std::ios::out | std::ios::app);
	if (paso == 0) {
		dbgFile << "------------------------------------------------------------------\n";
		dbgFile << "------------------------------------------------------------------\n\n";
	}
	dbgFile << "Paso: 0 antes de las operaciones, 1 para mover, 2 para sumar" << '\n';
	for (int i = 0; i < (dibujar->max_altura); i++) {
		for (int j = 0; j < (dibujar->max_altura); j++) {
			dbgFile << " " << dibujar->getCasillas(j, i);
		}
		dbgFile << '\n';
	}
	dbgFile << "paso: " << paso << "  |  fila/columna: " << fila_columna << "\n\n\n";

	dbgFile.close();
}

void DibujarDebug::writeError(int error, int caller, int param1, int param2, int param3) {
	dbgFile.open(DEBUG_FILE, std::ios::out | std::ios::app);
	switch (error) {
	case BAD_SETCASILLAS:
		dbgFile << "Bad setCasillas caller above! \nBad address --> column: " << param1 << " | row: " << param2 << "\n";
		dbgFile << "Caller: " << ((caller & CORRER_CASILLAS) == CORRER_CASILLAS ? "correrCasillas, " : "sumarCasillas, ")
			<< ((caller & HOR_CASILLAS) == HOR_CASILLAS ? "horizontal, " : "vertical, ")
			<< ((caller & INC_CASILLAS) == INC_CASILLAS ? "incrementando" : "decreciendo") << "\n\n";
		break;
	}
	dbgFile.close();
}
