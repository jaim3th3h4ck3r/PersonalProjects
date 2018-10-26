#include "pch.h"

#include <iostream>
#include <conio.h>
#include <vector>
#include <ctime>
#include <algorithm>
#include <stdexcept>

/* include for debugging, comment out for production */
//#define IS_DEBUGGING

#ifdef IS_DEBUGGING
#include <fstream>
#endif

enum callerTypes {
	CORRER_CASILLAS = 0b0100,
	SUMAR_CASILLAS = 0b0000,

	VER_CASILLAS = 0b0000,
	HOR_CASILLAS = 0b0010,

	INC_CASILLAS = 0b0001,
	DEC_CASILLAS = 0b0000
};

enum ERR_CODE {
	BAD_SETCASILLAS = 1
};



class DibujarDebug;

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

		int score;
	
	
		DibujarDebug* debug;
		friend class DibujarDebug;
	
		std::vector < int > matriz;
	
};

#ifdef IS_DEBUGGING
class DibujarDebug {
	public:
		explicit DibujarDebug(DibujarFunciones* dibujar_param);
		void writeDebug(int caller, int fila_columna);
		void writeError(int error, int caller, int param1 = 0, int param2 = 0, int param3 = 0);
	
	private:
		const std::string debugLogFile = "debug.log";
		std::ofstream dbgFile;
		DibujarFunciones* dibujar;
};
#endif

extern "C" int _setenvp {};

bool isNewGame;

int wmain(int argc, char **argv) {
	int altura;
	
	int charEntrada;

	if (argc == 2 && atoi(argv[1]) != 0) {
		altura = atoi(argv[1]);
	} else {
		altura = 4;
	}

	while (true) {
		auto *Dibujar1 = new DibujarFunciones(altura);

		//Para debugear
		#ifdef IS_DEBUGGING
		DibujarDebug *Debug = new DibujarDebug(Dibujar1);
		#endif

		isNewGame = false;
		while (!isNewGame) {
			Dibujar1->nuevaCasilla();
			Dibujar1->dibujarCasillas();
			wprintf(L"\n\nSiguiente movimiento: (flechas para moverse, N para nuevo juego, Esc para salir)");

			bool entryIsGood = false;
			while (!entryIsGood) {
				charEntrada = _getch();
				switch (charEntrada) {
					case 110:
						entryIsGood = true;
						isNewGame = true;
						break;
					case 27:
						#ifdef IS_DEBUGGING
						delete Debug;
						#endif
						delete Dibujar1;
						return 0;
					case 0xE0:
						charEntrada = _getch();
						switch (charEntrada) {
							case 72:
								Dibujar1->correrMatriz(1);
								entryIsGood = true;
								break;
							case 80:
								Dibujar1->correrMatriz(2);
								entryIsGood = true;
								break;
							case 75:
								Dibujar1->correrMatriz(3);
								entryIsGood = true;
								break;
							case 77:
								Dibujar1->correrMatriz(4);
								entryIsGood = true;
								break;
							default:
								wprintf(L"Comando invalido\n\nSiguiente movimiento: ");
						}
						break;
				}
				
			}
			system("cls");
		}
		system("cls");

		//Para debugear
		#ifdef IS_DEBUGGING
		delete Debug;
		#endif

		delete Dibujar1;
	}
}


//____________________________________________________________________________________
//Definicion de functiones de DibujarFunciones
//____________________________________________________________________________________

//constructor: declara la constante max_altura e inicializa la matriz
DibujarFunciones::DibujarFunciones(int altura) : max_altura(altura), score(0) {
	debug = nullptr;
	matriz.resize(max_altura*max_altura);
}

//devuelve el valor de la matriz, con verificacion de coordenadas
int DibujarFunciones::getCasillas(int x, int y) const {
	if (x >= max_altura || y >= max_altura || x < 0 || y < 0) {
		return 0;
	} else {
		return matriz[y*max_altura + x];
	}
}

//cambia un valor de la matriz, con verificacion de coordenadas
void DibujarFunciones::setCasillas(int x, int y, int valor, int caller) {
	try {
		matriz.at(x + y * max_altura) = valor;
	} catch (std::out_of_range &e) {
		#ifdef IS_DEBUGGING
		debug->writeError(BAD_SETCASILLAS, caller, x, y);
		#endif
		std::wcerr << L"ERROR -- " << e.what() << L"\nBad vector coordinate: (" << x << L", " << y << L")";
		std::system("pause");
		exit;
	}
}

//mete un 1 o un 2 al azar en un espacio vacio de la matriz
void DibujarFunciones::nuevaCasilla() {
	int x, y;
	srand(static_cast<unsigned int>(time(nullptr)));
	do {
		x = rand() % max_altura;
		y = rand() % max_altura;
	} while (getCasillas(x, y) != 0);
	setCasillas(x, y, (rand() % 2) + 1);
}

//ejecuta el mover la matriz llamando a correrFilaVertical o correrFilaHorizontal
void DibujarFunciones::correrMatriz(int direccion) {
	//debugging
	#ifdef IS_DEBUGGING
	debug->writeDebug(0, -1);
	#endif

	//1 -- arriba
	//2 -- abajo
	//3 -- izquierda
	//4 -- derecha
	switch (direccion) {
	case 1: //arriba
		for (int i = 0; i < max_altura; i++) {
			if (!columnaEsCero(i, 0, max_altura - 1)) {
				correrColumnaVertical(i, 1);
				sumarColumnaVertical(i, 1);
				correrColumnaVertical(i, 1);
			}
		}
		break;
	case 2: //abajo
		for (int i = 0; i < max_altura; i++) {
			if (!columnaEsCero(i, 0, max_altura - 1)) {
				correrColumnaVertical(i, -1);
				sumarColumnaVertical(i, -1);
				correrColumnaVertical(i, -1);
			}
		}
		break;
	case 3:
		for (int i = 0; i < max_altura; i++) {
			if (!filaEsCero(i, 0, max_altura - 1)) {
				correrFilaHorizontal(i, 1);
				sumarFilaHorizontal(i, 1);
				correrFilaHorizontal(i, 1);
			}
		}
		break;
	case 4:
		for (int i = 0; i < max_altura; i++) {
			if (!filaEsCero(i, 0, max_altura - 1)) {
				correrFilaHorizontal(i, -1);
				sumarFilaHorizontal(i, -1);
				correrFilaHorizontal(i, -1);
			}
		}
		break;
	default:
		std::wcerr << L"correrMatriz: parametro no valido";
		break;
	}
	if (!std::any_of(matriz.begin(), matriz.end(), [] (int x) { return x == 0; })) {
		isNewGame = true;
		wprintf(L"\n\nGAME OVER!\nPress N to start new game or");
		while (_getch() != 110) {};
	}
}



//corre una sola columna
//direccion: 1 arriba, -1 abajo 
void DibujarFunciones::correrColumnaVertical(int columna, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= 0 && i < max_altura; i += direccion) {
		while (getCasillas(columna, i) == 0) {
			if (columnaEsCero(columna, i, (start == 0 ? max_altura - 1 : 0))) {
				#ifdef IS_DEBUGGING
				debug->writeDebug(1, columna);
				#endif
				return;
			}
			for (int j = i; j >= 0 && j < max_altura; j += direccion) {
				setCasillas(columna, j, getCasillas(columna, j + direccion), CORRER_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);
			}
		}
	}
	#ifdef IS_DEBUGGING
	debug->writeDebug(1, columna);
	#endif
}

//suma una columna vertical
void DibujarFunciones::sumarColumnaVertical(int columna, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= (direccion == 1 ? 0 : 1) && i < (direccion == 1 ? max_altura - 1 : max_altura); i += direccion) {
		if (getCasillas(columna, i) == getCasillas(columna, i + direccion)) {
			setCasillas(columna, i, getCasillas(columna, i) * 2, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);
			score += getCasillas(columna, i);
			i += direccion;
			setCasillas(columna, i, 0, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);	
		}
	}

	#ifdef IS_DEBUGGING
	debug->writeDebug(2, columna);
	#endif
}


//corre una sola fila
//direccion: 1 izquierda, -1 derecha 
void DibujarFunciones::correrFilaHorizontal(int fila, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= 0 && i < max_altura; i += direccion) {
		while (getCasillas(i, fila) == 0) {
			if (filaEsCero(fila, i, (start == 0 ? max_altura - 1 : 0))) {
				#ifdef IS_DEBUGGING
				debug->writeDebug(1, fila);
				#endif
				return;
			}
			for (int j = i; j >= 0 && j < max_altura; j += direccion) {
				setCasillas(j, fila, getCasillas(j + direccion, fila), CORRER_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
			}
		}
	}
	#ifdef IS_DEBUGGING
	debug->writeDebug(1, fila);
	#endif
}

void DibujarFunciones::sumarFilaHorizontal(int fila, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= (direccion == 1 ? 0 : 1) && i < (direccion == 1 ? max_altura - 1 : max_altura); i += direccion) {
		if (getCasillas(i, fila) == getCasillas(i + direccion, fila)) {
			setCasillas(i, fila, getCasillas(i, fila) * 2, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
			score += getCasillas(i, fila);
			i += direccion;
			setCasillas(i, fila, 0, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
		}
	}
	#ifdef IS_DEBUGGING
	debug->writeDebug(2, fila);
	#endif
}



bool DibujarFunciones::columnaEsCero(int columna, int pos1, int pos2) const {
	for (int i = (pos1 < pos2 ? pos1 : pos2); i <= (pos1 > pos2 ? pos1 : pos2); i++) {
		if (getCasillas(columna, i) != 0) {
			return false;
		}
	}
	return true;
}

bool DibujarFunciones::filaEsCero(int fila, int pos1, int pos2) const {
	for (int i = (pos1 < pos2 ? pos1 : pos2); i <= (pos1 > pos2 ? pos1 : pos2); i++) {
		if (getCasillas(i, fila) != 0) {
			return false;
		}
	}
	return true;
}


void DibujarFunciones::dibujarCasillas() {
	wprintf(L"Score: %d\n", score);
	//Dibuja la linea de arriba de la matriz 
	for (int j = 0; j < max_altura; j++) {
		wprintf(L"_____________");
	}

	//Dibuja el resto de la matriz
	for (int i = 0; i < max_altura; i++) {
		for (int k = 0; k <= 1; k++) {
			wprintf(L"\n|");
			for (int j = 0; j < max_altura; j++) {
				wprintf(L"            |");
			}
		}

		wprintf(L"\n|");
		for (int j = 0; j < max_altura; j++) {
			if (getCasillas(j, i) != 0) {
				wprintf(L"    %4d    |", getCasillas(j, i));
			}
			else {
				wprintf(L"            |");
			}
		}

		for (int i = 0; i <= 0; i++) {
			wprintf(L"\n|");
			for (int j = 0; j < max_altura; j++) {
				wprintf(L"            |");
			}
		}

		wprintf(L"\n|");
		for (int j = 0; j < max_altura; j++) {
			wprintf(L"____________|");
		}
	}
}

void DibujarFunciones::setDebugInterface(DibujarDebug* debug_param) {
	debug = debug_param;
}

#ifdef IS_DEBUGGING
//____________________________________________________________________________________
//Definicion de functiones de DibujarDebug
//____________________________________________________________________________________

DibujarDebug::DibujarDebug(DibujarFunciones* dibujar_param) : dibujar(dibujar_param) {
	dibujar->setDebugInterface(this);

	std::ofstream* borrar = new std::ofstream;
	borrar->open(debugLogFile, std::ios::out | std::ios::trunc);
	borrar->close();
	delete borrar;
}

void DibujarDebug::writeDebug(int paso, int fila_columna) {
	dbgFile.open(debugLogFile, std::ios::out | std::ios::app);
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
	dbgFile.open(debugLogFile, std::ios::out | std::ios::app);
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
#endif