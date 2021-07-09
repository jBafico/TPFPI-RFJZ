#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ADDED 1
#define NOT_ADDED !ADDED
#define BLOCK 30
#define MINVOTES 100000 //minima cantidad de votos para entrar en mostPopularList
#define MAX_MOVIES 100

enum data {TITLE=1,YEAR=2,GENRES=4,RATING=5,VOTES=6};

typedef struct tNodeYear *tListYear;
typedef struct tNodeMostPopular * tListMostPopular;
typedef struct tNodeGenre * tListGenre;

typedef struct tNodeGenre{
    char * genre;
    unsigned int cantMovies;
    struct tNodeGenre * tail;
}tNodeGenre;

typedef struct tNode
{
    char *title;
    char *genres;
    float rating;
    size_t votes;
}tNode;

typedef struct tNodeYear
{
    unsigned int year;
    unsigned int numMovies;
    unsigned int numSeries;
    tListYear tail;
    tNode mostVotedSeries;
    tNode mostVotedMovie;
    tListGenre firstGenre;
}tNodeYear;

typedef struct tElemMostPopular{
    char *title;
    int year;
    float rating;
    unsigned long votes;
}tElemMostPopular;

typedef struct tNodeMostPopular{
    tElemMostPopular head;
    tListMostPopular tail;
}tNodeMostPopular;

typedef struct tMostPopularData{
    unsigned int size; //el tamano de la lista de mostPopular, maximo 100
    tListMostPopular first;
}tMostPopularData;

typedef struct imdbCDT
{
    tListYear first;
    tListYear current;
    tMostPopularData mostPopularData;
} imdbCDT;

static int checkNULL(void * pointer){
    if (pointer == NULL)
        return ERROR_CODE;
    return OK;
}

static char *getLineNoLimitFile(FILE *arch, int * controlFlag)
{
    int c, i = 0;
    char *s = NULL; // para que el primer realloc funcione como malloc
    while ((c = fgetc(arch)) != '\n')
    {
        if(c==EOF)
            return NULL;
        if (i % BLOCK == 0){
            s = realloc(s, i + BLOCK); // multiplicar por sizeof(char) no es necesario
            *controlFlag=checkNULL(s);
            if(*controlFlag==ERROR_CODE)
                return NULL;
        }
        s[i++] = c;
    }
    s = realloc(s, i + 1); // Liberamos lo que sobra del ultimo bloque
    *controlFlag=checkNULL(s);
    if(*controlFlag==ERROR_CODE)
        return NULL;
    s[i] = 0;
    return s;
}

imdbADT new(){
    return calloc(1, sizeof(imdbCDT));
}

void skipLine(FILE *arch)
{
    while (fgetc(arch) != '\n')
        ;
}

static char *
copy(const char * copyFrom, int * controlFlag){
    unsigned int i=0, j=0;
    char * copyTo = NULL;
    for(; copyFrom[j]!='\0'; i++, j++){
        if(i%BLOCK == 0){
            copyTo=realloc(copyTo, sizeof(char) * (BLOCK + i));
            *controlFlag=checkNULL(copyTo);
            if(*controlFlag==ERROR_CODE)
                return NULL;
        }
        copyTo[i]=copyFrom[j];
    }
    copyTo=realloc(copyTo, sizeof(char) * (i+1));
    *controlFlag=checkNULL(copyTo);
    if(*controlFlag==ERROR_CODE)
        return NULL;
    copyTo[i]='\0';
    return copyTo;
}

static tNode addType(tNode first, tNode node, int * controlFlag)
{
    if (node.votes > first.votes) {
        free(first.title);
        free(first.genres);
        *controlFlag=ADDED;
        return node; //cambio el nodo, lo supero en votos
    }
    *controlFlag=NOT_ADDED;
    return first; //no lo supero en votos, devuelvo el que ya estaba
}

static tListGenre addRecGenre(tListGenre first, char * genre, int * controlFlag){
    int c;
    if(first ==NULL || (c=strcmp(genre, first->genre))<0){
        tListGenre newGenre=malloc(sizeof(tNodeGenre));
        *controlFlag=checkNULL(newGenre);
        if(*controlFlag==ERROR_CODE)
            return NULL;
        newGenre->genre=copy(genre, controlFlag);
        if(*controlFlag==ERROR_CODE)
            return NULL;
        newGenre->tail=first;
        newGenre->cantMovies=1;
        return newGenre;
    }
    if(c==0){
        (first->cantMovies)++;
        return first;
    }
    first->tail=addRecGenre(first->tail, genre, controlFlag);
    return first;
}

static tListYear addRec(tListYear first, int year, char *type, tNode node, int * controlFlag)
{
    if (first == NULL || year > first->year)
    {
        tListYear newYear = calloc(1, sizeof(tNodeYear));
        *controlFlag=checkNULL(newYear);
        if(*controlFlag==ERROR_CODE)
            return NULL;

        if(!strcmp(type, "movie")) {
            newYear->mostVotedMovie = node;
            newYear->numMovies++;
            char *genre = strtok(node.genres, ",");
            while (genre != NULL) {
                newYear->firstGenre = addRecGenre(newYear->firstGenre, genre, controlFlag);
                if (*controlFlag == ERROR_CODE)
                    return NULL;
                genre = strtok(NULL, ",");
            }
        }
        else if(!strcmp(type, "tvSeries")) {
            newYear->mostVotedSeries = node;
            newYear->numSeries++;
        }
        else {
            free(newYear);
            *controlFlag=NOT_ADDED;
            return NULL;
        }
        newYear->year = year;
        newYear->tail = first;
        *controlFlag=ADDED;
        return newYear;
    }
    if (year == first->year)
    {
        if (strcmp(type, "movie") == 0){ // se agrega una pelicula
            first->numMovies++;
            char * genre=strtok(node.genres, ",");
            while(genre!=NULL){
                first->firstGenre=addRecGenre(first->firstGenre, genre, controlFlag);
                if(*controlFlag==ERROR_CODE)
                    return NULL;
                genre=strtok(NULL, ",");
            }
            first->mostVotedMovie = addType(first->mostVotedMovie, node, controlFlag);
            if(*controlFlag==ERROR_CODE)
                return NULL;
        }
        else if (strcmp(type,"tvSeries") == 0){ // se agrega una serie
            first->mostVotedSeries = addType(first->mostVotedSeries, node, controlFlag);
            first->numSeries++;
        }
        return first; // si no era ni pelicula ni serie, simplemente devuelvo lo que estaba pues no hay nada a insertar
    }
    first->tail = addRec(first->tail, year, type, node, controlFlag);
    return first;
}

static tListMostPopular addrecMP(tListMostPopular list,tElemMostPopular elem, int * controlFlag) //va a estar ordenada de men a may rating
{
    if ( list == NULL || list->head.rating > elem.rating || ((list->head.rating == elem.rating) && (elem.votes <= list->head.votes)) ){
        tListMostPopular newnode = malloc(sizeof(tNodeMostPopular));
        *controlFlag=checkNULL(newnode);
        if(*controlFlag==ERROR_CODE)
            return NULL;
        newnode->head = elem;
        newnode->tail = list;
        return newnode;
    }
    list->tail = addrecMP(list->tail,elem, controlFlag);
    return list;
}

static tListMostPopular deletefirst(tListMostPopular first)
{
    tListMostPopular aux = first->tail;
    free(first->head.title);
    free(first);
    return aux;
}

void addMostPopular(imdbADT imdb, tNode newNode, int year, int * controlFlag){
    if (newNode.votes < MINVOTES)
        return;
    if (imdb->mostPopularData.size == MAX_MOVIES && ( (imdb->mostPopularData.first->head.rating > newNode.rating) || ((imdb->mostPopularData.first->head.rating == newNode.rating) && imdb->mostPopularData.first->head.votes >= newNode.votes)) )
        return;

    tNodeMostPopular newNodeMostPopular;
    newNodeMostPopular.head.votes = newNode.votes;
    newNodeMostPopular.head.rating = newNode.rating;
    newNodeMostPopular.head.year = year;
    newNodeMostPopular.head.title = copy(newNode.title, controlFlag);

    if ( imdb->mostPopularData.size == MAX_MOVIES)
        imdb->mostPopularData.first = deletefirst(imdb->mostPopularData.first);
    else
        imdb->mostPopularData.size++;
    imdb->mostPopularData.first = addrecMP(imdb->mostPopularData.first, newNodeMostPopular.head, controlFlag);
}

static void printAndFreeMPrec(tListMostPopular list,FILE *out)
{
    if ( list == NULL)
        return;
    printAndFreeMPrec(list->tail,out);
    fprintf(out,"%d,%s,%lu,%.1f\n",list->head.year,list->head.title,list->head.votes,list->head.rating);
    //Se liberan  los elementos tras insertarse al archivo. Es mas efectivo que recorrer de vuelta la lista
    free(list->head.title);
    free(list);
}

static void freeUnused(tNode nodeToDelete){
    if(checkNULL(nodeToDelete.title)!=ERROR_CODE)
        free(nodeToDelete.title);
    if(checkNULL(nodeToDelete.genres)!=ERROR_CODE)
        free(nodeToDelete.genres);
}

int add(FILE *arch, imdbADT imdb)
{
    char * string;
    int controlFlag;
    while ((string = getLineNoLimitFile(arch, &controlFlag)) != NULL)
    {
        int year;
        char *token = strtok(string, ";");
        char *type= copy(token,&controlFlag);
        tNode newNode;
        // solo leemos datos hasta los votos, salteamos ending year
        for (int i = 0;i<=VOTES && controlFlag != ERROR_CODE; i++)
        {
            switch (i)
            {
                case TITLE:
                    newNode.title = copy(token, &controlFlag);
                    break;
                case YEAR:
                    year = atoi(token);
                    break;
                case GENRES:
                    newNode.genres = copy(token, &controlFlag);
                    break;
                case RATING:
                    newNode.rating = atof(token);
                    break;
                case VOTES:
                    newNode.votes = atoi(token);
                    break;
                default:
                    break;
            }
            token = strtok(NULL, ";");
        }
        free(string);
        if (controlFlag == ERROR_CODE)
        {
            freeUnused(newNode);
            free(type);
            return ERROR_CODE;
        }
        if (year > 0)
        {
            if ( !strcmp(type,"movie") )
            {
                addMostPopular(imdb, newNode, year, &controlFlag);
                if(controlFlag==ERROR_CODE){
                    freeUnused(newNode);
                    free(type);
                    return ERROR_CODE;
                }
            }
            imdb->first = addRec(imdb->first, year, type, newNode, &controlFlag);
            if(controlFlag!=ADDED)
                freeUnused(newNode);
            if(controlFlag==ERROR_CODE){
                free(type);
                return ERROR_CODE;
            }
        }
        else
            freeUnused(newNode);
        free(type);
    }
    return OK;
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
    tListYear toReturn = imdb->current;
    imdb->current = imdb->current->tail;
    return toReturn;
}

void query1(FILE *arch, imdbADT imdb)
{
    fprintf(arch, "year;films;series\n");
    toBegin(imdb);
    tListYear aux;

    while (hasNext(imdb))
    {
        aux = next(imdb);
        fprintf(arch, "%u;%u;%u\n", aux->year, aux->numMovies, aux->numSeries);
    }
    fclose(arch);
}

void query2(FILE * arch, imdbADT imdb) {
    toBegin(imdb);
    fprintf(arch, "%s;%s;%s\n", "year", "genre", "films");
    while (hasNext(imdb)) {
        tListYear year = next(imdb);
        tListGenre aux = year->firstGenre;
        for (; aux != NULL; aux = aux->tail) {
            fprintf(arch, "%d;%s;%d\n", year->year, aux->genre, aux->cantMovies);
        }
    }
    fclose(arch);
}

void query3(FILE *arch, imdbADT imdb){
    fprintf(arch, "startYear;film;votesFilm;ratingFilm;serie;votesSerie;ratingSerie\n");
    toBegin(imdb);
    tListYear aux;
    while(hasNext(imdb)) {
        aux=next(imdb);
        if(aux->numMovies == 0)
            fprintf(arch, "%u; ; ; ;%s;%lu;%.1f\n", aux->year, aux->mostVotedSeries.title, aux->mostVotedSeries.votes, aux->mostVotedSeries.rating);
        else if(aux->numSeries == 0)
            fprintf(arch, "%u;%s;%lu;%.1f; ; ; \n", aux->year, aux->mostVotedMovie.title, aux->mostVotedMovie.votes, aux->mostVotedMovie.rating);
        else
            fprintf(arch, "%u;%s;%lu;%.1f;%s;%lu;%.1f\n", aux->year, aux->mostVotedMovie.title, aux->mostVotedMovie.votes, aux->mostVotedMovie.rating, aux->mostVotedSeries.title, aux->mostVotedSeries.votes, aux->mostVotedSeries.rating);
    }
    fclose(arch);
}

void query4(FILE* out,imdbADT imdb)
{
    fprintf(out,"startYear;primaryTitle;numVotes;averageRating\n");
    printAndFreeMPrec(imdb->mostPopularData.first,out);
    fclose(out);
}

static void freeRecGenre(tListGenre type){
    if(type==NULL)
        return;
    freeRecGenre(type->tail);
    free(type->genre);
    free(type);
}

static void freeRecYear(tListYear year){
    if(year==NULL)
        return;
    freeRecYear(year->tail);
    freeRecGenre(year->firstGenre);
    free(year->mostVotedSeries.title);
    free(year->mostVotedSeries.genres);
    free(year->mostVotedMovie.genres);
    free(year->mostVotedMovie.title);
    free(year);
}

void freeimdb(imdbADT imdb){
    freeRecYear(imdb->first);
    free(imdb);
}
