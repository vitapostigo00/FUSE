#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "fuseHeaders.h"

extern elementoTabla* globalTable;
extern char* currentPath;
extern char* FUSEINITFILES;

int initEmptyFilesystem(){
    globalTable = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (globalTable == NULL) return 1;
    globalTable -> path = malloc(sizeof(char)*2);

    strcpy(globalTable -> path,"/");

    globalTable -> data = NULL;
    globalTable -> next = NULL;

    currentPath = (char*) malloc(sizeof(char)*LONGESTPATHSIZE);
    FUSEINITFILES = strdup("/FUSEINIT");

    if(currentPath == NULL){
        free(globalTable);
        return 1;
    }
    else{
        strcpy(currentPath,"/");
    }

    return 0;
}

int loadAsDir(const char* token) {
    elementoTabla* copia = globalTable;

    // Buscar el último elemento de la lista
    while (copia->next != NULL) {
        copia = copia->next;
    }

    // Crear un nuevo nodo
    copia->next = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (copia->next == NULL) {
        printf("Couldn't allocate memory for the node...");
        return 1;
    }

    // Inicializar el nuevo nodo
    copia->next->path = strdup(token);
    copia->next->data = NULL;
    copia->next->next = NULL;

    return 0;
}

// Función para guardar un archivo en la tabla (procesar el token)
int loadAsFile(const char* token) {
    
    printf("Entrada: %s\n",token);
    fflush(stdout);

    char* hash_str = malloc(sizeof(char)*LONGESTPATHSIZE);
    strcpy(hash_str,"../bin/");
    char* aux = hash_string(token);
    strcat(hash_str,aux);
    strcat(hash_str,".bin");

    FILE *file = fopen(hash_str, "rb");
    if (file == NULL) {
        perror("No se pudo abrir el archivo");
        return 1;
    }

    // Mover el puntero del archivo al final
    fseek(file, 0, SEEK_END);

    // Obtener la longitud del archivo
    long file_size = ftell(file);
    if (file_size == -1) {
        perror("Error al obtener el tamaño del archivo");
        fclose(file);
        return 1;
    }

    // Volver al inicio del archivo
    rewind(file);

    // Reservar memoria para almacenar el contenido del archivo
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("No se pudo asignar memoria");
        fclose(file);
        return 1;
    }

    // Leer el contenido del archivo en el buffer
    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        perror("No se pudo leer el contenido completo del archivo");
        free(buffer);
        fclose(file);
        return 1;
    }

    // Cerrar el archivo
    fclose(file);

    if (remove(hash_str) != 0) {
        perror("No se pudo borrar el archivo");
        return 1;
    }else{
        printf("se ha borrado lol");
    }
    free(hash_str);
    free(aux);

    elementoTabla* copia = globalTable;
    // Buscar el último elemento de la lista
    while (copia->next != NULL) {
        copia = copia->next;
    }

    // Crear un nuevo nodo
    copia->next = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (copia->next == NULL) {
        printf("Couldn't allocate memory for the node...");
        return 1;
    }

    // Inicializar el nuevo nodo
    copia->next->path = strdup(token);
    copia->next->data = malloc(sizeof(TFiles));

    copia->next->data->binario=buffer;
    copia->next->data->size=file_size;

    copia->next->next = NULL;
    
    return 0;
}

// Función para inicializar desde un archivo binario
int initFromBin(const char* filename) {
    if(initEmptyFilesystem()!=0){
        return 1;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("No se pudo abrir el archivo");
        return 1;
    }

    // Determinar el tamaño del archivo
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Leer el contenido del archivo en un buffer
    char* buffer = (char*)malloc(filesize + 1);
    if (!buffer) {
        perror("Error al asignar memoria para el buffer");
        fclose(file);
        return 1;
    }

    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // Asegurarse de que el buffer sea una cadena válida
    fclose(file);


    // Extraer y procesar las cadenas del buffer
    char* token = strtok(buffer, "|");
    while (token) {
        //printf("Token: %s\n",token);
        if (*token != '|') {
            if(token[strlen(token)-1] == '/'){
                loadAsDir(token);
            }else{
                loadAsFile(token);
            } 
        }
        token = strtok(NULL, "|");
    }

    free(buffer);
    return 0;
}

void exportarABin(const char* nombreArchivo, const char* buffer, size_t tamanio) {
    FILE* archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        perror("Error al crear el archivo");
        return;
    }

    fwrite(buffer, 1, tamanio, archivo);
    fclose(archivo);
}


void cleanFileSystem(){
    elementoTabla* next;
    while(globalTable!=NULL){
        next = globalTable -> next;
        free(globalTable -> path);
        if(globalTable -> data != NULL){
            free(globalTable -> data ->binario);
            free(globalTable -> data);
        }
        free(globalTable);
        globalTable = next;
    }
    printf("FileSystem has been cleaned. Closing.");
}


void saveAllDataFromFiles(){

    elementoTabla* current = (elementoTabla*) globalTable -> next;

    while (current != NULL) {
        if(current -> data!=NULL){
            
            char* hash_str = malloc(sizeof(char)*LONGESTPATHSIZE);
            strcpy(hash_str,"../bin/");
            char* aux = hash_string(current -> path);
            strcat(hash_str,aux);
            strcat(hash_str,".bin");

            FILE* file = fopen(hash_str, "wb");
            if (file == NULL) {
                // Si no se pudo abrir el archivo, imprimir un error.
                perror("Error al abrir el archivo");
                return;
            }
            free(aux);
            free(hash_str);

            fwrite(current -> data -> binario, 1, current -> data -> size , file);

            fclose(file);
        }
        current = current->next;
    }

}

void fileSystemToBin(const char* newBin) {
    // Abre el archivo en modo escritura ("w"), lo cual borra el contenido si existe o crea uno nuevo si no existe.
    FILE *file = fopen(newBin, "wb");

    if (file == NULL) {
        // Si no se pudo abrir el archivo, imprimir un error.
        perror("Error al abrir el archivo");
        return;
    }

    // Primera parte: escribir paths y signo de interrogación final
    elementoTabla* current = globalTable -> next;

    fwrite("|",sizeof(char) ,1, file);

    while (current != NULL) {
        // Escribe el formato en el archivo: |tamaño/path|
        fwrite(current->path, sizeof(char), strlen(current->path), file);
        fwrite("|",sizeof(char) ,1, file);

        // Avanza al siguiente nodo
        current = current->next;
    }

    // Cierra el archivo
    fclose(file);

    saveAllDataFromFiles();
}

void exitFileSystem(const char* newBin){
    fileSystemToBin(newBin);
    cleanFileSystem();
}

int createRawEntry(char* newEntry){

    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(newEntry)+2));
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return 1;
    }

    strcpy(newString,"/"); 
    strcat(newString, newEntry);
    

    // Encontrar el último elemento en la tabla global
    elementoTabla* toAppend = globalTable;
    while (toAppend->next != NULL){
        toAppend = toAppend->next;
    }

    // Crear un nuevo elemento para el nuevo directorio
    toAppend->next = malloc(sizeof(elementoTabla));
    if (toAppend->next == NULL) {
        perror("Error al asignar memoria para nuevo elemento de la tabla");
        free(newString);
        return 1;
    }
    toAppend = toAppend->next;

    // Asignar la ruta al nuevo elemento
    toAppend->path = malloc(sizeof(char)*(strlen(newString)+1));
    if (toAppend->path == NULL) {
        perror("Error al asignar memoria para la ruta del nuevo elemento");
        free(toAppend->next);
        free(newString);
        return 1;
    }
    strcpy(toAppend->path, newString);
    free(newString);

    // Inicializar los otros campos del nuevo elemento
    toAppend->data = NULL;
    toAppend->next = NULL;

    return 0;
}