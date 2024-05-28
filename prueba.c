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
        int pathLength = *(int*)ptr;
        ptr += sizeof(int);

        char *path = (char*)malloc(pathLength + 1);
        strncpy(path, ptr, pathLength);
        path[pathLength] = '\0';
        ptr += pathLength;

        elementoTabla *newElement = (elementoTabla*)malloc(sizeof(elementoTabla));
        newElement->path = path;
        newElement->next = NULL;

        if (ptr < buffer + fileSize) {
            unsigned long dataSize = *(unsigned long*)ptr;
            ptr += sizeof(unsigned long);

            char *data = (char*)malloc(dataSize);
            memcpy(data, ptr, dataSize);
            ptr += dataSize;

            newElement->data = (TFiles*)malloc(sizeof(TFiles));
            newElement->data->size = dataSize;
            newElement->data->data = data;
        } else {
            newElement->data = NULL;
        }

        if (head == NULL) {
            head = newElement;
            current = head;
        } else {
            current->next = newElement;
            current = newElement;
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

void createFileSystemBin() {
    FILE *file = fopen("filesystem.bin", "wb");
    if (!file) {
        perror("Error al crear el fichero binario");
        exit(-1);
    }

    // Primera entrada
    int pathLength1 = strlen("/file1.txt");
    printf("Long: %i\n",pathLength1);
    char path1[] = "/file1.txt";
    unsigned long dataSize1 = strlen("Hello, World!");
    char data1[] = "Hello, World!";

    fwrite(&pathLength1, sizeof(int), 1, file);
    fwrite(path1, sizeof(char), pathLength1, file);
    fwrite(&dataSize1, sizeof(unsigned long), 1, file);
    fwrite(data1, sizeof(char), dataSize1, file);

    // Segunda entrada
    int pathLength2 = strlen("/file2.txt");
    char path2[] = "/file2.txt";
    unsigned long dataSize2 = strlen("Goodbye!");
    char data2[] = "Goodbye!";

    fwrite(&pathLength2, sizeof(int), 1, file);
    fwrite(path2, sizeof(char), pathLength2, file);
    fwrite(&dataSize2, sizeof(unsigned long), 1, file);
    fwrite(data2, sizeof(char), dataSize2, file);

    fclose(file);
    printf("filesystem.bin creado exitosamente.\n");
}

int main() {
    // Crear el archivo filesystem.bin
    createFileSystemBin();

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
