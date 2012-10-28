#include <cstdio>
#include <iostream>
#include <math.h>
#include <string>

#include "utils.hpp"

using namespace std;

int str_comprobarCadena(const char *linea, const char *cadena) {
	return (strcmp(linea, cadena));
} /* str_comprobarCabecera */

char stringtochar(const char *str) {
	char c = ' ';
	if (str != NULL) {
		c = str[0];
	}
	return (c); // Devuelve un espacio si la cadena es nula
} /* stringtochar */

char stringtochar(string str) {
	char c = ' ';
	if (str.size() > 0) {
		c = str[0];
	}
	return (c); // Devuelve un espacio si la cadena es nula
} /* stringtochar */

string chartostring(char c) {
	string str = "";
	str += c;
	return (str);
}
;

string inttostring(int n) {

	char* result = (char *) malloc(17);
	string str;

	sprintf(result, "%d", n);
	str = result;
	return (str);
}

float redondearAlza(float num, int numdec) {
	float factor = pow(10.0, numdec);
	return (ceil(num * factor) / (float) factor);
}

// Versión utilizando strings
int tokenizar_linea(string linea, string tok[]) {
	string delimitadors = ";";
	return (tokenizar_linea(linea, delimitadors, tok));
}

int tokenizar_linea(string linea, string delimitadors, string tok[]) {
	int leidos = 0;
	string totsDelimitadors;
	char * tmp;
	char * linea_c = strdup(linea.c_str());

	totsDelimitadors = delimitadors + "\t\n";

	if (linea.size() > 0) {
		tmp = strtok(linea_c, totsDelimitadors.c_str());
		if (tmp != NULL) {
			tok[leidos] = tmp;
			leidos = leidos + 1;

			while (((tmp = strtok(NULL, totsDelimitadors.c_str())) != NULL)
					&& (leidos < 10)) {
				tok[leidos] = tmp;
				leidos = leidos + 1;
			}
		};
	};
	free(linea_c);
	return (leidos);
}

// Aquesta funci— neteja de carˆcters de final de linea els strings
string string_sin_caracter_carro(string linea) {
	char*tmp;
	char * cadena = strdup(linea.c_str());
	while((tmp=strchr(cadena,13))!=NULL)   // 13 Žs el codi ASCII del retorno de carro CR
	  sprintf(tmp,"%s",tmp+1);
	string lineaRetorn = cadena;
	free(cadena);
	return (lineaRetorn);
}
