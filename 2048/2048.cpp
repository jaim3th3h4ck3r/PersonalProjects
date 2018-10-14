// Si quieres debugear, define DEBUG como 1 en main.cpp y en DibujarFunciones.h
// Si DEBUG es 0, no es necesario incluir DibujarDebug.h y dibujarDebug.cpp en el proyecto
// Suele venir bien limpiar el proyecto antes de compilar cuando se cambia este parametro
#define DEBUG 1

#include "pch.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include "DibujarFunciones.h"

#if DEBUG == 1
#include "DibujarDebug.h"
#endif


int main(int argc, char **argv) {
	int altura;
	std::string entrada;

	if (argc == 2 && atoi(argv[1]) != 0)
		altura = atoi(argv[1]);
	else
		altura = 4;


	while (true) {
		DibujarFunciones *Dibujar1 = new DibujarFunciones(altura);

		//Para debugear
		#if DEBUG == 1
		DibujarDebug *Debug = new DibujarDebug(Dibujar1);
		#endif

		while (true) {
			Dibujar1->nuevaCasilla();
			Dibujar1->dibujarCasillas();
			std::cout << std::endl << std::endl << "Siguiente movimiento: ";
			std::cin >> entrada;
			if (entrada == "nuevo")break;
			while (true) {
				if (entrada == "arriba") {
					Dibujar1->correrMatriz(1);
					break;
				}
				else if (entrada == "abajo") {
					Dibujar1->correrMatriz(2);
					break;
				}
				else if (entrada == "izquierda") {
					Dibujar1->correrMatriz(3);
					break;
				}
				else if (entrada == "derecha") {
					Dibujar1->correrMatriz(4);
					break;
				}
				else {
					std::cout << "Comando invalido" << std::endl << std::endl << "Siguiente movimiento: ";
					std::cin >> entrada;
				}
			}
			system("cls");
		}
		system("cls");

		//Para debugear
		#if DEBUG == 1
		delete Debug;
		#endif

		delete Dibujar1;
	}
}