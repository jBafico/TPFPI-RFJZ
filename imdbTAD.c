#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCK 10

typedef struct tNode *tList;
typedef struct tNodeYear *tListYear;

typedef struct tNode
{
    char *title;
    char *genres;
    float rating;
    unsigned long votes;
    tList tail;
} tNode;

typedef struct tNodeYear
{
    unsigned int year;
    unsigned int numMovies;
    unsigned int numSeries;
    tListYear tail;
    tList firstSeries;
    tList firstMovies;
} tNodeYear;

typedef struct imdbCDT
{
    tListYear first;
    tListYear current;
} imdbCDT;

imdbADT add(FILE *arch, imdbADT imdb)
{
    while (!feof(arch))
    {
        int year;
        char *type;
        tList newNode = malloc(sizeof(tNode));
        if (newNode == NULL)
            noMemoryAbort();
        char *string = getLineNoLimitFile(arch);
        char *token = strtok(string, ";");
        type = token;
        int flag = 1;
        for (int i = 0; flag; i++)
        {
            switch (i)
            {
            case 1:
                newNode->title = copy(token); //poner funcion copia
                break;
            case 2:
                year = atoi(token);
                break;
            case 4:
                newNode->genres = copy(token); // poner funcion copy
                break;
            case 5:
                newNode->rating = atof(token);
                break;
            case 6:
                newNode->votes = atoi(token);
                flag = 0;
                break;
            default:
                break;
            }
            token = strtok(NULL, ";");
        }
        imdb->first = addRec(imdb->first, year, type, newNode);
    }
}

char *getLineNoLimitFile(FILE *arch)
{
    int i = 0;
    unsigned char c;
    char *s = NULL; // para que el primer realloc funcione como malloc
    while ((c = fgetc(arch)) != '\n' && !feof(arch))
    {
        if (i % BLOCK == 0)
            s = realloc(s, i + BLOCK); // multiplicar por sizeof(char) no es necesario
        if (s == NULL)
            noMemoryAbort();
        s[i++] = c;
    }
    s = realloc(s, i + 1); // Liberamos lo que sobra del ultimo bloque
    s[i] = 0;
    return s;
}

