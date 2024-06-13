#include "fuseHeaders.h"

void print_time(time_t raw_time) {
    struct tm *timeinfo;
    char buffer[80];

    timeinfo = localtime(&raw_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("%s\n", buffer);
}

//Nos devuelve el último bloque que esté libre
int nextEmtpyBlock(FileSystemInfo *fs){
    for(int i = 1; i < FILESYSTEM_SIZE; i++){
        if(strcmp(fs[i].path,"")==0){
            return i;
        }
    }
    return -1;
}

//Nos devuelve el último bloque ocupado (ha de tener next a -1)
int lastUsedBlock(FileSystemInfo *fs){

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

char* buildFullPathGeneral(const char* filename){
    if(strcmp(filename,".")==0){
        return currentDir -> path;
    }
    if(strcmp(filename,"..")==0){
        if(strcmp(currentDir -> path,"/")==0){
            printf("Ya estás en / ...\n");
            return NULL;
        }

        int i = strlen(currentDir -> path)-2;
        while(i >= 0 && currentDir -> path[i] != '/'){
            i--;
        }
        char* retorno = malloc(sizeof(char)*(i+2));
        int j;
        for (j=0; j < i+1;j++){
            retorno[j] = currentDir -> path[j];
        }
        retorno[i+1]='\0';
        return retorno;
    }
    unsigned int size = strlen(filename) + strlen(currentDir -> path) +2;

    if(size >= LONGEST_FILENAME){
        return NULL;
    }

    char* newPath = (char*)malloc(LONGEST_FILENAME);
    if(newPath==NULL){
        printf("Error al reservar memoria para el path.");
        return NULL;
    }

    strcpy(newPath, currentDir -> path);
    strcat(newPath, filename);

    return newPath;
}


char* buildFullPathDir(const char* filename){
    if(strcmp(filename,".")==0){
        return currentDir -> path;
    }
    if(strcmp(filename,"..")==0){
        if(strcmp(currentDir -> path,"/")==0){
            printf("Ya estás en / ...\n");
            return NULL;
        }

        int i = strlen(currentDir -> path)-2;
        while(i >= 0 && currentDir -> path[i] != '/'){
            i--;
        }
        char* retorno = malloc(sizeof(char)*(i+2));
        int j;
        for (j=0; j < i+1;j++){
            retorno[j] = currentDir -> path[j];
        }
        retorno[i+1]='\0';
        return retorno;
    }
    unsigned int size = strlen(filename) + strlen(currentDir -> path) +2;

    if(size >= LONGEST_FILENAME){
        return NULL;
    }

    char* newPath = (char*)malloc(LONGEST_FILENAME);
    if(newPath==NULL){
        printf("Error al reservar memoria para el path.");
        return NULL;
    }

    strcpy(newPath, currentDir -> path);
    strcat(newPath, filename);
    strcat(newPath,"/");

    return newPath;
}

//Devuelve 0 si la cadena 1 es prefijo de la segunda. -1 en otro caso
int isPrefix(char* prefix, char* secondChain){
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
void reemplazar_prefijo(char *cadena, char *prefijo, char *nuevo_prefijo) {
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
    // Clona la cadena para no modificar el original
    char cadena_copia[LONGEST_FILENAME];
    char* resultado = malloc(sizeof(char)*LONGEST_FILENAME);
    strncpy(cadena_copia, cadena, sizeof(cadena_copia) - 1);
    cadena_copia[sizeof(cadena_copia) - 1] = '\0';

    // Encuentra el último '/'
    char *ultimo_slash = strrchr(cadena_copia, '/');

    // Encuentra el penúltimo '/' si existe
    char *penultimo_slash = NULL;
    if (ultimo_slash != NULL) {
        *ultimo_slash = '\0'; // Temporalmente termina la cadena aquí
        penultimo_slash = strrchr(cadena_copia, '/');
    }

    // Restaura el último '/' en la copia de la cadena
    if (ultimo_slash != NULL) {
        *ultimo_slash = '/';
    }

    // Copia el resultado
    if (penultimo_slash != NULL) {
        strncpy(resultado, penultimo_slash + 1, strlen(penultimo_slash + 1) + 1);
    } else {
        strncpy(resultado, cadena, strlen(cadena) + 1);
    }

    return resultado;
}

//Devuelve -1 si no existía el path, si no devuelve la posicion del array donde se encuentra.
int exists(FileSystemInfo* fs, char* absoluteFilename){
    int current = 0;
    while(fs[current].siguiente != -1 && strcmp(fs[current].path, absoluteFilename)!=0){
        current = fs[current].siguiente;
    }
    if(strcmp(fs[current].path, absoluteFilename)==0){
        return current;
    }
    return -1;
}

void printFileSystemState(FileSystemInfo *fs, const char *filename) {
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