#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct tNode{
    char * Title;
    char * Genres;
    float Rating;
    unsigned long Votes;
    struct tNode * tail;
}tNode;

typedef tNode * tList;

typedef struct tNodeYear{
    unsigned int Year;
    unsigned int NumMovies;
    unsigned int NumSeries;
    struct tNodeYear * tail;
    tList * firstSeries;
    tList * firstMovies;
}tNodeYear;

typedef tNodeYear * tListYear;

typedef struct imdbCDT{
    tListYear * first;
}imdbCDT;

imdbADT new(){
    return calloc(1, sizeof(imdbCDT));
}

