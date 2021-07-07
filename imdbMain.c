#include <stdio.h>
#include <stdlib.h>
#include "imdbTAD.h"

int main(void)
{
    imdbADT content;
    FILE *arch;
    arch = fopen("imdbv2.csv","r");
    if ( arch == NULL )
    {
        fprintf(stderr,"no se pudo abrir el archivo");
        exit(1);
    }
    skipline(arch);
    // crearADT
}







