#define FUSE_USE_VERSION 26

#include "fuseHeaders.h"
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

// Inicializa el sistema de archivos
int initFileSystem() {
    clusterActualSize = 0;

    rootElement = (clustElem *) malloc(sizeof(clustElem));
    if (rootElement == NULL) return 1;

    rootElement->fatherDir = NULL;
    strcpy(rootElement->filename, "/"); // Root

    // Actual time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    rootElement->fecha = tm;
    // Actual time

    rootElement->clusterPointer = NULL;

    clusterActualSize++;
    currentDir = rootElement; // Root es el directorio actual.

    return 0;
}

// Crear un nuevo directorio
int my_mkdir_internal(const char newDir[LONGESTFILENAME-1]) {
    // Verifica si es relativa o absoluta la dirección
    if (newDir[0] == '/') {
        printf("/ not allowed as first character.");
        return 1;
    } else if (clusterActualSize >= MAXCLUSTERSIZE) {
        printf("FAT16 SIZE EXCEEDED, STORAGE IS FULL.");
        return 1;
    }

    clustElem* newElement = (clustElem *) malloc(sizeof(clustElem));
    if (newElement == NULL) return 1;

    newElement->fatherDir = currentDir;

    // Copiamos el nombre a partir del primer elemento
    newElement->filename[0] = '/';
    strcpy(newElement->filename + 1, newDir);

    // Actual time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    newElement->fecha = tm;
    // Actual time

    newElement->clusterPointer = NULL;

    // Añadimos a la lista de directorios hijos del nodo padre
    sonElemList* newChild = (sonElemList*) malloc(sizeof(sonElemList));
    newChild->elemento = newElement;
    newChild->next = (sonElemList*) currentDir->clusterPointer;
    currentDir->clusterPointer = newChild;

    clusterActualSize++;
    return 0;
}

// Lista el contenido del directorio actual
void ls() {
    sonElemList* child = (sonElemList*) currentDir->clusterPointer;
    while (child != NULL) {
        printf("%s\n", child->elemento->filename);
        child = child->next;
    }
}

// Cambia el directorio actual
int cd(const char newDir[LONGESTFILENAME-1]) {
    if (strcmp(newDir, "..") == 0) {
        if (currentDir->fatherDir != NULL) {
            currentDir = currentDir->fatherDir;
        }
        return 0;
    }

    sonElemList* child = (sonElemList*) currentDir->clusterPointer;
    while (child != NULL) {
        if (strcmp(child->elemento->filename + 1, newDir) == 0) {
            currentDir = child->elemento;
            return 0;
        }
        child = child->next;
    }
    printf("Directory no se encuentra.\n");
    return 1;
}

// Limpia el sistema de archivos
int cleanFileSystem() {
    // Implementar limpieza de la memoria asignada
    return 0;
}

// Funciones auxiliares para encontrar elementos
clustElem* find_element(const char *path) {
    if (strcmp(path, "/") == 0) return rootElement;

    clustElem *dir = currentDir;
    sonElemList *child = (sonElemList *)dir->clusterPointer;
    while (child != NULL) {
        if (strcmp(child->elemento->filename, path) == 0) {
            return child->elemento;
        }
        child = child->next;
    }
    return NULL;
}

static int my_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    clustElem *element = find_element(path);
    if (!element) return -ENOENT;

    if (element->filename[0] == '/') {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = BYTESPERCLUSTER;
    }
    stbuf->st_mtime = stbuf->st_ctime = stbuf->st_atime = mktime(&element->fecha);

    return 0;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    clustElem *dir = find_element(path);
    if (!dir) return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    sonElemList *child = (sonElemList *)dir -> clusterPointer;
    while (child != NULL) {
        filler(buf, child->elemento->filename + 1, NULL, 0);
        child = child->next;
    }
    return 0;
}

static int my_mkdir(const char *path, mode_t mode) {
    (void) mode;
    char newDir[LONGESTFILENAME-1];
    strcpy(newDir, path + 1); // Eliminamos el primer '/'
    return my_mkdir_internal(newDir);
}

static int my_open(const char *path, struct fuse_file_info *fi) {
    clustElem *element = find_element(path);
    if (!element) return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY) return -EACCES;
    return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    clustElem *element = find_element(path);
    if (!element || element->filename[0] == '/') return -ENOENT;

    myData *data = (myData *)element->clusterPointer;
    if (!data) return -ENOENT;

    size_t len = strlen(data->infoFichero);
    if (offset < len) {
        if (offset + size > len) size = len - offset;
        memcpy(buf, data->infoFichero + offset, size);
    } else size = 0;
    return size;
}

static struct fuse_operations fuse_example_operations = {
    .getattr = my_getattr,
    .readdir = my_readdir,
    .mkdir = my_mkdir,
    .open = my_open,
    .read = my_read,
};

int main(int argc, char *argv[]) {
    if (initFileSystem() == 1) {
        return 1;
    }
    return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
