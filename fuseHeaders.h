#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#include <stdio.h>
#include <stdlib.h>
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
unsigned int dataActualSize;

clustElem* rootElement = NULL;
clustElem* currentDir = NULL;

void showDate(struct tm time);
int initFileSystem ();
int mkdir(char newDir[LONGESTFILENAME-1]);
void ls();
void cd(char dir[LONGESTFILENAME-1]);
int mkf(char dir[LONGESTFILENAME], char* content);
int rmf(char dir[LONGESTFILENAME]);
int rmdir(const char* dirName);
int renameDir(const char* oldName, const char* newName);
int cleanFileSystem();
char* cat(char dir[LONGESTFILENAME]);
//Función privada:
char* getTextFrom(myData* myFile);

#endif // FUSEHEADERS_H