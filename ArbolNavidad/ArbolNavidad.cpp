#include "pch.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <time.h>

int main(int argc, char *argv[]) {
	int altura, bolas;
	int ancho_tronco;
	int comienzo_tronco;
	int alto_tronco;

	int bolas_modo = 1;

	if (argc == 1) {
		printf("Introduzca altura del arbol (min: 1+; rec: 5+): ");
		std::cin >> altura;

		//Ancho del tronco: por defecto calcula un quinto de la altura, y despues le resta 1 si es par
		ancho_tronco = (altura / 5) - (1 - ((altura / 5) % 2));

		//Altura del tronco: por defecto un tercio de la altura del arbol
		alto_tronco = (altura / 3);

		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));

	}
	else if (argv[1][0] == '?' || _stricmp(argv[1], "h") == 0 || _stricmp(argv[1], "help") == 0) {
		printf("ARBOL DE NAVIDAD -- AYUDA");
		printf("\n\nEste programa imprime un arbol de navidad en el terminal.\nUso:");
		printf("\n%s [altura] (--altura-tronco [numero] --anchura-tronco [numero] --no-bolas)", argv[0]);
		return 0;
	}
	else if (atoi(argv[1]) == 0) {
		printf("Argumento '%s' no definido.", argv[1]);
		return -1;
	}
	else {
		//La altura es el primer argumento
		altura = atoi(argv[1]);

		//Ancho del tronco: por defecto calcula un quinto de la altura, y despues le resta 1 si es par
		ancho_tronco = (altura / 5) - (1 - ((altura / 5) % 2));

		//Altura del tronco: por defecto un tercio de la altura del arbol
		alto_tronco = (altura / 3);

		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));

		//Parsear los argumentos (si los hay)
		//Los 'continue' aceleran el programa: si dan con un argumento correcto, saltan al siguiente bucle
		for (int argument_parsing = 2; argument_parsing < argc; argument_parsing++) {
			if (_stricmp(argv[argument_parsing], "--altura-tronco") == 0) {
				alto_tronco = atoi(argv[++argument_parsing]);
				continue;
			}
			else if (_stricmp(argv[argument_parsing], "--ancho-tronco") == 0) {
				ancho_tronco = atoi(argv[++argument_parsing]);

				if (ancho_tronco % 2 == 0) {
					comienzo_tronco = 2 * (altura - (ancho_tronco / 2));
				}
				else {
					comienzo_tronco = 2 * (altura - (ancho_tronco / 2) - 1);
				}
				continue;
			}
			else if (_stricmp(argv[argument_parsing], "--no-bolas") == 0) {
				bolas_modo = 0;
				continue;
			}
			else {
				printf("Argumento n.%d '%s' no definido.", argument_parsing, argv[argument_parsing]);
				return -argument_parsing;
			}
		}
		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));
	}

	system("cls");

	//Dibuja una estrella arriba del arbol
	for (int star_space1 = 1; star_space1 <= (altura - 1); star_space1++) {
		printf("  ");
	}
	printf(" ^\n");
	for (int star_space2 = 1; star_space2 <= (altura - 2); star_space2++) {
		printf("  ");
	}
	printf(" < X >\n");
	for (int star_space3 = 1; star_space3 <= (altura - 1); star_space3++) {
		printf("  ");
	}
	printf(" v\n");
	for (int star_space4 = 1; star_space4 <= (altura - 1); star_space4++) {
		printf("  ");
	}
	printf(" ^\n");

	//Empieza el generador srand si se permiten dibujar bolas
	if (bolas_modo != 0) { srand(time(0)); }

	for (int arbol_filas = 2; arbol_filas <= altura; arbol_filas++) {
		for (int arbol_espacios = 1; arbol_espacios <= (altura - arbol_filas); arbol_espacios++) {
			printf("  ");
		}
		for (int arbol_relleno = 1; arbol_relleno <= (2 * arbol_filas - 1); arbol_relleno++) {
			if (bolas_modo == 0) { printf(" *"); }
			else {
				bolas = rand();
				switch (bolas % 15) {
				case 0:
					printf(" O");
					break;
				case 1:
					printf(" @");
					break;
				case 2:
					printf(" $");
					break;
				default:
					printf(" *");
					break;
				}
			}
		}
		printf("\n");
	}


	for (int tronco_filas = 1; tronco_filas <= alto_tronco; tronco_filas++) {
		for (int tronco_espacios = 1; tronco_espacios < comienzo_tronco; tronco_espacios++) {
			printf("  ");
		}
		for (int tronco_relleno = 1; tronco_relleno <= ancho_tronco; tronco_relleno++) {
			printf(" |");
		}
		printf("\n");
	}
	system("pause");
	return 0;
}
