#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "fuseHeaders.h"

extern elementoTabla* globalTable;
extern char* currentPath;
extern char* FUSEINITFILES;


void copiarDesdeArchivo(const char* filename, char* newFile){
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Lee los datos del archivo
    char *buffer = (char *) malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error en la asignación de memoria para buffer.\n");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);

    // Inserta los datos en la lista
    guardarDatos(newFile, buffer, file_size);

    free(buffer);
    
}

int devolverArchivo(char* nuevoArchivo,char* archivoEnFUSe){
    char* nombreAbuscar = malloc(sizeof(char)*(strlen(currentPath)+strlen(archivoEnFUSe)+1));
    strcpy(nombreAbuscar,currentPath);
    strcat(nombreAbuscar,archivoEnFUSe);

    elementoTabla* copia = pathExists(nombreAbuscar);

    if (copia == NULL){
        printf("El elemento a devolver no se ha encontrado!\n");
        free(nombreAbuscar);
        return 1;
    }

    FILE *file = fopen(nuevoArchivo, "wb");  // Abre el archivo en modo escritura binaria
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo para escribir.\n");
        return 1;
    }

    fwrite(copia->data->binario, 1, copia->data->size, file);  // Escribe los datos binarios en el archivo

    fclose(file);  // Cierra el archivo
    return 0;
}