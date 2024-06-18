#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LONGEST_FILENAME 64  // Tamaño de path más largo permitido
#define FILESYSTEM_SIZE 1024 // Número de entradas en el sistema de archivos

#define DATASYSTEM_SIZE 1024 // Número de entradas en el sistema de datos
#define BLOCKSIZE 128       // Numero de bytes por bloque

typedef struct info {
    char path[LONGEST_FILENAME];
    int siguiente;
    time_t creation_time;
    time_t last_access;             
    time_t last_modification;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    nlink_t nlink;
    int hasData;
} FileSystemInfo;

extern FileSystemInfo* fs;
extern size_t filesize;
extern int fd;
extern struct stat st;

extern FileSystemInfo* currentDir;

typedef struct data{
	int firstDataBlock;
    int currentBlockSize;
	unsigned long totalSize;
	char data[BLOCKSIZE];
	int siguiente;
} DataSystemInfo;

extern DataSystemInfo *ds;
extern size_t dataFilesize;
extern int dataFd;
extern struct stat dataSt;

// Declaraciones de funciones de fileSystemLib.c
void initialize_filesystem();
void init(const char *filename);
void cleanup();
void changeDirectory(const char* newDir);
int createDir(const char* filename);
void removeDir(const char* filename);
//int renameItem(FileSystemInfo *fs, const char* oldName, const char* newName);
void borrar(const char* absolutePath);


// Separador entre las declaraciones de fileSystemLib y fileSystemUtils
// Declaraciones de funciones de fileSystemUtils.c
int exists(const char* absoluteFilename);
void print_time(time_t raw_time);
int nextEmptyBlock();
int lastUsedBlock();
char* buildFullPath(const char* filename);
//char* buildFullPathDir(const char* filename);
int isPrefix(const char* prefix, const char* secondChain);
void printFileSystemState(const char *filename);
int subdir_inmediato(const char* parent,const char* child);
void ultimoElemento(const char *cadena, char *resultado);

void initialize_datasystem();
void init_datasystem(const char *filename);
int primerElementoLibre();
int hayEspacio(int numBloques);
int copiarFichero(int primBloque,FILE* archivo,long tamano,int blockNumToWrite);
int insertData(char* filename);
char* cat(int data);
void escribirArchivoBinario(const char* nombreArchivo, int data, size_t tamano);
void borrarFile(int data);
size_t sizeOfFile(int data);

#endif
