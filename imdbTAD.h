

#ifndef TPF_IMDBTAD_H
#define TPF_IMDBTAD_H
#include <stdio.h>

typedef struct imdbCDT * imdbADT;

imdbADT new();
void skipLine(FILE *arch);
void add(FILE *arch, imdbADT imdb);

#endif //TPF_IMDBTAD_H
