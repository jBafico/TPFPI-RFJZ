#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCK 10

typedef struct tNode *tList;
typedef struct tNodeYear *tListYear;

typedef struct tNodeGenre{
    char * genre;
    unsigned int cantMovies;
    struct tNodeGenre * tail;
}tNodeGenre;

typedef tNodeGenre * tListGenre;

typedef struct tNode
{
    char *title;
    char *genres;
    float rating;
    size_t votes;
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
    tListGenre firstGenre;
} tNodeYear;

typedef struct imdbCDT
{
    tListYear first;
    tListYear current;
}imdbCDT;

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

static tListGenre addRecGenre(tListGenre first, char * genre){
    int c;
    if(first ==NULL || (c=strcmp(genre, first->genre))<0){
        tListGenre newGenre=malloc(sizeof(tNodeGenre));
        newGenre->genre=copy(genre);
        newGenre->tail=first;
        newGenre->cantMovies=1;
        return newGenre;
    }
    if(c==0){
        first->cantMovies+=1;
        return first;
    }
    first->tail=addRecGenre(first->tail, genre);
    return first;
}

static tListYear addRec(tListYear first, int year, char *type, tList node)
{
    if (first == NULL || year > first->year)
    {
        int class=0;
        if(strcmp(type, "movie") == 0)
            class = 1;
        else if(strcmp(type, "tvSeries") == 0)
            class = 2;
        else
            return NULL;

        tListYear newYear = calloc(1, sizeof(tNodeYear));
        if (newYear == NULL)
            noMemoryAbort();

        newYear->year = year;
        newYear->tail = first;
        if (class==1){
            newYear->firstMovies = node;
            char * genre=strtok(node->genres, ",");
            while(genre!=NULL) {
                newYear->firstGenre = addRecGenre(newYear->firstGenre, genre);
                genre = strtok(NULL, ",");
            }
        }
        else
            newYear->firstSeries = node;

        return newYear;
    }
    if (year == first->year)
    {
        if (strcmp(type, "movie") == 0){
            first->firstMovies = addRecType(first->firstMovies, node);
            char * genre=strtok(node->genres, ",");
            while(genre!=NULL){
                first->firstGenre=addRecGenre(first->firstGenre, genre);
                genre=strtok(NULL, ",");
            }
        }
        else
            first->firstSeries = addRecType(first->firstSeries, node);
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

void query2(FILE * arch, imdbADT imdb) {
    imdb->current = toBegin(imdb);
    fprintf(arch, "%s;%s;%s", "year", "genre", "films");
    while (hasNext(imdb)) {
        tListYear year = next(imdb);
        tListGenre aux = year->firstGenre;
        for (; aux != NULL; aux = aux->tail) {
            fprintf(arch, "%d;%s;%d", year->year, aux->genre, aux->cantMovies);
        }
    }
}

void query3(FILE *arch, imdbADT imdb){
    fprintf(arch, "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie\n");
    toBegin(imdb);
    tListYear aux;
    while(hasNext(imdb)) {
        aux=next(imdb);
        if(aux->numMovies == 0)
            fprintf(arch, "%u; ; ; ;%s;%lu;%f\n", aux->year, aux->firstSeries->title, aux->firstSeries->votes, aux->firstSeries->rating);
        else if(aux->numSeries == 0)
            fprintf(arch, "%u;%s;%lu;%f; ; ; \n", aux->year, aux->firstMovies->title, aux->firstMovies->votes, aux->firstMovies->rating);
        else
            fprintf(arch, "%u;%s;%lu;%f;%s;%lu;%f\n", aux->year, aux->firstMovies->title, aux->firstMovies->votes, aux->firstMovies->rating, aux->firstSeries->title, aux->firstSeries->votes, aux->firstSeries->rating);
    }
    fclose(arch);
}