//
// Created by juan on 7/5/2021.
//

#include "imdbTAD.h"

typedef struct tNodeSeries{
    char * Title;
    unsigned int EndYear;
    char * Genres;
    float Rating;
    unsigned long Votes;
    struct tNodeSeries * tail;
}tNodeSeries;

typedef tNodeSeries * tListSeries

typedef struct tNodeMovies{
    char * Title;
    char * Genres;
    float Rating;
    unsigned long Votes;
    unsigned int RunTimeMinutes;
    struct tNodeMovies * tail;
}tNodeMovies;

typedef tNodeMovies * tListMovies

typedef struct tNodeYear{
    unsigned int Year;
    unsigned int NumMovies;
    unsigned int NumSeries;
    struct tNodeYear * tail;
    tListSeries * firstSeries;
    tListMovies * firstMovies;
}tNodeYear;

typedef tNodeYear * tListYear

typedef struct imdbCDT{
    tListYear * first;
}imdbCDT;