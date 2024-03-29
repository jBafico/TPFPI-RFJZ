#include <stdio.h>
#include <stdlib.h>
#include "imdbTAD.h"
#define ARGAMOUNT 2
#define QUERYAMOUNT 4

enum querys {Q1=0,Q2,Q3,Q4};

void initFiles(FILE **filevec);
int checkarg(int argc);
void noMemoryAbort(void);

int main(int argc, char *argv[])
{
    if (!checkarg(argc))
        return 1;
    imdbADT imdb;
    FILE *arch;
    arch = fopen(argv[1],"r");
    if (arch == NULL)
    {
        fprintf(stderr,"No se pudo abrir el archivo");
        exit(1);
    }
    skipLine(arch);
    imdb=new();
    if(imdb==NULL || add(arch, imdb)==ERROR_CODE)
        noMemoryAbort();
    FILE *q1 = NULL,*q2 = NULL,*q3 = NULL,*q4 = NULL;
    FILE *filevec[] = {q1,q2,q3,q4};
    initFiles(filevec);
    query1(filevec[Q1],imdb);
    query2(filevec[Q2],imdb);
    query3(filevec[Q3],imdb);
    query4(filevec[Q4],imdb);
    freeimdb(imdb);
    return 0;
}

void noMemoryAbort(void)
{
    fprintf(stderr,"No se ha podido reservar memoria");
    exit(1);
}

void initFiles(FILE **filevec)
{
    char *filenames[] = {"query1.csv","query2.csv","query3.csv","query4.csv"};
    for (int i = 0; i < QUERYAMOUNT; i++)
    {
        filevec[i] = fopen(filenames[i],"w");
        if (filevec[i] == NULL)
        {
            fprintf(stderr,"No se ha podido generar el archivo %s\n",filenames[i]);
            // si falla en abrirse algun archivo, cierro los otros
            for ( int j = 0; j < i;j++)
                fclose(filevec[j]);
            exit(1);
        }
    }
}

int checkarg(int argc)
{
    if (argc > ARGAMOUNT || argc < ARGAMOUNT)
    {
        printf("Por favor ingrese un unico archivo a procesar");
        return NOTOK;
    }
    return OK;
}
