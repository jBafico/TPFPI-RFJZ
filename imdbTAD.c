#include "imdbTAD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCK 10
#define MINVOTES 100000 //minima cantidad de votos para entrar en mostPopularList

typedef struct tNode *tList;
typedef struct tNodeYear *tListYear;
typedef struct tNodeMostPopular * tListMostPopular;

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
}tNode;

typedef struct tNodeYear
{
    unsigned int year;
    unsigned int numMovies;
    unsigned int numSeries;
    tListYear tail;
    tList firstSeries;
    tList firstMovies;
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
        (first->cantMovies)++;
        return first;
    }
    first->tail=addRecGenre(first->tail, genre);
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
            char * genre=strtok(node->genres, ",");
            while(genre!=NULL) {
                newYear->firstGenre = addRecGenre(newYear->firstGenre, genre);
                genre = strtok(NULL, ",");
            }
        }
        else{
            newYear->firstSeries = node;
            newYear->numSeries++;
        }
        return newYear;
    }
    if (year == first->year)
    {
        if (strcmp(type, "movie") == 0){ // se agrega una pelicula
            first->firstMovies = addRecType(first->firstMovies, node);
            first->numMovies++;
            char * genre=strtok(node->genres, ",");
            while(genre!=NULL){
                first->firstGenre=addRecGenre(first->firstGenre, genre);
                genre=strtok(NULL, ",");
            }
        }
        else if (strcmp(type,"tvSeries") == 0){ // se agrega una serie
            first->firstSeries = addRecType(first->firstSeries, node);
            first->numSeries++;
        }
        return first; // si no era ni pelicula ni serie, simplemente devuelvo lo que estaba pues no hay nada a insertar
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

static void printMostPopularRec(tListMostPopular list,FILE *out)
{
    if ( list == NULL)
        return;
    printMostPopularRec(list->tail,out);
    fprintf(out,"%d,%s,%f,%lu\n",list->head.year,list->head.title,list->head.rating,list->head.votes);
    //todo preguntara a zaka si hay que freear algo mas de su lista o si uso igualdad de punteros
    free(list);
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
                newNode->title = copy(token);
                break;
            case 2:
                year = atoi(token);
                break;
            case 4:
                newNode->genres = copy(token);
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
static void freeRecGenre(tListGenre type){
    if(type==NULL)
        return;
    freeRecGenre(type->tail);
    free(type->genre);
    free(type);
}

static void freeRecType(tList type){
    if(type==NULL)
        return;
    freeRecType(type->tail);
    free(type->title);
    free(type->genres);
    free(type);
}

static void freeRecYear(tListYear year){
    if(year==NULL)
        return;
    freeRecYear(year->tail);
    freeRecGenre(year->firstGenre);
    freeRecType(year->firstSeries);
    freeRecType(year->firstMovies);
    free(year);
}

void freeimdb(imdbADT imdb){
    freeRecYear(imdb->first);
    free(imdb);
}