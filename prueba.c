#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fuseHeaders.h"

// Variable global para almacenar la tabla inicializada
elementoTabla *globalTable = NULL;

unsigned long getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    unsigned long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

int initFileSystem() {
    FILE *file = fopen("filesystem.bin", "rb");
    if (!file) {
        perror("Error al abrir el fichero binario");
        return -1;
    }

    unsigned long fileSize = getFileSize(file);
    printf("File size: %lu\n", fileSize);

    char *buffer = (char*)malloc(fileSize);
    if (!buffer) {
        perror("Error al asignar memoria para el buffer");
        fclose(file);
        return -1;
    }

    fread(buffer, 1, fileSize, file);
    fclose(file);

    elementoTabla *head = NULL;
    elementoTabla *current = NULL;

    char *ptr = buffer;
    while (ptr < buffer + fileSize) {
        // Verificar si es el final del archivo
        if (strncmp(ptr, "/0/0/0/0/0/0/0/0", 16) == 0) {
            break;
        }

        // Leer la longitud del path
        char *sep = strchr(ptr, '/');
        int pathLength = (int)(sep - ptr);
        char *path = (char*)malloc(pathLength + 1);
        strncpy(path, ptr, pathLength);
        path[pathLength] = '\0';
        ptr = sep + 1; // Mover el puntero después del '/'

        printf("Path length: %d\n", pathLength);
        printf("Path: %s\n", path);

        elementoTabla *newElement = (elementoTabla*)malloc(sizeof(elementoTabla));
        if (!newElement) {
            perror("Error al asignar memoria para el elemento de la tabla");
            free(path);
            free(buffer);
            return -1;
        }

        newElement->path = path;
        newElement->next = NULL;
        newElement->data = NULL;

        if (*ptr == '?') {
            ptr += 1; // Saltar el '?'

            // Leer la longitud del path del archivo
            sep = strchr(ptr, '/');
            int filePathLength = (int)(sep - ptr);
            ptr = sep + 1; // Mover el puntero después del '/'

            // Leer el tamaño de los datos
            sep = strchr(ptr, '/');
            unsigned long dataSize = strtoul(ptr, NULL, 10);
            ptr = sep + 1; // Mover el puntero después del '/'

            printf("Data size: %lu\n", dataSize);

            char *data = (char*)malloc(dataSize);
            if (!data) {
                perror("Error al asignar memoria para los datos");
                free(newElement);
                free(path);
                free(buffer);
                return -1;
            }

            memcpy(data, ptr, dataSize);
            ptr += dataSize;

            newElement->data = (TFiles*)malloc(sizeof(TFiles));
            if (!newElement->data) {
                perror("Error al asignar memoria para TFiles");
                free(data);
                free(newElement);
                free(path);
                free(buffer);
                return -1;
            }

            newElement->data->size = dataSize;
            newElement->data->data = data;
        }

        if (head == NULL) {
            head = newElement;
            current = head;
        } else {
            current->next = newElement;
            current = newElement;
        }

        if (*ptr == '|') {
            ptr += 1;
        }
    }

    free(buffer);

    globalTable = head;

    return 0;
}

void printFileSystem(elementoTabla *table) {
    elementoTabla *current = table;
    while (current != NULL) {
        printf("Path: %s\n", current->path);
        if (current->data != NULL) {
            printf("Size: %lu\n", current->data->size);
            printf("Data: %.*s\n", (int)current->data->size, current->data->data);
        }
        printf("\n");
        current = current->next;
    }
}

int main() {
    // Inicializa el sistema de archivos
    if (initFileSystem() == 0) {
        printf("Sistema de archivos inicializado correctamente.\n");

        // Imprime la tabla de elementos para verificar
        printFileSystem(globalTable);
    } else {
        printf("Error al inicializar el sistema de archivos.\n");
    }

    return 0;
}
