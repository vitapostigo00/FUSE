#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuseHeaders.h"

int initEmptyFilesystem(){
    globalTable = malloc(sizeof(elementoTabla));
    if (globalTable == NULL) return 1;
    globalTable -> path = malloc(sizeof(char)*2);

    strcpy(globalTable -> path,"/\0");

    globalTable -> data = NULL;
    globalTable -> next = NULL;

    currentPath = (char*) malloc(sizeof(char)*LONGESTPATHSIZE);

    if(currentPath == NULL){
        free(globalTable);
        return 1;
    }

    return 0;
}

elementoTabla* pathExists(char* path){
    elementoTabla* copia = (elementoTabla*) &globalTable;
    while(copia != NULL && strcmp(copia -> path, path) == 1){
        copia = copia -> next;
    }
    return copia; //Devuelve NULL si no existe
}

char* checksPrevios(char* newDir){
    if(strchr(newDir, '/')){
        return "The new directory can't contain the: / sign.";
    }
    if(strchr(newDir, '?')){
        return "The new directory can't contain the: ? sign.";
    }
    if(strlen(newDir) + strlen(currentPath) >= LONGESTPATHSIZE - 1){
        return "Path is too long for the given dir to create.";
    }
    
    return NULL;
}


int createDir(char* newDir){
    char* msgError = checksPrevios(newDir);
    if(msgError!=NULL){
        printf("%s",msgError);
    }
    strcat(newDir, currentPath);
    if(pathExists(newDir)!=NULL){
        printf("El elemento a crear ya existe");
    }



    return 0;
}





int main(int argc, char **argv) {
    int initialization;

    if(argc == 1){
        initialization = initEmptyFilesystem();
    }
    /*
    else{
        initialization = initFilesystemFromBinary();
    }*/

    if(initialization == 0){
        printf("Filesystem propperly mounted\n");
    }else{
        printf("Error at init, aborpting.\n");
    }

    return initialization;
}
