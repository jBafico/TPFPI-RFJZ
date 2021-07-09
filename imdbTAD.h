#ifndef TPF_IMDBTAD_H
#define TPF_IMDBTAD_H
#include <stdio.h>
#define ERROR_CODE -1
#define OK 1
#define NOTOK !OK

typedef struct imdbCDT * imdbADT;

/* Crea un nuevo imdb vacio. */
imdbADT new();
/* Recibe un archivo y saltea una linea, desde la posicion donde se encuentre el puntero */
void skipLine(FILE *arch);

/* Crea un archivo llamado "query1.csv" con la cantidad de peliculas y series por año */
void query1(FILE *arch, imdbADT imdb);

/* Crea un archivo llamado "query2.csv" con la cantidad de peliculas por año y por genero */
void query2(FILE *arch, imdbADT imdb);

/* Crea un archivo llamado "query3.csv" con la pelicula y la serie con mas votos de cada año */
void query3(FILE *arch, imdbADT imdb);

/* Crea un archivo llamado "query4.csv" con las 100 (cien) peliculas con mejor calificacion */
void query4(FILE *out, imdbADT imdb);

/*
 * Carga los datos a la estructura
 * Devuelve 1 si los datos se agregaron correctamente.
 * Devuelve -1 si no hubo memoria para cargar todos los datos
 */
int add(FILE *arch, imdbADT imdb);

/* Libera los recursos reservados por el TAD */
void freeimdb(imdbADT imdb);

#endif //TPF_IMDBTAD_H
