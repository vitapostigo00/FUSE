#define FUSE_USE_VERSION 26

//#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
//#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

//Constantes:
#define LONGESTFILENAME 32
#define MAXCLUSTERSIZE  65536       //2^16 (FAT16)
#define BYTESPERCLUSTER 32768       //32KB por cluster

typedef struct ClusterElement{
    struct ClusterElement* fatherDir;       // Solo hay un padre por directorio.
    char filename[LONGESTFILENAME];                        // Nombre del archivo, será único (debe dar fallo si se intentan 2 iguales en una misma carpeta.)
    struct tm fecha;                        // Numero de segundos desde el 1 de enero de 1970 hasta la creació del archivo (para fecha)
    // Si es un directorio (fileName[0]=='/' entonces el puntero mira a una linkedList de ClusterElement que serán los hijos)
    // en caso contrario, el puntero es a Data. Como no sabemos a priori la aritmética, lo ponemos a void y después lo casteamos.
    void* clusterPointer;                   // Puntero a los datos (si es un archivo)
} clustElem;

typedef struct childElements{
    struct childElements* next;
    struct ClusterElement* elemento;
} sonElemList;

typedef struct Data{
    char infoFichero [BYTESPERCLUSTER];
    struct Data* next;
} myData;

//Variables globales: (inicializado en init)
unsigned int clusterActualSize;
clustElem* rootElement = NULL;
clustElem* currentDir = NULL;

void showDate(struct tm time) {
  printf("Creation: %d-%02d-%02d %02d:%02d:%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
}
