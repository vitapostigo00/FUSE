#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include "fuseHeaders.h"

void print_time(time_t raw_time) {
    struct tm *timeinfo;
    char buffer[80];

    timeinfo = localtime(&raw_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s\n", buffer);
}

//Nos devuelve el último bloque que esté libre
int nextEmtpyBlock(FileSystemInfo *fs){
    for(int i = 1; i < FILESYSTEM_SIZE; i++){
        if(strcmp(fs[i].path,"")==0){
            return i;
        }
    }
    return -1;
}

//Nos devuelve el último bloque ocupado (ha de tener next a -1)
int lastUsedBlock(FileSystemInfo *fs){

    if(fs[0].siguiente==-1){
        return 0;
    }
    int actual = fs[0].siguiente;
    int next = fs[actual].siguiente;

    while((next)!=-1){
        actual = fs[actual].siguiente;
        next = fs[actual].siguiente;
    }

    if(next == -1 && actual != -1){
        return actual;
    }

    return -1;

}

char* buildFullPathGeneral(const char* filename){
    if(strcmp(filename,".")==0){
        return currentDir -> path;
    }
    if(strcmp(filename,"..")==0){
        if(strcmp(currentDir -> path,"/")==0){
            printf("Ya estás en / ...\n");
            return NULL;
        }

        int i = strlen(currentDir -> path)-2;
        while(i >= 0 && currentDir -> path[i] != '/'){
            i--;
        }
        char* retorno = malloc(sizeof(char)*(i+2));
        int j;
        for (j=0; j < i+1;j++){
            retorno[j] = currentDir -> path[j];
        }
        retorno[i+1]='\0';
        return retorno;
    }
    unsigned int size = strlen(filename) + strlen(currentDir -> path) +2;

    if(size >= LONGEST_FILENAME){
        return NULL;
    }

    char* newPath = (char*)malloc(LONGEST_FILENAME);
    if(newPath==NULL){
        printf("Error al reservar memoria para el path.");
        return NULL;
    }

    strcpy(newPath, currentDir -> path);
    strcat(newPath, filename);

    return newPath;
}


char* buildFullPathDir(const char* filename){
    if(strcmp(filename,".")==0){
        return currentDir -> path;
    }
    if(strcmp(filename,"..")==0){
        if(strcmp(currentDir -> path,"/")==0){
            printf("Ya estás en / ...\n");
            return NULL;
        }

        int i = strlen(currentDir -> path)-2;
        while(i >= 0 && currentDir -> path[i] != '/'){
            i--;
        }
        char* retorno = malloc(sizeof(char)*(i+2));
        int j;
        for (j=0; j < i+1;j++){
            retorno[j] = currentDir -> path[j];
        }
        retorno[i+1]='\0';
        return retorno;
    }
    unsigned int size = strlen(filename) + strlen(currentDir -> path) +2;

    if(size >= LONGEST_FILENAME){
        return NULL;
    }

    char* newPath = (char*)malloc(LONGEST_FILENAME);
    if(newPath==NULL){
        printf("Error al reservar memoria para el path.");
        return NULL;
    }

    strcpy(newPath, currentDir -> path);
    strcat(newPath, filename);
    strcat(newPath,"/");

    return newPath;
}

//Devuelve 0 si la cadena 1 es prefijo de la segunda. -1 en otro caso
int isPrefix(char* prefix, char* secondChain){
    if(strlen(prefix) > strlen(secondChain)){
        return -1;
    }
    int i;
    for(i = 0; i < strlen(prefix); i++){
        if(prefix[i] != secondChain[i]){
            return -1;
        }
    }
    return 0;
}

//Devuelve -1 si ya existía el path, si no devuelve la posicion del array donde se encuentra.
int exists(FileSystemInfo* fs, char* absoluteFilename){
    int current = 0;
    while(fs[current].siguiente != -1 && strcmp(fs[current].path, absoluteFilename)!=0){
        current = fs[current].siguiente;
    }
    if(strcmp(fs[current].path, absoluteFilename)==0){
        return current;
    }
    return -1;
}

void printFileSystemState(FileSystemInfo *fs, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < FILESYSTEM_SIZE; i++) {
        if (fs[i].path[0] != '\0') { // Solo imprimir entradas válidas
            char creationTimeStr[20];
            strftime(creationTimeStr, sizeof(creationTimeStr), "%Y-%m-%d %H:%M:%S", localtime(&fs[i].creation_time));
            fprintf(file, "Index: %d\nPath: %s\nSiguiente: %d\nCreation Time: %s\n\n",
                    i, fs[i].path, fs[i].siguiente, creationTimeStr);
        }
    }

    fclose(file);
}