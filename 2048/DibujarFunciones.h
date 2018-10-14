#ifndef DIBUJAR_H
#define DIBUJAR_H

#define DEBUG 1

#include <vector>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

class DibujarDebug;

enum callerTypes {
	CORRER_CASILLAS = 0b0100,
	SUMAR_CASILLAS = 0b0000,

	VER_CASILLAS = 0b0000,
	HOR_CASILLAS = 0b0010,

	INC_CASILLAS = 0b0001,
	DEC_CASILLAS = 0b0000
};

class DibujarFunciones {
public:
	explicit DibujarFunciones(int altura);

	void dibujarCasillas();
	void nuevaCasilla();
	int getCasillas(int x, int y) const;
	void correrMatriz(int direccion);

	const int max_altura;

private:
	void setCasillas(int x, int y, int valor, int caller = 0);

	void correrColumnaVertical(int columna, int direccion);
	void sumarColumnaVertical(int columna, int direccion);
	void correrFilaHorizontal(int fila, int direccion);
	void sumarFilaHorizontal(int fila, int direccion);

	bool columnaEsCero(int columna, int pos1, int pos2) const;
	bool filaEsCero(int fila, int pos1, int pos2) const;

	void setDebugInterface(DibujarDebug* debug_param);


	DibujarDebug* debug;
	friend class DibujarDebug;

	std::vector < int > matriz;

};

#endif // DIBUJAR_H
