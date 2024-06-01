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


int initFromBin(const char* myFile) {
    if(initEmptyFilesystem()==1){
        return 1;
    }

    FILE *file;
    char firstChar;
    char numberStr[256];  // Buffer para almacenar el número
    int index;

    // Intenta abrir el archivo en modo binario de lectura
    file = fopen(myFile, "rb");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        return -1;  // Error al abrir el archivo
    }

    // Lee y procesa bloques hasta encontrar un bloque que empieza con '?'
    while (1) {
        if (fread(&firstChar, sizeof(char), 1, file) != 1) {
            printf("Error o fin del archivo alcanzado inesperadamente.\n");
            fclose(file);
            return -1;
        }

        if (firstChar == '?') {
            printf("Encontrado el caracter de terminacion '?'.\n");
            break;
        } else if (firstChar != '|') {
            printf("Formato de archivo inesperado, se esperaba '|', se encontro: %c\n", firstChar);
            fclose(file);
            return -1;
        }

        // Inicializa el buffer y el índice para cada nuevo bloque
        memset(numberStr, 0, sizeof(numberStr));
        index = 0;
        
        // Leer caracteres hasta encontrar '/'
        while (fread(&firstChar, sizeof(char), 1, file) == 1 && firstChar != '/') {
            if (index < sizeof(numberStr) - 1) {
                numberStr[index++] = firstChar;
            }
        }
        numberStr[index] = '\0';

        if (firstChar != '/') {
            printf("No se encontro el caracter '/' despues del numero.\n");
            fclose(file);
            return -1;
        }

        long numChars = atol(numberStr);

        // Reservar memoria y leer la cantidad de caracteres especificada
        char *data = malloc(numChars + 1);
        if (data == NULL) {
            printf("No se pudo reservar memoria.\n");
            fclose(file);
            return -1;
        }
        if (fread(data, sizeof(char), numChars, file) != numChars) {
            printf("No se pudo leer la cantidad esperada de caracteres.\n");
            free(data);
            fclose(file);
            return -1;
        }

        data[numChars] = '\0';
        
        int aborpt = createRawEntry(data);
        if(aborpt==1){
            printf("Fallo en el formato de entrada: \n");
            return 1;
        }
        free(data);
    }
    // Cierra el archivo
    fclose(file);
    return attachData(myFile);  // Éxito
}

int attachData(const char* filename) {
    FILE *file = fopen(filename, "rb");  // Usar "rb" para modo binario

    if (file == NULL) {
        // Si no se pudo abrir el archivo, imprimir un error.
        perror("Error al abrir el archivo");
        return -1;
    }

    // Saltar la primera línea
    char ch;
    while ((ch = fgetc(file)) != EOF && ch != '\n');

    // Leer la segunda línea
    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, file) == -1) {
        perror("Error al leer la segunda línea");
        fclose(file);
        return -1;
    }

    // Procesar la segunda línea
    char *ptr = line;
    while (*ptr != '\0') {
        if (*ptr == '|') {
            ptr++; // Avanzar el puntero

            // Leer el tamaño del path
            int path_length = 0;
            while (*ptr >= '0' && *ptr <= '9') {
                path_length = path_length * 10 + (*ptr - '0');
                ptr++;
            }

            if (*ptr != '/') {
                fprintf(stderr, "Formato inválido: se esperaba '/' después del tamaño del path\n");
                free(line);
                fclose(file);
                return -1;
            }
            ptr++; // Saltar el carácter '/'

            // Leer el path
            char *path = (char *)malloc(path_length + 1);
            strncpy(path, ptr, path_length);
            path[path_length] = '\0';
            ptr += path_length;

            // Leer el tamaño del binario
            unsigned long bin_size = 0;
            while (*ptr >= '0' && *ptr <= '9') {
                bin_size = bin_size * 10 + (*ptr - '0');
                ptr++;
            }
            bin_size--;

            // Leer el binario en base a la longitud dada anteriormente
            char *binario = (char *)malloc(bin_size + 1);
            if (fread(binario, 1, bin_size, file) != bin_size) {
                printf("No se pudo leer la cantidad esperada de caracteres binarios.\n");
                free(path);
                free(binario);
                free(line);
                fclose(file);
                return -1;
            }

            binario[bin_size] = '\0';

            if (insertarEnDatos(path, binario, bin_size) == 1) {
                free(path);
                free(binario);
                free(line);
                fclose(file);
                return 1;
            }

        } else {
            ptr++;
        }
    }

    // Liberar la memoria de la línea y cerrar el archivo
    free(line);
    fclose(file);

    return 0;
}

int insertarEnDatos(char* path, char* binario, unsigned long size){

    char* newPath = malloc(sizeof(char)*(strlen(path)+2));
    newPath[0] = '/';newPath[1] = '\0';
    strcat(newPath,path);
    
    free(path);

    elementoTabla* copiaElemento = pathExists(newPath);

    if(copiaElemento==NULL || copiaElemento -> data != NULL){
        perror("No se han podido guardar los datos.\n");
        free(newPath); free(binario);
        return 1;
    }

    free(newPath);

    copiaElemento -> data = malloc(sizeof(TFiles));
    copiaElemento -> data -> binario = binario;
    copiaElemento -> data -> size = size;

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

void fileSystemToBin(const char* newBin) {
    // Abre el archivo en modo escritura ("w"), lo cual borra el contenido si existe o crea uno nuevo si no existe.
    FILE *file = fopen(newBin, "w");

    if (file == NULL) {
        // Si no se pudo abrir el archivo, imprimir un error.
        perror("Error al abrir el archivo");
        return;
    }

    // Primera parte: escribir paths y signo de interrogación final
    elementoTabla* current = globalTable -> next;
    while (current != NULL) {
        // Calcula el tamaño del path
        int path_length = strlen(current->path);

        // Escribe el formato en el archivo: |tamaño/path|
        fprintf(file, "|%d%s", (path_length-1), current->path);

        // Avanza al siguiente nodo
        current = current->next;
    }

    // Escribe el carácter de finalización y un salto de línea
    fprintf(file, "?\n");

    // Segunda parte: escribir datos binarios formateados
    current = globalTable -> next;
    while (current != NULL) {
        if (current->data != NULL) {
            exportarABin("MIBINARIO.BIN",current->data->binario,current->data->size);
        }

        // Avanza al siguiente nodo
        current = current->next;
    }

    // Cierra el archivo
    fclose(file);
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