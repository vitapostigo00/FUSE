#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include "fuseHeaders.h"

int initEmptyFilesystem(){
    globalTable = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (globalTable == NULL) return 1;
    globalTable -> path = malloc(sizeof(char)*2);

    strcpy(globalTable -> path,"/");

    globalTable -> data = NULL;
    globalTable -> next = NULL;

    currentPath = (char*) malloc(sizeof(char)*LONGESTPATHSIZE);

    if(currentPath == NULL){
        free(globalTable);
        return 1;
    }
    else{
        strcpy(currentPath,"/");
    }

    return 0;
}

elementoTabla* pathExists(char* path){
    elementoTabla* copia = (elementoTabla*) &globalTable;
    while(copia != NULL && strcmp(copia -> path, path) != 0){
        copia = copia -> next;
    }
    return copia; //Devuelve NULL si no existe
}
elementoTabla* ultimoElemento(){
    elementoTabla* copia = (elementoTabla*) &globalTable;
    while(copia -> next != NULL){
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
    if(strlen(newDir) + strlen(currentPath) >= LONGESTPATHSIZE - 1){
        return "Path is too long for the given dir to create.";
    }
    
    return NULL;
}


int createDir(char* newDir){
    char* msgError = checksPrevios(newDir);
    
    if(msgError!=NULL){
        printf("%s",msgError);
        return 1;
    }

    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(newDir)+2));
    strcpy(newString,currentPath);
    strcat(newString, newDir);
    strcat(newString, "/");

    if(pathExists(newDir)!=NULL){
        printf("El elemento a crear ya existe");
        free(newString);
        return 1;
    }

    elementoTabla* toAppend = (elementoTabla*) &globalTable;

    printf("Header tabla global: %p\n", &globalTable);
    printf("Header toAppend %p\n", toAppend);
    printf("GlobalNext:%p\n",toAppend->next);

    while(toAppend -> next != NULL){
        printf("macarrones\n");
        toAppend = toAppend -> next;
    }

    printf("Header tabla global: %p\n", &globalTable);
    printf("Header toAppend %p\n", toAppend);

    toAppend -> next = (elementoTabla*) malloc(sizeof(elementoTabla));
    printf("PostMalloc:%p\n",toAppend->next);

    toAppend = toAppend -> next;

    printf("Post Next:%p\n",toAppend);

    toAppend -> path = malloc(sizeof(char)*(strlen(newString)+2));
    printf("Path: %p\n",toAppend -> path);
    strcpy(toAppend -> path,newString);
    free(newString);
    printf("Path: %p\n",toAppend -> path);

    printf("Path:%s\n",toAppend -> path);

    //printf("Path: %s\n",toAppend -> path);
    toAppend -> data = NULL;
    toAppend -> next = NULL;

    return 0;
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
        char* rest = child + parent_len + 1;
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

char* ls(){ 
    char* retorno="";
    elementoTabla* paco = (elementoTabla*) (globalTable -> next);


    //printf("GlobalTable p : %p\n", &(globalTable));
    printf("GlobalTable p next: %s\n", (globalTable->path));
    printf("String 1: %s\n",paco -> path);
    fflush(stdout);
    //printf("String 1: %s\nString 2: %s\n",copia->path,copia->next->path);
    printf("HOLAA\n");
    fflush(stdout);
    while(paco != NULL){
        printf("HOLAA2\n");
        fflush(stdout);
        if(paco -> path == NULL){
            printf("LOCOO\n");
            fflush(stdout);
        }else{
            printf("LOCOO2\n");
            fflush(stdout);
        }
        if(subdir_inmediato(currentPath,paco->path)==0){
            printf("Copia path: %s\n",paco -> path);
            fflush(stdout);//ultimoComponente(copia -> path)
            strcat(retorno, "Juan");
            strcat(retorno,"   ");
            printf("Yo si");
        }else{
            printf("Yo no");
        }
        fflush(stdout);
        paco = paco -> next;
    }

    printf("%s\n",retorno);

    return retorno;
}



int main(int argc, char **argv) {
    int initialization;

    initialization = initEmptyFilesystem();
    /*
    else{
        initialization = initFilesystemFromBinary();
    }*/

    if(initialization == 0){
        printf("Filesystem propperly mounted\n");
        createDir("Dir33");
        createDir("Dir2");
        printf("eyy");
        fflush(stdout);

        printf("GT: %p\n",&globalTable); 

        printf("Valor nodo 2: %s\n",globalTable -> next -> path); 
        fflush(stdout);
        //char* copiaLs = ls();
        //printf("%s",copiaLs);
    }else{
        printf("Error at init, aborpting.\n");
    }

    return initialization;
}
