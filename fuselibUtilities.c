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
    while(copia != NULL && strcmp(copia -> path, path) != 0){
        copia = copia -> next;
    }
    return copia;
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
    unsigned short int len = strlen(filename);
    int i;  int savei;
    for (i=len-2; i>=0; i--){
        if(filename[i]=='/'){
            savei = i;
            break;
        }
    }

    char* nuevoString = malloc(sizeof(char)*(savei+1));

    for(int i = 0; i <= savei; i++){
        nuevoString[i] = filename[i];
    }
    nuevoString[savei+1]='\0';    

    strcpy(filename,nuevoString);

}

// Función para verificar si un string empieza con un prefijo dado
int startsWith(const char *str, const char *prefix) {
    while (*prefix) {
        if (*prefix++ != *str++) {
            return 0;
        }
    }
    return 1;
}

// Función para reemplazar el prefijo en una lista enlazada
void cambiarHijos(const char *path, const char *newPrefix) {
    elementoTabla* head = (elementoTabla*) globalTable;

    while (head != NULL) {
        if (startsWith(head->path, path)) {
            // Creamos un nuevo string con el prefijo reemplazado
            size_t newLen = strlen(newPrefix) + strlen(head->path) - strlen(path) + 1;
            char *newStr = (char *)malloc(newLen);
            strcpy(newStr, newPrefix);
            strcat(newStr, head->path + strlen(path));

            // Liberamos el string viejo y asignamos el nuevo
            free(head->path);
            head->path = newStr;
        }
        head = head->next;
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
        remove_last_elementArg(newFrom);

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

void mostrarTodo(){
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL){
        printf("Path: %s\n",copia->path);
        copia = copia -> next;
    }
}