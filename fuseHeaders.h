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
#define INITIAL_BLOCK_SIZE 65536

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

typedef struct data{
	int firstDataBlock;
	int currentBlockSize;
	unsigned long totalSize;
	char* data;
	int siguiente;
} dataFile;

extern FileSystemInfo* currentDir;
//~ dataFile* mydata;

// Declaraciones de funciones de fileSystemLib.c
void init(const char *filename, FileSystemInfo **fs, size_t *filesize, int *fd, struct stat *st);
void cleanup(FileSystemInfo *fs, size_t filesize, int fd);
void changeDirectory(FileSystemInfo *fs, const char* newDir);
int createDir(FileSystemInfo *fs, const char* filename);
void removeDir(FileSystemInfo *fs, const char* filename);
int renameItem(FileSystemInfo *fs, const char* oldName, const char* newName);
void borrar(FileSystemInfo *fs, const char* absolutePath);


// Separador entre las declaraciones de fileSystemLib y fileSystemUtils
// Declaraciones de funciones de fileSystemUtils.c
int exists(FileSystemInfo* fs, const char* absoluteFilename);
void print_time(time_t raw_time);
int nextEmptyBlock(FileSystemInfo *fs);
int lastUsedBlock(FileSystemInfo *fs);
char* buildFullPath(const char* filename);
//char* buildFullPathDir(const char* filename);
int isPrefix(const char* prefix, const char* secondChain);
void printFileSystemState(FileSystemInfo *fs, const char *filename);


#endif
