//#define FUSE_USE_VERSION 26

#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
//#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

int initFileSystem (){
    clusterActualSize=0;

    rootElement = (clustElem *) malloc(sizeof(clustElem));
    
    if(rootElement==NULL) return 1;

    rootElement->fatherDir = NULL;
    strcpy(rootElement->filename, "/"); //Root

    //Actual time//
    time_t t = time(NULL);              //Hay que mirar si se guarda o si hay que hacerle otro malloc.
    struct tm tm = *localtime(&t);
    rootElement->fecha = tm;
    //Actual time//

    //Como clusterElem solo tiene un puntero de este tipo, tenemos que castearlo en función de si es un directorio o archivo.
    //si es un directorio (este caso), lo casteamos a sonElemList.
    //en caso contrario, se castea a myData.
    sonElemList* aux = (sonElemList *) malloc(sizeof(sonElemList));

    //No hay directorios dentro así que dejamos ambos punteros a null.
    aux -> next = NULL;
    aux -> elemento = NULL;

    //Lo casteamos de vuelta como puntero a void para guardarlo en la estructura.
    rootElement->clusterPointer = (void*) aux;

    clusterActualSize++;
    currentDir = rootElement;            //Root es el directorio actual.

    return 0;
}

int initFileSystem2 (){
    clusterActualSize=0;

    rootElement = (clustElem *) malloc(sizeof(clustElem));
    
    if(rootElement==NULL) return 1;

    rootElement->fatherDir = NULL;
    strcpy(rootElement->filename, "/"); //Root

    //Actual time//
    time_t t = time(NULL);              //Hay que mirar si se guarda o si hay que hacerle otro malloc.
    struct tm tm = *localtime(&t);
    rootElement->fecha = tm;
    //Actual time//

    //Como clusterElem solo tiene un puntero de este tipo, tenemos que castearlo en función de si es un directorio o archivo.
    //si es un directorio (este caso), lo casteamos a sonElemList.
    //en caso contrario, se castea a myData.
    sonElemList* aux = (sonElemList *) malloc(sizeof(sonElemList));

    //No hay directorios dentro así que dejamos ambos punteros a null.
    aux -> next = NULL;
    aux -> elemento = NULL;

    //Lo casteamos de vuelta como puntero a void para guardarlo en la estructura.
    rootElement->clusterPointer = (void*) aux;

    clusterActualSize++;
    currentDir = rootElement;            //Root es el directorio actual.

    return 0;
}



int mkdir(char newDir[LONGESTFILENAME-1]){
    if(newDir[0]=='/'){
        printf("/ not allowed as first character for a directory.");
        return 1;
    }else if(clusterActualSize>=65536){
        printf("FAT16 SIZE EXCEEDEED, STORAGE IS FULL.");
        return 1;
    }

    clustElem* newElement = (clustElem *) malloc(sizeof(clustElem));
    
    if(newElement==NULL) return 1;

    printf("Ke");

    newElement->fatherDir = currentDir;

    //Copiamos el nombre a partir del primer elemento
    newElement->filename[0] = '/';
    strcpy(newElement->filename + 1, newDir);

    //Actual time//
    time_t t = time(NULL);              //Hay que mirar si se guarda o si hay que hacerle otro malloc.
    struct tm tm = *localtime(&t);
    newElement->fecha = tm;
    //Actual time//

    //Añadimos a la lista de directorios hijos del nodo padre
    //Puntero casteado a la estructura correspondiente.
    sonElemList* controlP = (sonElemList*) currentDir -> clusterPointer;

    while(controlP -> next != NULL){
        controlP = controlP -> next;  //Recorremos la lista para insertar al final.
    }

    controlP -> next = (sonElemList *) malloc(sizeof(sonElemList));
    if(controlP -> next == NULL){return 1;}

    controlP = controlP -> next;//Nos ponemos en la última dirección y añaadimos en la lista de control

    controlP -> next = NULL;
    controlP -> elemento = newElement;

    //Como clusterElem solo tiene un puntero de este tipo, tenemos que castearlo en función de si es un directorio o archivo.
    //si es un directorio (este caso), lo casteamos a sonElemList.
    //en caso contrario, se castea a myData.
    sonElemList* aux = (sonElemList *) malloc(sizeof(sonElemList));

    if(aux==NULL){return 1;}

    //No hay directorios dentro así que dejamos ambos punteros a null.
    aux -> next = NULL;
    aux -> elemento = NULL;

    //Lo casteamos de vuelta como puntero a void para guardarlo en la estructura.
    newElement->clusterPointer = (void*) aux;

    clusterActualSize++;

    return 0;
}

void ls(){
    sonElemList* aux = (sonElemList*) currentDir -> clusterPointer;
    if(aux -> elemento -> filename[0] == NULL){
        printf("Albacete");
    }else if(aux -> elemento -> filename[0] == '\0'){
        printf("Awa");
    }else{
        printf("%s",aux -> elemento -> filename);
    }
    fflush(stdout);
    return;

    /*
    sonElemList* aux = (sonElemList*) currentDir -> clusterPointer;
    while(aux != NULL){
        clustElem* elemento = aux -> elemento;
        printf("%s",elemento -> filename);
        aux = aux -> next;
    }
    */
}


int cleanFileSystem(){//Solo libera root, si se ha declarado otro se queda suelto en memoria.
    free(rootElement);
    return 0;
}


int main(int argc, char *argv[]){

    //Error in memory allocation.
    if(initFileSystem()==1){return 1;}

    if(mkdir("Paco")==1){return 1;}else{
        printf("A"); fflush(stdout);
    }

    ls();

    showDate(rootElement->fecha);

    if(cleanFileSystem()==1){return 1;}

    printf("FileSystem closing propperly.\n");
    return 0;
}