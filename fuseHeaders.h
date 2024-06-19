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

///////////////Variables DEFINE///////////////
#define LONGEST_FILENAME 255 // Tamaño de path más largo permitido
#define FILESYSTEM_SIZE 1024 // Número de entradas en el sistema de archivos

#define DATASYSTEM_SIZE 65536// Número de entradas en el sistema de datos
#define BLOCKSIZE 1024       // Numero de bytes por bloque
///////////////Variables DEFINE///////////////
//////////////////////////////ESTRUCTURA DE CONTROL + GLOBALES//////////////////////////////
typedef struct info {
    char path[LONGEST_FILENAME];
    int siguiente;
    time_t creation_time;
    time_t last_access;             
    time_t last_modification;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    int nlink;
    int hasData;
} FileSystemInfo;

extern FileSystemInfo* fs;
extern size_t filesize;
extern int fd;
extern struct stat st;
extern FileSystemInfo* currentDir;
//////////////////////////////ESTRUCTURA DE CONTROL + GLOBALES//////////////////////////////
///////////////////////////////ESTRUCTURA DE DATOS + GLOBALES///////////////////////////////
typedef struct data{
	int firstDataBlock;
    int currentBlockSize;
	unsigned long totalSize;
	char dat[BLOCKSIZE];
	int siguiente;
} DataSystemInfo;

extern DataSystemInfo *ds;
extern size_t dataFilesize;
extern int dataFd;
extern struct stat dataSt;
///////////////////////////////ESTRUCTURA DE DATOS + GLOBALES///////////////////////////////
///////////////////////////////FILESYSTEMLIB.C////////////////////////////////
void initialize_filesystem();
void init(const char *);
void cleanup();
void changeDirectory(const char*);
int createDir(const char*);
int createFile(const char*, const char*, mode_t);
void borrar(const char*);
void deleteElement(const char*);
///////////////////////////////FILESYSTEMLIB.C////////////////////////////////
//////////////////////////////FILESYSTEMUTILS.C///////////////////////////////
int exists(const char*);
int subdir_inmediato(const char*,const char*);
int nextEmptyBlock();
int lastUsedBlock();
void actualizar_padre(int, const char*);
char* padrefrompath(const char* path);
char* buildFullPath(const char*);
int isPrefix(const char*, const char*);
void reemplazar_prefijo(char*, const char*, const char*);
int bloqueslibres();
int nodoslibres();
void ultimoElemento(const char*, char*);
void printFileSystemState(const char *);
//////////////////////////////FILESYSTEMUTILS.C///////////////////////////////
///////////////////////////////DATASYSTEMLIB.C////////////////////////////////
void initialize_datasystem();
void init_datasystem(const char*);
int primerElementoLibre();
int hayEspacio(int);
int copiarSinCheck(int, const char*, int);
int copiarStream(int , const char*, unsigned long , int);
int createEmpty();
int escribirDesdeBuffer(const char*, unsigned long);
int copiarFichero(int, FILE*, long , int);
int insertData(const char*);
char* cat(int);
void escribirArchivoBinario(const char*, int, size_t);
int borrarFile(int);
size_t sizeOfFile(int);
///////////////////////////////DATASYSTEMLIB.C////////////////////////////////
#endif