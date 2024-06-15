#include "fuseHeaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "fileSystemLib.c"
#include "fileSystemUtils.c"
#include "fuseHeaders.h"

extern dataFile* mydata;
extern FileSystemInfo* currentDir;  // Si está definida en otro archivo
int fd;

void init_data_system() {
    size_t filesize;
    int fd;  // Declaración local del file descriptor
    struct stat st;  // Declaración de la variable para almacenar el estado del archivo

    fd = open("datasystem.bin", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Failed to open datasystem.bin");
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &st) == -1) {
        perror("Failed to get file status");
        close(fd);  // Asegúrate de cerrar el descriptor de archivo en caso de error
        exit(EXIT_FAILURE);
    }

    filesize = FILESYSTEM_SIZE * sizeof(FileSystemInfo);
    if (st.st_size != filesize) {
        if (ftruncate(fd, filesize) == -1) {
            perror("Failed to resize file");
            close(fd);  // Asegúrate de cerrar el descriptor de archivo en caso de error
            exit(EXIT_FAILURE);
        }
    }

    FileSystemInfo* fileSystem = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fileSystem == MAP_FAILED) {
        perror("Failed to map file system");
        close(fd);  // Asegúrate de cerrar el descriptor de archivo en caso de error
        exit(EXIT_FAILURE);
    }

    // Aquí podrías llamar a cualquier función de inicialización necesaria
    close(fd);  // Cierra el descriptor de archivo después de usarlo
}

int findFileSystemIndex(const char* path) {
    return exists(currentDir, path);
}

int createFile(FileSystemInfo *fs, char *filename) {
    int emptyBlockIndex;
    int lastBlock;

    if (strchr(filename, '/') != NULL) {
        printf("Illegal character in the filename.\n");
        return -1;
    }

    char* fullPathString = buildFullPathGeneral(filename);
    if (fullPathString == NULL) {
        return -1;
    }

    if (exists(fs, fullPathString) != -1) {
        printf("File already exists.\n");
        free(fullPathString);
        return -1;
    }

    emptyBlockIndex = nextEmptyBlock(fs);
    if (emptyBlockIndex == -1) {
        printf("File system is full.\n");
        free(fullPathString);
        return -1;
    }

    lastBlock = lastUsedBlock(fs);
    assert(fs[lastBlock].siguiente == -1);

    fs[lastBlock].siguiente = emptyBlockIndex;

    fs[emptyBlockIndex].hasData = emptyBlockIndex; // Suponiendo que los datos del archivo serán manejados por índice
    strcpy(fs[emptyBlockIndex].path, fullPathString);
    fs[emptyBlockIndex].siguiente = -1;
    fs[emptyBlockIndex].creation_time = time(0);
    fs[emptyBlockIndex].last_access = time(0);
    fs[emptyBlockIndex].last_modification = time(0);
    fs[emptyBlockIndex].uid = getuid();
    fs[emptyBlockIndex].gid = getgid();
    fs[emptyBlockIndex].mode = S_IFREG | 0644;
    fs[emptyBlockIndex].nlink = 1;

    free(fullPathString);

    return 0;
}



int rmfile(FileSystemInfo* currentDir, const char* path) {
    // Verifica que el archivo exista
    int fileIndex = exists(currentDir, path);
    if (fileIndex == -1) {
        printf("Error: El archivo no existe.\n");
        return -1;  // El archivo no existe
    }

    // Verificar que no es un directorio
    if (currentDir[fileIndex].hasData == -1) {
        printf("Error: El path especificado es un directorio.\n");
        return -1;  // Intento de eliminar un directorio usando rmfile
    }

    // Llamar a la función borrar para eliminar el archivo
    borrar(currentDir, currentDir[fileIndex].path);
    printf("Archivo eliminado con éxito.\n");
    return 0;  // Éxito
}


/*
void cleanup_data_system() {
    closeFileSystem(fileSystem, fd, NULL);
}
*/


int main() {
 init_data_system();  // Inicializar el sistema de archivos

    // Crear un directorio usando la función de la biblioteca
    if (createDir(currentDir, "/myDirectory") == 0) {
        printf("Directorio creado con éxito.\n");
    } else {
        printf("Error al crear el directorio.\n");
    }

    // Crear un fichero dentro de ese directorio
    char* filePath = "/myDirectory/myFile.txt";
    if (createFile(currentDir, filePath) == 0) {
        printf("Fichero creado con éxito dentro del directorio.\n");
    } else {
        printf("Error al crear el fichero dentro del directorio.\n");
    }

    // Prueba de borrado de fichero
    if (rmfile(currentDir, filePath) == 0) {
        printf("Fichero borrado con éxito.\n");
    } else {
        printf("Error al borrar el fichero.\n");
    }

    // Limpiar y cerrar el sistema
    //cleanup_data_system();

    return 0;
}
