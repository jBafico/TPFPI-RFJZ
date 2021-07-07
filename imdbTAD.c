#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCK 10
#define MINVOTES 100000 //minima cantidad de votos para entrar en mostPopularList

typedef struct tNode *tList;
typedef struct tNodeYear *tListYear;
typedef struct tNodeMostPopular * tListMostPopular;

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

typedef struct tElemMostPopular{
    char *title;
    int year;
    float rating;
    unsigned long votes;
}tElemMostPopular;

typedef struct tNodeMostPopular{
    tElemMostPopular head;
    struct tNodeMostPopular * tail;
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
        if (class==1)
            newYear->firstMovies = node;
        else
            newYear->firstSeries = node;
        return newYear;
    }
    if (year == first->year)
    {
        if (strcmp(type, "movie") == 0)
            first->firstMovies = addRecType(first->firstMovies, node);
        else
            first->firstSeries = addRecType(first->firstSeries, node);
        return first;
    }
    first->tail = addRec(first->tail, year, type, node);
    return first;
}

static tListMostPopular addrecMP(tListMostPopular list,tElemMostPopular elem) //va a estar ordenada de men a may rating
{
    if ( list == NULL || list->head.rating > elem.rating || ((list->head.rating == elem.rating) && (elem.votes <= list->head.votes))  ){
        //que pasa cuando el tamano ya era 100
        tListMostPopular newnode = malloc(sizeof(tNodeMostPopular));
        newnode->head = elem;
        newnode->tail = list;
        return newnode;
    }
    list->tail = addrecMP(list->tail,elem);
    return list;
}

static tListMostPopular deletefirst(tListMostPopular first)
{
    tListMostPopular aux = first->tail;
    free(first);
    return aux;
}

void addMostPopular(imdbADT imdb, tList newNode, int year){
    if (newNode->votes < MINVOTES)
        return;
    if (imdb->mostPopularData.size == 100 && ( (imdb->mostPopularData.first->head.rating > newNode->rating) || ((imdb->mostPopularData.first->head.rating == newNode->rating) && imdb->mostPopularData.first->head.votes >= newNode->rating) ) ){
        return;
    }

    tListMostPopular newNodeMostPopular = malloc(sizeof(tNodeMostPopular));
    if (newNodeMostPopular == NULL) //podemos hacer una funcion aux para esto
        noMemoryAbort();

    tElemMostPopular newElem = newNodeMostPopular->head;
    //esto lo puedo convertir a funcion aux ---
    newElem.title = newNode->title;//copie el puntero de newnode, cuidado de no freear antes de tiempo o modificar el string
    newElem.votes = newNode->votes;
    newElem.rating = newNode->rating;
    newElem.year = year;
    if ( imdb->mostPopularData.size == 100)
    {
        imdb->mostPopularData.first = deletefirst(imdb->mostPopularData.first);
    }
    else{
        imdb->mostPopularData.size++;
    }
    imdb->mostPopularData.first = addrecMP(imdb->mostPopularData.first, newElem);
}

void printMostPopularRec(tListMostPopular list,FILE *out)
{
    if ( list == NULL)
        return;
    printMostPopularRec(list->tail,out);
    fprintf(out,"%d,%s,%f,%lu\n",list->head.year,list->head.title,list->head.rating,list->head.votes);
}

//funcion que copie los datos de la lista a un archivo
void query4(FILE* out,imdbADT imdb)
{
    fprintf(out,"startYear;primaryTitle;numVotes;averageRating\n");
    printMostPopularRec(imdb->mostPopularData.first,out);
    fclose(out);
}

void add(FILE *arch, imdbADT imdb)
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
        addMostPopular(imdb, newNode, year);
        imdb->first = addRec(imdb->first, year, type, newNode);
    }
}

