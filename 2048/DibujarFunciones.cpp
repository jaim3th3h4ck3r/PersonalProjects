#include "pch.h"
#include "DibujarFunciones.h"

#if DEBUG == 1
#include "DibujarDebug.h"
#endif

//constructor: declara la constante max_altura e inicializa la matriz
DibujarFunciones::DibujarFunciones(int altura) : max_altura(altura) {
	debug = nullptr;
	matriz.resize(max_altura*max_altura);
}

//devuelve el valor de la matriz, con verificacion de coordenadas
int DibujarFunciones::getCasillas(int x, int y) const {
	if (x >= max_altura || y >= max_altura || x < 0 || y < 0)
		return 0;
	else
		return matriz[y*max_altura + x];
}

//cambia un valor de la matriz, con verificacion de coordenadas
void DibujarFunciones::setCasillas(int x, int y, int valor, int caller) {
	if (x >= max_altura || y >= max_altura) {
#if DEBUG == 1
		debug->writeError(BAD_SETCASILLAS, caller, x, y);
#endif
	}
	else
		matriz[x + y * max_altura] = valor;
	return;
}

//mete un 1 o un 2 al azar en un espacio vacio de la matriz
void DibujarFunciones::nuevaCasilla() {
	int x, y;
	srand(time(NULL));
	do {
		x = rand() % max_altura;
		y = rand() % max_altura;
	} while (getCasillas(x, y) != 0);
	setCasillas(x, y, (rand() % 2) + 1);
}

//ejecuta el mover la matriz llamando a correrFilaVertical o correrFilaHorizontal
void DibujarFunciones::correrMatriz(int direccion) {
	//debugging
#if DEBUG == 1
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
		std::cerr << "correrMatriz: parametro no valido";
	}
	return;
}



//corre una sola columna
//direccion: 1 arriba, -1 abajo 
void DibujarFunciones::correrColumnaVertical(int columna, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= 0 && i < max_altura; i += direccion) {
		while (getCasillas(columna, i) == 0) {
			if (columnaEsCero(columna, i, (start == 0 ? max_altura - 1 : 0))) {
#if DEBUG == 1
				debug->writeDebug(1, columna);
#endif
				return;
			}
			for (int j = i; j >= 0 && j < max_altura; j += direccion) {
				setCasillas(columna, j, getCasillas(columna, j + direccion), CORRER_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);
			}
		}
	}
#if DEBUG == 1
	debug->writeDebug(1, columna);
#endif
	return;
}

//suma una columna vertical
void DibujarFunciones::sumarColumnaVertical(int columna, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= (direccion == 1 ? 0 : 1) && i < (direccion == 1 ? max_altura - 1 : max_altura); i += direccion) {
		if (getCasillas(columna, i) == getCasillas(columna, i + direccion)) {
			setCasillas(columna, i, getCasillas(columna, i) * 2, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);
			i += direccion;
			setCasillas(columna, i, 0, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | VER_CASILLAS);
		}
	}

#if DEBUG == 1
	debug->writeDebug(2, columna);
#endif

	return;
}


//corre una sola fila
//direccion: 1 izquierda, -1 derecha 
void DibujarFunciones::correrFilaHorizontal(int fila, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= 0 && i < max_altura; i += direccion) {
		while (getCasillas(i, fila) == 0) {
			if (filaEsCero(fila, i, (start == 0 ? max_altura - 1 : 0))) {
#if DEBUG == 1
				debug->writeDebug(1, fila);
#endif
				return;
			}
			for (int j = i; j >= 0 && j < max_altura; j += direccion) {
				setCasillas(j, fila, getCasillas(j + direccion, fila), CORRER_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
			}
		}
	}
#if DEBUG == 1
	debug->writeDebug(1, fila);
#endif
	return;

	//    int start;
//    
//    if(direccion == 1){start = 0;}
//    else{start = max_altura - 1;}
//    
//    for(int i=start; i>=0 && i<max_altura; i+=direccion){
//        while(getCasillas(i, fila) == 0){
////            if(filaEsCero(fila, i, (max_altura-1)-start) == true){
////                // Justo despues del primer 'for', sin ejecutar la instruccion despues del segundo 'for'
////                goto breakpoint;
////            }
//            for(int j = i + direccion; (i+j) >= 0 && (i+j) < max_altura; j+=direccion){
//                setCasillas(i+j, fila, getCasillas(i + j + direccion, fila));
//            }
//        }
//    }
//    
////    breakpoint:
//    #if DEBUG == 1
//    debug->writeDebug(1, fila);
//    #endif
//    
//    return;
}

void DibujarFunciones::sumarFilaHorizontal(int fila, int direccion) {

	const int start = (direccion == 1 ? 0 : max_altura - 1);

	for (int i = start; i >= (direccion == 1 ? 0 : 1) && i < (direccion == 1 ? max_altura - 1 : max_altura); i += direccion) {
		if (getCasillas(i, fila) == getCasillas(i + direccion, fila)) {
			setCasillas(i, fila, getCasillas(i, fila) * 2, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
			i += direccion;
			setCasillas(i, fila, 0, SUMAR_CASILLAS | (direccion == 1 ? INC_CASILLAS : DEC_CASILLAS) | HOR_CASILLAS);
		}
	}

#if DEBUG == 1
	debug->writeDebug(2, fila);
#endif

	return;
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
	//Dibuja la linea de arriba de la matriz 
	for (int j = 0; j < max_altura; j++) {
		std::cout << "_____________";
	}

	//Dibuja el resto de la matriz
	for (int i = 0; i < max_altura; i++) {
		for (int k = 0; k <= 1; k++) {
			std::cout << std::endl << "|";
			for (int j = 0; j < max_altura; j++) {
				std::cout << "            |";
			}
		}

		std::cout << std::endl << "|";
		for (int j = 0; j < max_altura; j++) {
			if (getCasillas(j, i) != 0) {
				std::cout << "    " << std::setw(4) << getCasillas(j, i) << "    |";
			}
			else {
				std::cout << "            |";
			}
		}

		for (int i = 0; i <= 0; i++) {
			std::cout << std::endl << "|";
			for (int j = 0; j < max_altura; j++) {
				std::cout << "            |";
			}
		}

		std::cout << std::endl << "|";
		for (int j = 0; j < max_altura; j++) {
			std::cout << "____________|";
		}
	}
}

void DibujarFunciones::setDebugInterface(DibujarDebug* debug_param) {
	debug = debug_param;
}