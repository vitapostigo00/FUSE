//#define FUSE_USE_VERSION 26

#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
//#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>


void showDate(struct tm time) {
    printf("Creation: %d-%02d-%02d %02d:%02d:%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
}


int initFileSystem (){
    clusterActualSize=0;
    dataActualSize=0;
    
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
    }else if(clusterActualSize>=MAXCLUSTERSIZE){
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

void cd(char dir[LONGESTFILENAME-1]){
    if(strcmp(dir,"..")==0){
        if(currentDir==rootElement){
            printf("You are already in root");
        }else{
            currentDir = currentDir -> fatherDir;
        }
        return;
    }

    char filename[LONGESTFILENAME];
    filename[0] = '/';
    strcpy(filename + 1, dir);

    sonElemList* aux = (sonElemList*) currentDir -> clusterPointer;

    while(aux != NULL  && strcmp(filename, aux -> elemento ->filename)!=0){
        aux = aux -> next;
    }

    if(aux == NULL){
        printf("Directorio no encontrado.");
    }else if(strcmp(filename, aux -> elemento -> filename)==0){
        currentDir = aux -> elemento;
    }else{
        printf("Fallo en la matrix");
    }

}

int mkf(char dir[LONGESTFILENAME], char* content){
    if(dir[0]=='/'){
        printf("/ not allowed as first character.");
        return 1;
    }else if(clusterActualSize>=MAXCLUSTERSIZE){
        printf("FAT16 SIZE EXCEEDEED, CLUSTER STORAGE IS FULL.");
        return 1;
    }
    
    unsigned int dataToFill = strlen(content)/BYTESPERCLUSTER + 1;
    if(dataToFill+dataActualSize >= MAXCLUSTERSIZE){
        printf("FAT16 SIZE EXCEEDEED, DATA STORAGE IS FULL.");
        return 1;
    }

    clusterActualSize++;
    dataActualSize += dataToFill;

    clustElem* newElement = (clustElem *) malloc(sizeof(clustElem));
    
    if(newElement==NULL) return 1;

    newElement -> fatherDir = currentDir;

    strcpy(newElement->filename, dir);

    //Actual time//
    time_t t = time(NULL);              //Hay que mirar si se guarda o si hay que hacerle otro malloc.
    struct tm tm = *localtime(&t);
    newElement->fecha = tm;
    //Actual time//

    //Voy a hacerlo primero con el puntero relativo, el absoluto se hará después.

    //Reservamos primero memoria para el primer fragmento de Data:
    currentDir -> clusterPointer = malloc(sizeof(myData));
    if(currentDir -> clusterPointer == NULL) return 1;

    //Puntero casteado a la estructura correspondiente. Hacemos copia de la dirección para no perderlo
    myData* controlP = &(currentDir -> clusterPointer);

    //Si el tamagno del contenido es menor que el tamagno de 1 cluster lo copiamos en 1 cluster y ponemos el puntero
    //mirando a sí mismo para indicar que ha terminado.
    unsigned int i;
    for(i=0; i < dataToFill, i++ ){
        //Última iteración
        if(i == dataToFill - 1 ){
            strncpy(controlP -> infoFichero, &content[i*BYTESPERCLUSTER], strlen(content)%BYTESPERCLUSTER);
            controlP -> next = controlP;//Cuando el puntero se apunta a sí mismo indica fin del archivo.
            return 0;
        }else{
            strncpy(controlP -> infoFichero, &content[i*BYTESPERCLUSTER], BYTESPERCLUSTER);
            controlP -> next = malloc(sizeof(myData));
            if(controlP -> next == NULL){printf("No se ha podido reservar memoria...");return 1;}
            controlP = controlP -> next;
        }
    }
    
    printf("No se deberia llegar aqui");
    return 1;
}

int rmf(char dir[LONGESTFILENAME]){
    return 0;
}

int rmdir(const char* dirName) {

    // Buscar el directorio en el directorio actual
    sonElemList* prev = NULL;
    sonElemList* current = (sonElemList*) currentDir -> clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename + 1, dirName) != 0) {
        prev = current;
        current = current->next;
    }

    // Si no se encontró el directorio
    if (current == NULL) {
        printf("Directorio no encontrado.\n");
        return 1;
    }

    // Comprobar si el directorio está vacío
    if (current->elemento->clusterPointer != NULL) {
        printf("Directorio no está vacío.\n");
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

    printf("Directorio %s eliminado.\n", dirName);
    return 0;
}

int renameDir(const char* oldName, const char* newName) {

    // Buscar el directorio en el directorio actual
    sonElemList* current = (sonElemList*) currentDir->clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename + 1, oldName) != 0) {
        current = current->next;
    }

    // Si no se encontró el directorio
    if (current == NULL) {
        printf("Directorio no encontrado.\n");
        return 1;
    }

    // Verificar que no haya otro directorio con el nuevo nombre en el mismo directorio
    sonElemList* ok = (sonElemList*) currentDir->clusterPointer;
    while (ok != NULL) {
        if (strcmp(ok->elemento->filename + 1, newName) == 0) {
            printf("Directorio con el mismo nombre encontrado.\n");
            return 1;
        }
        ok = ok->next;
    }

    // Cambiar el nombre
    strcpy(current->elemento->filename + 1, newName);
    printf("Directorio renombrado como: %s\n", current->elemento->filename);
    return 0;
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

    cd("Folder 1");
	
    mkdir("kkwete");
	
    ls();

    cd("..");

    ls();

    showDate(rootElement->fecha);

    if(cleanFileSystem()==1){return 1;}

    printf("FileSystem closing propperly.\n");
    return 0;
}
