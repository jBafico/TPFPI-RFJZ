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

static void noMemoryAbort(void)
{
    fprintf(stderr,"No se ha podido reservar memoria");
    exit(1);
}

static unsigned char *getLineNoLimitFile(FILE *arch)
{
    int i = 0;
    unsigned char c;
    unsigned char *s = NULL; // para que el primer realloc funcione como malloc
    while ((c = fgetc(arch)) != '\n')
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

imdbADT new(){
    imdbADT newIMDB=calloc(1, sizeof(imdbCDT));
    if(newIMDB == NULL)
        noMemoryAbort();
    return newIMDB;
}

void skipLine(FILE *arch)
{
    while (fgetc(arch) != '\n')
        ;
}

static char *
copy(const char * copyFrom){
    unsigned int i=0, j=0;
    char * copyTo;
    for(; copyFrom[j]!='\0'; i++, j++){
        if(i%BLOCK == 0)
            copyTo=realloc(copyTo, sizeof(char) * (BLOCK + i));
        if(copyTo == NULL)
            noMemoryAbort();
        copyTo[i]=copyFrom[j];
    }
    copyTo=realloc(copyTo, sizeof(char) * (i+1));
    if(copyTo == NULL)
        noMemoryAbort();
    copyTo[i]='\0';
    return copyTo;
}

static tList addRecType(tList first, tList node)
{
    if (first == NULL || node->votes >= first->votes)
    {
        node->tail = first;
        return node;
    }
    first->tail = addRecType(first->tail, node);
    return first;
}

static tListYear addRec(tListYear first, int year, char *type, tList node)
{
    if (first == NULL || year > first->year)
    {
        int class;
        if(strcmp(type, "movie") == 0)
            class = 1;
        else if(strcmp(type, "tvSeries") == 0)
            class = 0;
        else
            return NULL;
        tListYear newYear = calloc(1, sizeof(tNodeYear));
        if (newYear == NULL)
            noMemoryAbort();
        newYear->year = year;
        newYear->tail = first;
        if (class==1){
            newYear->firstMovies = node;
            newYear->numMovies++;
        }
        else{
            newYear->firstSeries = node;
            newYear->numSeries++;
        }
        return newYear;
    }
    if (year == first->year)
    {
        // cambios
        if (strcmp(type, "movie") == 0){
            // se agrega una pelicula
            first->firstMovies = addRecType(first->firstMovies, node);
            first->numMovies++;
        }else if ( strcmp(type,"tvSeries") == 0){
            // se agrega una serie
            first->firstSeries = addRecType(first->firstSeries, node);
            first->numSeries++;
        }
        // si no era ni pelicula ni serie, simplemente devuelvo lo que estaba pues no hay nada a insertar
        return first;
    }
    first->tail = addRec(first->tail, year, type, node);
    return first;
}


imdbADT add(FILE *arch, imdbADT imdb)
{
    while (!feof(arch))
    {
        int year;
        char *type;
        tList newNode = malloc(sizeof(tNode));
        if (newNode == NULL)
            noMemoryAbort();
        unsigned char *string = getLineNoLimitFile(arch);
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


static void toBegin(imdbADT imdb)
{
    imdb->current = imdb->first;
}

static int hasNext(imdbADT imdb)
{
    return imdb->current != NULL;
}

static tListYear next(imdbADT imdb)
{
    tListYear toreturn = imdb->current;
    imdb->current = imdb->current->tail;
    return toreturn;
}

void query1(FILE *arch, imdbADT imdb)
{
    fprintf(arch, "year;films;series\n");
    tobegin(imdb);
    tListYear aux;

    while (hasNext(imdb))
    {
        aux = next(imdb);
        fprintf(arch, "%u;%u;%u\n", aux->year, aux->numMovies, aux->numSeries);
    }
    fclose(arch);
}
