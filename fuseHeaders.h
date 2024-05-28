#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

typedef struct Files{
    unsigned long size;       // Tamaño de los datos binarios
    char *data;               // Puntero a los datos binarios
} TFiles;

typedef struct tabla{
    char* path;
    //struct tm fechaCreacion;
    struct Files* data;
    struct tabla* next;
}elementoTabla;

#endif // FUSEHEADERS_H

/*
PATHLENGTH,PATH|PATHLENGTH,PATH|PATHLENGTH,PATH?
PATHLENGTH,PATH,SIZE,DATA|PATHLENGTH,PATH,SIZE,DATA|/0/0/0/0/0/0/0/0
*/