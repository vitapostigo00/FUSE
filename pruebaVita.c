#include <stdio.h>
#include <stdlib.h>

// Función para leer un archivo y guardar su contenido en un buffer
char* leerArchivo(const char* nombreArchivo, size_t* tamanio) {
    FILE* archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    fseek(archivo, 0, SEEK_END);
    *tamanio = ftell(archivo);
    rewind(archivo);

    char* buffer = (char*)malloc(*tamanio + 1);
    if (buffer == NULL) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return NULL;
    }

    fread(buffer, 1, *tamanio, archivo);
    buffer[*tamanio] = '\0';

    fclose(archivo);
    return buffer;
}

// Función para exportar el contenido del buffer a un archivo .bin
void exportarABin(const char* nombreArchivo, const char* buffer, size_t tamanio) {
    FILE* archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        perror("Error al crear el archivo");
        return;
    }

    fwrite(buffer, 1, tamanio, archivo);
    fclose(archivo);
}

// Función para leer un archivo .bin y crear un nuevo archivo con el contenido
void leerBinYCrearArchivo(const char* archivoBin, const char* nuevoNombre) {
    FILE* archivo = fopen(archivoBin, "rb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo binario");
        return;
    }

    fseek(archivo, 0, SEEK_END);
    size_t tamanio = ftell(archivo);
    rewind(archivo);

    char* buffer = (char*)malloc(tamanio + 1);
    if (buffer == NULL) {
        perror("Error al asignar memoria");
        fclose(archivo);
        return;
    }

    fread(buffer, 1, tamanio, archivo);
    buffer[tamanio] = '\0';
    fclose(archivo);

    FILE* nuevoArchivo = fopen(nuevoNombre, "wb");
    if (nuevoArchivo == NULL) {
        perror("Error al crear el nuevo archivo");
        free(buffer);
        return;
    }

    fwrite(buffer, 1, tamanio, nuevoArchivo);
    fclose(nuevoArchivo);
    free(buffer);
}

int main() {
    const char* nombreArchivo = "arcade.mp4";
    const char* nombreBinario = "trukaso.bin";
    const char* nombreNuevoArchivo = "arcadegod.mp4";

    size_t tamanio;
    char* buffer = leerArchivo(nombreArchivo, &tamanio);
    if (buffer == NULL) {
        return 1;
    }

    exportarABin(nombreBinario, buffer, tamanio);
    free(buffer);

    leerBinYCrearArchivo(nombreBinario, nombreNuevoArchivo);

    return 0;
}