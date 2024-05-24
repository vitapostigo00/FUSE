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

    //Lo casteamos de vuelta como puntero a void para guardarlo en la estructura.
    rootElement->clusterPointer = NULL;

    clusterActualSize++;
    currentDir = rootElement;            //Root es el directorio actual.

    return 0;
}


int mkdir(char newDir[LONGESTFILENAME-1]){

    //habrá que mirar si es relativa o absoluta la dirección viendo si se pasa un parámetro o 2 (creo).
    if(newDir[0]=='/'){
        printf("/ not allowed as first character.");
        return 1;
    }else if(clusterActualSize>=65536){
        printf("FAT16 SIZE EXCEEDEED, STORAGE IS FULL.");
        return 1;
    }

    clustElem* newElement = (clustElem *) malloc(sizeof(clustElem));
    
    if(newElement==NULL) return 1;

    newElement -> fatherDir = currentDir;

    //Copiamos el nombre a partir del primer elemento
    newElement->filename[0] = '/';
    strcpy(newElement->filename + 1, newDir);

    //Actual time//
    time_t t = time(NULL);              //Hay que mirar si se guarda o si hay que hacerle otro malloc.
    struct tm tm = *localtime(&t);
    newElement->fecha = tm;
    //Actual time//

    newElement -> clusterPointer = NULL;

    //Voy a hacerlo primero con el puntero relativo, el absoluto se hará después.

    //Añadimos a la lista de directorios hijos del nodo padre
    //Puntero casteado a la estructura correspondiente. Hacemos copia de la dirección para no perderlo
    sonElemList* controlP = &(currentDir -> clusterPointer);

    //Avanzamos en la lista hasta el último nodo. Si la lista estaba vacía, la declaramos.
    if(controlP != NULL){
        while(controlP -> next != NULL){
            controlP = controlP -> next;  //Recorremos la lista para insertar al final.
        }
        controlP -> next = (sonElemList*) malloc(sizeof(sonElemList));
        controlP = controlP -> next;
    }else{
        controlP = (sonElemList*) malloc(sizeof(sonElemList));
    }

    controlP -> next = NULL;
    controlP -> elemento = newElement;

    clusterActualSize++;

    return 0;
}

void ls(){

    printf("Directorio actual: %s\n",currentDir -> filename);

    sonElemList* aux = (sonElemList*) currentDir -> clusterPointer;

    while(aux!=NULL){
        printf("%s     ",aux -> elemento ->filename);
        fflush(stdout);
        aux = aux -> next;
    }
    printf("\n");
    fflush(stdout);
    return;
}

<<<<<<< Updated upstream
int rmdir(const char* dirName) {
    if (dirName[0] == '/') {
        printf("/ not allowed as first character for a directory.\n");
        return 1;
    }

    // Buscar el directorio en el directorio actual
    sonElemList* prev = NULL;
    sonElemList* current = (sonElemList*) currentDir->clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename + 1, dirName) != 0) {
        prev = current;
        current = current->next;
    }

    // Si no se encontró el directorio
    if (current == NULL) {
        printf("Directory not found.\n");
        return 1;
    }

    // Comprobar si el directorio está vacío
    if (current->elemento->clusterPointer != NULL) {
        printf("Directory is not empty.\n");
        return 1;
    }

    // Eliminar el directorio de la lista de hijos
    if (prev == NULL) {
        currentDir->clusterPointer = current->next;
    } else {
        prev->next = current->next;
    }

    // Liberar la memoria del directorio
    free(current->elemento);
    free(current);

    clusterActualSize--;

    printf("Directory %s deleted.\n", dirName);
    return 0;
=======
void cd(char dir[LONGESTFILENAME-1]){

    char filename[LONGESTFILENAME];
    newElement->filename[0] = '/';
    strcpy(filename + 1, newDir);

>>>>>>> Stashed changes
}


int cleanFileSystem(){//Solo libera root, si se ha declarado otro se queda suelto en memoria.
    free(rootElement);
    return 0;
}


int main(int argc, char *argv[]){

    //Error in memory allocation.
    if(initFileSystem()==1){return 1;}

    mkdir("Folder 1"); 
    mkdir("Folder 2");
    
    ls();
<<<<<<< Updated upstream
	
	rmdir("Folder 1");
	
    ls();
=======

    //ls();
>>>>>>> Stashed changes

    showDate(rootElement->fecha);

    if(cleanFileSystem()==1){return 1;}

    printf("FileSystem closing propperly.\n");
    return 0;
}
