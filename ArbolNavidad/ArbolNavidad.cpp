#include "pch.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <TCHAR.h>


int wmain(int argc, wchar_t **argv) {
	int altura, bolas;
	int ancho_tronco;
	int comienzo_tronco;
	int alto_tronco;

	bool bolas_modo = true;

	if (argc == 1) {
		altura = 7;

		//Ancho del tronco: por defecto calcula un quinto de la altura, y despues le resta 1 si es par
		ancho_tronco = (altura / 5) + 1 - (1 - ((altura / 5) % 2));

		//Altura del tronco: por defecto un tercio de la altura del arbol
		alto_tronco = (altura / 3);

		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));

	}
	else if (argv[1][0] == '?' || _wcsicmp(argv[1], L"h") == 0 || _wcsicmp(argv[1], L"help") == 0) {
		wprintf(L"ARBOL DE NAVIDAD -- AYUDA");
		wprintf(L"\n\nEste programa imprime un arbol de navidad en el terminal.\nUso:");
		wprintf(L"\n%s [altura] (--altura-tronco [numero] --anchura-tronco [numero] --no-bolas)", argv[0]);
		return 0;
	}
	else if (_wtoi(argv[1]) == 0) {
		wprintf(L"Argumento '%s' no definido.", argv[1]);
		return -1;
	}
	else {
		//La altura es el primer argumento
		altura = _wtoi(argv[1]);

		//Ancho del tronco: por defecto calcula un quinto de la altura, y despues le resta 1 si es par
		ancho_tronco = (altura / 5) - (1 - ((altura / 5) % 2));

		//Altura del tronco: por defecto un tercio de la altura del arbol
		alto_tronco = (altura / 3);

		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));

		//Parsear los argumentos (si los hay)
		//Los 'continue' aceleran el programa: si dan con un argumento correcto, saltan al siguiente bucle
		for (int argument_parsing = 2; argument_parsing < argc; argument_parsing++) {
			if (_wcsicmp(argv[argument_parsing], L"--altura-tronco") == 0) {
				alto_tronco = _wtoi(argv[++argument_parsing]);
				continue;
			}
			else if (_wcsicmp(argv[argument_parsing], L"--ancho-tronco") == 0) {
				ancho_tronco = _wtoi(argv[++argument_parsing]);

				if (ancho_tronco % 2 == 0) {
					comienzo_tronco = (altura - (ancho_tronco / 2)) * 2;
				}
				else {
					comienzo_tronco = (altura - (ancho_tronco / 2) - 1) * 2;
				}
				continue;
			}
			else if (_wcsicmp(argv[argument_parsing], L"--no-bolas") == 0) {
				bolas_modo = false;
				continue;
			}
			else {
				wprintf(L"Argumento n.%d '%s' no definido.", argument_parsing, argv[argument_parsing]);
				return -argument_parsing;
			}
		}
		//Calcular el comienzo del tronco
		comienzo_tronco = (altura - (ancho_tronco / 2));
	}

	system("cls");

	//Dibuja una estrella arriba del arbol
	for (int star_space1 = 1; star_space1 <= (altura - 1); star_space1++) {
		wprintf(L"  ");
	}
	wprintf(L" ^\n");
	for (int star_space2 = 1; star_space2 <= (altura - 2); star_space2++) {
		wprintf(L"  ");
	}
	wprintf(L" < X >\n");
	for (int star_space3 = 1; star_space3 <= (altura - 1); star_space3++) {
		wprintf(L"  ");
	}
	wprintf(L" v\n");
	for (int star_space4 = 1; star_space4 <= (altura - 1); star_space4++) {
		wprintf(L"  ");
	}
	wprintf(L" ^\n");

	//Empieza el generador srand si se permiten dibujar bolas
	if (bolas_modo) {
		srand(static_cast<unsigned int>(time(nullptr)));
	}

	for (int arbol_filas = 2; arbol_filas <= altura; arbol_filas++) {
		for (int arbol_espacios = 1; arbol_espacios <= (altura - arbol_filas); arbol_espacios++) {
			wprintf(L"  ");
		}
		for (int arbol_relleno = 1; arbol_relleno <= (2 * arbol_filas - 1); arbol_relleno++) {
			if (!bolas_modo) {
				wprintf(L" *");
			} else {
				bolas = rand();
				switch (bolas % 15) {
				case 0:
					wprintf(L" O");
					break;
				case 1:
					wprintf(L" @");
					break;
				case 2:
					wprintf(L" $");
					break;
				default:
					wprintf(L" *");
					break;
				}
			}
		}
		wprintf(L"\n");
	}


	for (int tronco_filas = 1; tronco_filas <= alto_tronco; tronco_filas++) {
		if (ancho_tronco % 2 == 0) {
			wprintf(L" ");
		}
		for (int tronco_espacios = 1; tronco_espacios < comienzo_tronco; tronco_espacios++) {
			wprintf(L"  ");
		}
		for (int tronco_relleno = 1; tronco_relleno <= ancho_tronco; tronco_relleno++) {
			wprintf(L" |");
		}
		wprintf(L"\n");
	}

	system("pause");
	return 0;
}
