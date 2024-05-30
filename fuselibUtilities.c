#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <libgen.h>

#include "fuseHeaders.h"

elementoTabla* pathExists(char* path){
    elementoTabla* copia = (elementoTabla*) globalTable;
    //printf("Path a matchear:%s\n",path);
    while(copia != NULL && strcmp(copia -> path, path) != 0){
        //printf("ElementoPath:%s\n",copia -> path);
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
    if(strchr(newDir, '.')){
        return "The new directory can't contain the: . sign.";
    }
    if(strlen(newDir) + strlen(currentPath) >= LONGESTPATHSIZE - 1){
        return "Path is too long for the given dir to create.";
    }
    
    return NULL;
}


int subdir_inmediato(const char* parent,const char* child) {
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    if (parent[parent_len - 1] == '/') {
        parent_len--;
    }

    // Asegurarse de que 'child' comienza con 'parent' y que el siguiente caracter es '/'
    if (strncmp(parent, child, parent_len) == 0 && child[parent_len] == '/') {
        // Verificar que solo hay un nivel de directorio de diferencia
        const char* rest = child + parent_len + 1;
        // Verificar que no hay más slashes después del primer nivel
        return (strchr(rest, '/') == NULL || strchr(rest, '/') == rest + strlen(rest) - 1);
    }

    return 0;
}

char* ultimoComponente(char* path) {
    // Hacer una copia de la ruta para no modificar la original
    char* path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("Error al duplicar la ruta");
        return NULL;
    }

    // Utilizar basename para obtener el último componente de la ruta
    char* last_component = basename(path_copy);

    // Hacer otra copia para retornar, porque path_copy será liberada
    char* result = strdup(last_component);
    free(path_copy);  // Liberar la memoria de la copia original

    return result;
}



void remove_last_element() {
    size_t len = strlen(currentPath);

    if (currentPath[len - 1] == '/') {
        currentPath[len - 1] = '\0';
        len--;
    }

    for (size_t i = len - 1; i > 0; i--) {
        if (currentPath[i] == '/') {
            if (i == 0) {
                currentPath[1] = '\0';
            } else {
                currentPath[i + 1] = '\0';
            }
            return;
        }
    }
    
    strcpy(currentPath, "/");
}

void remove_last_elementArg(char* filename) {
    size_t len = strlen(filename);

    if (filename[len - 1] == '/') {
        filename[len - 1] = '\0';
        len--;
    }

    for (size_t i = len - 1; i > 0; i--) {
        if (filename[i] == '/') {
            if (i == 0) {
                filename[1] = '\0';
            } else {
                filename[i + 1] = '\0';
            }
            return;
        }
    }
}

void cambiarHijos(const char* de, const char* a) {
    size_t longitud_directorio = strlen(de);
    elementoTabla* current = globalTable;

    while (current != NULL) {
        if (strncmp(current->path, de, longitud_directorio) == 0 &&
            (current->path[longitud_directorio] == '/' || current->path[longitud_directorio] == '\0')) {
            char* pos = strstr(current->path, de);
            if (pos) {
                size_t longitud_antes = pos - current->path;
                size_t nueva_longitud = strlen(current->path) - strlen(de) + strlen(a) + 1;
                char* nueva_ruta = (char*)malloc(nueva_longitud);
                if (nueva_ruta) {
                    strncpy(nueva_ruta, current->path, longitud_antes);
                    strcpy(nueva_ruta + longitud_antes, a);
                    strcpy(nueva_ruta + longitud_antes + strlen(a), pos + strlen(de));
                    free(current->path);
                    current->path = nueva_ruta;
                }
            }
        }
        current = current->next;
    }
}

//Declara char* en memoria. Hay que liberarlos...
char* absoluteFromRelative(const char* rel){

    if(strcmp(rel,"/")==0){
        return strdup("/\0");
    }
    else if(strcmp(rel,".")==0){
        return strdup(currentPath);
    }
    else if(strcmp(rel,"..")==0){
        char* newFrom = strdup(currentPath);
        remove_last_element(newFrom);
        if(strcmp(currentPath,newFrom)==0){ //Si el dir no cambia es que estamos saltando a root.
            free(newFrom);
            return strdup("/\0");
        }
        return newFrom;
    }
    else{
        char* newFrom = malloc(sizeof(char)*(strlen(currentPath)+strlen(rel)+1));
        //newFrom[0]='\0';
        strcpy(newFrom, currentPath);
        strcat(newFrom, rel);
        return newFrom;
    }
}