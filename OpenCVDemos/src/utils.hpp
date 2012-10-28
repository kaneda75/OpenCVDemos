#ifndef UTILS_H
#define UTILS_H

#define NO_ERROR       0
#define ERROR_LECTURA  1
#define MAX_LONG_TOKEN 30

#define NUM_TOKENS_MAX_LINEA_PARTICIPACION   10
#define NUM_TOKENS_PARTICIPACION              2
#define NUM_TOKENS_LINEA_PROPIEDAD   4
#define NUM_TOKENS_LINEA_TRANSMISION            3
#define NUM_TOKENS_LINEA_DETALLES_TRANSMISION   2
#define NUM_TOKENS_LINEA_PROPIETARIO  3

#define FILE_PROPIEDADES        "propiedades.txt"
#define FILE_INFORME        	"informe.txt"
#define FILE_TRANSMISIONES      "transmisiones.txt"
#define FILE_NUEVAS_PROPIEDADES "propiedades.txt"
#define FILE_PARTICIPACIONES    "participaciones.txt"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>


using namespace std;

int f_leerLinea( FILE *ftxt, char *linea );
int str_comprobarCadena( const char *linea, const char *cadena );
float redondearAlza( float num, int numdec);

/* version string */
int tokenizar_linea( string, string [] );
int tokenizar_linea( string linea, string delimitadors, string tok[]);  
string string_sin_caracter_carro(string linea);

char stringtochar( const char *str );
char stringtochar( string str );

string chartostring( char c );
string inttostring ( int n  ); 

#endif
