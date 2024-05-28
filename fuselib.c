#include "fuseHeaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

void showDate(struct tm time) {
    printf("Creation: %d-%02d-%02d %02d:%02d:%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
}

int initFileSystem() {
    // Inicializar variables globales
    clusterActualSize = 0;
    dataActualSize = 0;
    checkBeforeCat = 0;

    // Crear y configurar el directorio raíz
    rootElement = (clustElem*)malloc(sizeof(clustElem));
    if (rootElement == NULL) {
        perror("Error allocating memory for rootElement");
        return 1;
    }

    rootElement->fatherDir = NULL; // El directorio raíz no tiene padre
    strcpy(rootElement->filename, "/"); // El nombre del directorio raíz es "/"

    // Establecer la fecha de creación del directorio raíz
    time_t t = time(NULL);
    rootElement->fecha = *localtime(&t);

    // El directorio raíz inicialmente no tiene hijos
    rootElement->clusterPointer = NULL;

    // El directorio actual es el directorio raíz
    currentDir = rootElement;

    // Incrementar el tamaño actual del clúster ya que hemos creado el directorio raíz
    clusterActualSize++;

    return 0;
}


int mkdir(const char* newDir) {
    if (newDir[0] == '/') {
        printf("/ not allowed as first character.\n");
        return 1;
    } else if (clusterActualSize >= MAXCLUSTERSIZE) {
        printf("FAT16 SIZE EXCEEDED, STORAGE IS FULL.\n");
        return 1;
    }

    clustElem* newElement = (clustElem *)malloc(sizeof(clustElem));
    if (newElement == NULL) return 1;

    newElement->fatherDir = currentDir;

    newElement->filename[0] = '/';
    strcpy(newElement->filename + 1, newDir);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    newElement->fecha = tm;

    newElement->clusterPointer = NULL;

    sonElemList* controlP = (sonElemList*)currentDir->clusterPointer;

    sonElemList* newChild = (sonElemList*)malloc(sizeof(sonElemList));
    if (newChild == NULL) return 1;
    newChild->next = NULL;
    newChild->elemento = newElement;

    if (controlP == NULL) {
        currentDir->clusterPointer = newChild;
    } else {
        while (controlP->next != NULL) {
            controlP = controlP->next;
        }
        controlP->next = newChild;
    }

    clusterActualSize++;
    return 0;
}


void ls() {
    printf("Directorio actual: %s\n", currentDir->filename);

    sonElemList* aux = (sonElemList*)currentDir->clusterPointer;

    while (aux != NULL) {
        printf("%s     ", aux->elemento->filename);
        fflush(stdout);
        aux = aux->next;
    }
    printf("\n");
    fflush(stdout);
    return;
}

void cd(const char* dir) {
    if (strcmp(dir, "..") == 0) {
        if (currentDir == rootElement) {
            printf("You are already in root.\n");
        } else {
            currentDir = currentDir->fatherDir;
        }
        return;
    }

    char filename[LONGESTFILENAME];
    filename[0] = '/';
    strcpy(filename + 1, dir);

    sonElemList* aux = (sonElemList*)currentDir->clusterPointer;

    while (aux != NULL && strcmp(filename, aux->elemento->filename) != 0) {
        aux = aux->next;
    }

    if (aux == NULL) {
        printf("Directory not found.\n");
    } else {
        currentDir = aux->elemento;
    }
}


int mkf(const char* dir, const char* content) {
    if (dir[0] == '/') {
        printf("/ not allowed as first character.");
        return 1;
    } else if (clusterActualSize >= MAXCLUSTERSIZE) {
        printf("FAT16 SIZE EXCEEDEED, CLUSTER STORAGE IS FULL.");
        return 1;
    }

    unsigned int dataToFill = strlen(content) / BYTESPERCLUSTER + 1;
    if (dataToFill + dataActualSize >= MAXCLUSTERSIZE) {
        printf("FAT16 SIZE EXCEEDEED, DATA STORAGE IS FULL.");
        return 1;
    }

    clustElem* newElement = (clustElem*)malloc(sizeof(clustElem));
    if (newElement == NULL) return 1;

    newElement->fatherDir = currentDir;
    strcpy(newElement->filename, dir);

    // Actual time
    time_t t = time(NULL);
    newElement->fecha = *localtime(&t);

    // Initialize the data structure for the new file
    newElement->clusterPointer = malloc(sizeof(myData));
    if (newElement->clusterPointer == NULL) {
        free(newElement);
        return 1;
    }
    myData* dataCopy = (myData*)newElement->clusterPointer;
    dataCopy->next = dataCopy; // Initially point to itself
    strncpy(dataCopy->infoFichero, content, BYTESPERCLUSTER);

    // Add the new file to the list of children of the current directory
    sonElemList* controlP = (sonElemList*)currentDir->clusterPointer;
    if (controlP == NULL) {
        controlP = (sonElemList*)malloc(sizeof(sonElemList));
        if (controlP == NULL) {
            free(newElement->clusterPointer);
            free(newElement);
            return 1;
        }
        controlP->elemento = newElement;
        controlP->next = NULL;
        currentDir->clusterPointer = controlP;
    } else {
        while (controlP->next != NULL) {
            controlP = controlP->next;
        }
        controlP->next = (sonElemList*)malloc(sizeof(sonElemList));
        if (controlP->next == NULL) {
            free(newElement->clusterPointer);
            free(newElement);
            return 1;
        }
        controlP->next->elemento = newElement;
        controlP->next->next = NULL;
    }

    clusterActualSize++;
    dataActualSize += dataToFill;

    return 0;
}



int rmf(const char* filename) {
    if (filename[0] == '/') {
        printf("/ not allowed as first character.\n");
        return 1;
    }

    // Buscar el archivo a eliminar
    sonElemList* prev = NULL;
    sonElemList* current = (sonElemList*)currentDir->clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename, filename) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("File not found.\n");
        return 1;
    }

    // Verificar si el elemento a eliminar es un archivo
    if (current->elemento->clusterPointer == NULL) {
        printf("Not a file.\n");
        return 1;
    }

    // Liberar la memoria asociada con el archivo
    myData* data = (myData*)(current->elemento->clusterPointer);
    myData* currentData = data;

    do {
        myData* nextData = currentData->next;
        free(currentData);
        currentData = nextData;
    } while (currentData != data);

    // Eliminar el archivo de la lista de hijos del directorio actual
    if (prev == NULL) {
        currentDir->clusterPointer = current->next;
    } else {
        prev->next = current->next;
    }

    // Liberar la memoria del elemento eliminado
    free(current->elemento);
    free(current);

    // Decrementar el contador de clústeres
    clusterActualSize--;

    return 0;
}


int rmdir(const char* dirName) {
    if (dirName[0] != '/') {
        printf("Invalid directory name.\n");
        return 1;
    }

    // Buscar el directorio a eliminar
    sonElemList* prev = NULL;
    sonElemList* current = (sonElemList*)currentDir->clusterPointer;

    while (current != NULL && strcmp(current->elemento->filename, dirName) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Directory not found.\n");
        return 1;
    }

    // Liberar la memoria de los elementos hijo del directorio
    sonElemList* child = (sonElemList*)current->elemento->clusterPointer;
    while (child != NULL) {
        sonElemList* nextChild = child->next;
        free(child->elemento);
        free(child);
        child = nextChild;
    }

    // Eliminar el directorio de la lista de hijos del directorio padre
    if (prev == NULL) {
        currentDir->clusterPointer = current->next;
    } else {
        prev->next = current->next;
    }

    // Liberar la memoria del directorio eliminado
    free(current->elemento);
    free(current);

    // Decrementar el contador de clústeres
    clusterActualSize--;

    return 0;
}


int renameDir(const char* oldName, const char* newName) {
    if (oldName[0] == '/' || newName[0] == '/') {
        printf("/ not allowed as first character.");
        return 1;
    }

    char oldFilename[LONGESTFILENAME];
    char newFilename[LONGESTFILENAME];
    strcpy(oldFilename, oldName);
    strcpy(newFilename, newName);

    sonElemList* aux = (sonElemList*)currentDir->clusterPointer;

    while (aux != NULL && strcmp(oldFilename, aux->elemento->filename) != 0) {
        aux = aux->next;
    }

    if (aux == NULL) {
        printf("Directory not found.");
        return 1;
    }

    strcpy(aux->elemento->filename, newFilename);
    return 0;
}

void freeClusterElement(clustElem* element) {
    if (element == NULL) return;

    // Free the children elements if it's a directory
    if (element->filename[0] == '/') {
        sonElemList* children = (sonElemList*)element->clusterPointer;
        while (children != NULL) {
            sonElemList* nextChild = children->next;
            freeClusterElement(children->elemento);
            free(children);
            children = nextChild;
        }
    } else {
        // Free the data if it's a file
        myData* data = (myData*)element->clusterPointer;
        if (data != NULL) {
            myData* start = data;
            do {
                myData* nextData = data->next;
                free(data);
                data = nextData;
            } while (data != start && data != NULL);
        }
    }

    free(element);
}

int cleanFileSystem() {
    if (rootElement == NULL) return 0;

    freeClusterElement(rootElement);
    rootElement = NULL;
    currentDir = NULL;

    return 0;
}

char* cat(const char* filename) {
    if (filename[0] == '/') {
        return "Folders can't retrieve any text, only files can.";
    }

    sonElemList* aux = (sonElemList*)currentDir->clusterPointer;

    while (aux != NULL && strcmp(filename, aux->elemento->filename) != 0) {
        aux = aux->next;
    }

    if (aux == NULL) {
        return "Archivo no encontrado.";
    }

    myData* textoCat = (myData*)aux->elemento->clusterPointer;
    unsigned int longitud = 0;

    // Calculate the total length of the content
    myData* temp = textoCat;
    do {
        longitud += strlen(temp->infoFichero);
        temp = temp->next;
    } while (temp != textoCat);

    char* buffer = (char*)malloc(longitud + 1);
    if (buffer == NULL) {
        return "No hay espacio para el buffer de texto";
    }
    buffer[0] = '\0';

    // Copy the content into the buffer
    temp = textoCat;
    do {
        strcat(buffer, temp->infoFichero);
        temp = temp->next;
    } while (temp != textoCat);

    checkBeforeCat = 1;
    return buffer;
}



void remove_allocated_chars(char* str) {
    free(str);
}

int main(int argc, char *argv[]) {
    if (initFileSystem() == 1) {
        return 1;
    }

    mkdir("Folder 1"); 
    mkdir("Folder 2");
    
    ls();
    
    cd("Folder 1");

    mkf("arxivo", "holasoyvitaestoyprobandolosarchivoscontamagno4");

    ls();

    char* toPrintCat = cat("arxivo");
    printf("Contenido de arxivo: %s\n", toPrintCat);
    if (checkBeforeCat) {
        free(toPrintCat);
        checkBeforeCat = 0;
    }
	
	rmf("arxivo");
	
	ls();
	
    cd("..");
    
    ls();
    
    rmdir("Folder 1");
    
    ls();

    if (cleanFileSystem() == 1) {
        return 1;
    }

    printf("FileSystem closing propperly.\n");
    return 0;
}

