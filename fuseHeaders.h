#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

// Constantes:
#define LONGESTFILENAME 32
#define MAXCLUSTERSIZE  65536 // 2^16 (FAT16)
#define BYTESPERCLUSTER 4 // 32KB por cluster cambiado a 4 para pruebas

typedef struct ClusterElement {
    struct ClusterElement* fatherDir; // Solo hay un padre por directorio.
    char filename[LONGESTFILENAME]; // Nombre del archivo, será único (debe dar fallo si se intentan 2 iguales en una misma carpeta.)
    struct tm fecha; // Numero de segundos desde el 1 de enero de 1970 hasta la creación del archivo (para fecha)
    void* clusterPointer; // Puntero a los datos (si es un archivo)
} clustElem;

typedef struct childElements {
    struct childElements* next;
    struct ClusterElement* elemento;
} sonElemList;

typedef struct Data {
    char infoFichero[BYTESPERCLUSTER];
    struct Data* next;
} myData;

// Variables globales: (inicializado en init)
unsigned int clusterActualSize;
unsigned int dataActualSize;

unsigned short checkBeforeCat;

clustElem* rootElement;
clustElem* currentDir;

void showDate(struct tm time);
int initFileSystem();
int mkdir(const char* newDir);
void ls();
void cd(const char* dir);
int mkf(const char* dir, const char* content);
int rmf(const char* dir);
int rmdir(const char* dirName);
int renameDir(const char* oldName, const char* newName);
int cleanFileSystem();
char* cat(const char* dir);
// Función privada:
void remove_allocated_chars(char* str);

#endif // FUSEHEADERS_H
