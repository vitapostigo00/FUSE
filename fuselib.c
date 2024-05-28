#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

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

