#include "fuseHeaders.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_time(time_t raw_time) {
    struct tm *timeinfo;
    char buffer[80];

    timeinfo = localtime(&raw_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s\n", buffer);
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

// Nos devuelve el último bloque que esté libre
int nextEmptyBlock(){
    for(int i = 1; i < FILESYSTEM_SIZE; i++){
        if(strcmp(fs[i].path,"")==0){
            return i;
        }
    }
    return -1;
}

// Nos devuelve el último bloque ocupado (ha de tener next a -1)
int lastUsedBlock(){
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

char* buildFullPath(const char* filename){          //MIRAR cómo lo genera
    if(strcmp(filename,".")==0){
        return currentDir -> path;
    }
    if(strcmp(filename,"..")==0){
        if(strcmp(currentDir -> path,"/")==0){
            printf("Ya estás en / ...\n");
            return NULL;
        }

        int i = strlen(currentDir -> path)-1;
        while(i >= 0 && currentDir -> path[i] != '/'){
            i--;
        }
        char* retorno = malloc(sizeof(char)*(i+1));
        int j;
        for (j=0; j < i+1 ; j++){
            retorno[j] = currentDir -> path[j];
        }
        retorno[i]='\0';
        return retorno;
    }
    unsigned int size = strlen(filename) + strlen(currentDir -> path) + 1;

    if(size >= LONGEST_FILENAME){
        printf("El nuevo nombre de fichero pasa de la cantidad especificada\n");
        return NULL;
    }

    char* newPath = (char*)malloc(LONGEST_FILENAME);
    if(newPath==NULL){
        printf("Error al reservar memoria para el path.\n");
        return NULL;
    }

    strcpy(newPath, currentDir -> path);
    strcat(newPath, filename);

    return newPath;
}

// Devuelve 0 si la cadena 1 es prefijo de la segunda. -1 en otro caso
int isPrefix(const char* prefix, const char* secondChain){
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

// Función para reemplazar el prefijo de `cadena` con `nuevo_prefijo`
void reemplazar_prefijo(char *cadena, const char *prefijo, const char *nuevo_prefijo) {
    // Verificar si `prefijo` es realmente un prefijo de `cadena`
    if (isPrefix(prefijo, cadena)==0) {
        // Calcular el tamaño de la parte de la cadena después del prefijo
        size_t tamano_prefijo = strlen(prefijo);
        size_t tamano_nuevo_prefijo = strlen(nuevo_prefijo);
        size_t tamano_restante = strlen(cadena) - tamano_prefijo;

        // Mover la parte restante de la cadena hacia adelante
        memmove(cadena + tamano_nuevo_prefijo, cadena + tamano_prefijo, tamano_restante + 1);

        // Copiar el nuevo prefijo en la posición correcta
        memcpy(cadena, nuevo_prefijo, tamano_nuevo_prefijo);
    }
}

char* ultimoElemento(const char *cadena) {
    
    if (strcmp(cadena, "/") == 0) {
        return NULL;
    }

    char* resultado = malloc(sizeof(char) * LONGEST_FILENAME);
    if (resultado == NULL) {
        perror("No se pudo asignar memoria");
        return NULL;
    }

    int longitud = strlen(cadena);
    int i = longitud - 1;

    while (i >= 0 && cadena[i] != '/') {
        i--;
    }
    strncpy(resultado, cadena + i + 1, longitud - i - 1);

    resultado[longitud - i - 1] = '\0';

    return resultado;
}

int exists(const char* absoluteFilename){
    int current = 0;
    while(fs[current].siguiente != -1 && strcmp(fs[current].path, absoluteFilename)!=0){
        current = fs[current].siguiente;
    }
    if(strcmp(fs[current].path, absoluteFilename)==0){
        return current;
    }
    return -1;
}

void printFileSystemState(const char *filename) {
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
