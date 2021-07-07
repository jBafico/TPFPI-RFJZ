

#ifndef TPF_IMDBTAD_H
#define TPF_IMDBTAD_H
#include <stdio.h>

typedef struct imdbCDT * imdbADT;

imdbADT new();
void skipLine(FILE *arch);
void query1(FILE *arch, imdbADT imdb);
void query2(FILE *arch, imdbADT imdb);
void query3(FILE *arch, imdbADT imdb);
void query4(FILE *out, imdbADT imdb);
void add(FILE *arch, imdbADT imdb);

#endif //TPF_IMDBTAD_H
