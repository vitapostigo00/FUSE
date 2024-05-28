#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>

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
    checkBeforeCat = 0;
    currentDir = rootElement;            //Root es el directorio actual.

    return 0;
}


int makedir(char newDir[LONGESTFILENAME-1]){

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
    sonElemList* controlP = (sonElemList*) &(currentDir -> clusterPointer);

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

    while(aux != NULL){
        printf("%s     ",aux -> elemento -> filename);
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

    //Ponemos el puntero de la carpeta padre mirando al nuevo fichero:
    sonElemList* controlP = (sonElemList*) &(currentDir -> clusterPointer);

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


    //Si cabe en una sola estructura... (ha de ser menor estricto para poder guardar el \0)
    if(strlen(content) < BYTESPERCLUSTER){
        newElement -> clusterPointer = malloc(sizeof(myData));
        if(newElement -> clusterPointer == NULL) return 1;

        myData* dataCopy = (myData*) &(newElement -> clusterPointer);
        dataCopy -> next = dataCopy;
        strcpy(dataCopy -> infoFichero, content);
        return 0;
    }

    unsigned int i;
    newElement -> clusterPointer = malloc(sizeof(myData));
    if(newElement -> clusterPointer == NULL) return 1;
    myData* dataCopy = (myData*) &(newElement -> clusterPointer);
    for(i=0 ; i < dataToFill; i++){
        unsigned int w;
        if(i+1 == dataToFill){
            for (w = 0; w < (strlen(content)%BYTESPERCLUSTER)-1 ; w++){
                dataCopy -> infoFichero[w] = content[i*BYTESPERCLUSTER+w];
            }
            dataCopy -> infoFichero[BYTESPERCLUSTER-1] = '\0';
            dataCopy -> next = dataCopy;
        }else{
            for (w = 0; w < BYTESPERCLUSTER-1 ; w++){
                dataCopy -> infoFichero[w] = content[i*BYTESPERCLUSTER+w];
            }
            dataCopy -> infoFichero[BYTESPERCLUSTER-1] = '\0';
            dataCopy -> next = (myData*) malloc(sizeof(myData));
            if(dataCopy -> next == NULL) return 1;
            dataCopy = dataCopy -> next;
        }

    }
    return 0;






































}

void remove_allocated_chars(char *str) {
    int length = strlen(str);

    // Desplazar los caracteres restantes hacia el inicio del string
    for (int i = 0; i < length - BYTESPERCLUSTER + 1; i++) {
        str[i] = str[i + BYTESPERCLUSTER];
    }
}

char* cat(char filename[LONGESTFILENAME]){
    if(filename[0]=='/'){
        return "Folders can't retrieve any text, only files can.";
    }

    sonElemList* aux = (sonElemList*) currentDir -> clusterPointer;

    while(aux != NULL  && strcmp(filename, aux -> elemento -> filename)!=0){
        aux = aux -> next;
    }

    if(aux == NULL){
        return "Archivo no encontrado.";
    }
    else if(strcmp(filename, aux -> elemento -> filename) == 0){
       // printf("Archivo con nombre: %s\n" , aux -> elemento -> filename);
        myData* textoCat = (myData*) &(aux -> elemento -> clusterPointer);
        unsigned int contador = 0;
        unsigned int longitud = 1;
        while(textoCat != textoCat -> next){
            contador++;
            longitud+=strlen(textoCat -> infoFichero);
            textoCat = textoCat -> next;
        }

        textoCat = (myData*) &(aux -> elemento -> clusterPointer);
        char* buffer = malloc(sizeof(char)*longitud);
        if(buffer == NULL){return "No hay espacio para el buffer de texto";}
        buffer[0] = '\0';
        unsigned int i;

        for(i = 0; i <= contador; i++ ){
            strcat(buffer, textoCat -> infoFichero);
            textoCat = textoCat -> next;
        }
        checkBeforeCat = 1;
        return buffer;
    }else{
        return "Logic issue, this message shouldn't be shown in screen";
    }

}

int rmf(char filename[LONGESTFILENAME]){
    // Buscar el directorio en el directorio actual
    sonElemList* prev = NULL;
    sonElemList* current = (sonElemList*) currentDir -> clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename, filename) != 0) {
        prev = current;
        current = current->next;
    }

    // Si no se encontró el directorio
    if (current == NULL) {
        printf("Directorio no encontrado.\n");
        return 1;
    }

    // Comprobar si el directorio está vacío
    if (current->elemento->clusterPointer == NULL) {
        printf("Fallo en la estructura de datos. No hay datos asociados al fichero...\n");
        return 1;
    }

    // Eliminar el directorio de la lista de hijos
    if (prev == NULL) {
        currentDir->clusterPointer = current->next;
    } else {
        prev->next = current->next;
    }

    myData* copy = (myData*) &(current -> elemento -> clusterPointer);
    
    while(copy -> next != copy){
        myData* previo = (myData*) copy;
        copy = copy -> next;
        free(previo);
    }

    // Liberar la memoria del directorio
    free(current->elemento);
    free(current);

    clusterActualSize--;

    printf("Fichero %s eliminado.\n", filename);
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
    
    makedir("Folder 1"); 
    makedir("Folder 2");

    cd("Folder 1");

    mkf("arxivo","holasoyvitaestoyprobandolosarchivoscontamagno4\n");

;

    ls();

    //Hemos de liberar el puntero al archivo despues de imprimirlo ya que tenemos que darle memoria para poder
    //pasarlo entre funciones.
    
    char* toPrintCat = cat("arxivo");
    printf("Arch1: %s\n", toPrintCat);
    if(checkBeforeCat){
        free(toPrintCat);
        checkBeforeCat = 0;
    }
    
    rmf("arxivo");

    ls();

    cd("..");

    rmdir("Folder 1");

    ls();

    //showDate(rootElement->fecha);

    if(cleanFileSystem()==1){return 1;}

    printf("FileSystem closing propperly.\n");
    //fflush(stdout);
    return 0;
}
