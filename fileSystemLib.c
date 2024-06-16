#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include "fuseHeaders.h"

FileSystemInfo* currentDir = NULL;

void initialize_filesystem() {
    printf("Initializing filesystem: Setting up root and clearing blocks.\n");
    fs[0].hasData = -1;
    fs[0].path[0]='\0';
    strcpy(fs[0].path, "/");
    fs[0].siguiente = -1;
    fs[0].creation_time = time(0);
    fs[0].last_access = time(0);
    fs[0].last_modification = time(0);
    fs[0].uid = getuid();
    fs[0].gid = getgid();
    fs[0].mode = S_IFDIR | 0755;
    fs[0].nlink = 1;

    for (int i = 1; i < FILESYSTEM_SIZE; i++) {
        fs[i].hasData = -1;
        strcpy(fs[i].path, "\0");
        fs[i].siguiente = -1;
        fs[i].creation_time = time(NULL);
        fs[i].last_access = time(NULL);
        fs[i].last_modification = time(NULL);
        fs[i].uid = getuid();
        fs[i].gid = getgid();
        fs[i].mode = 0644;
        fs[i].nlink = 0;
    }
    printf("Filesystem initialized: Root set and all blocks cleared.\n");
}

void init(const char *filename) {
    size_t filesize;
    struct stat st;

    printf("Opening filesystem storage file: %s\n", filename);
    int fileDescriptor = open(filename, O_RDWR | O_CREAT, 0666);
    if (fileDescriptor == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    printf("File opened successfully.\n");

    if (fstat(fileDescriptor, &st) == -1) {
        perror("Failed to stat file");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    }
    printf("File size obtained: %ld bytes.\n", st.st_size);

    filesize = FILESYSTEM_SIZE * sizeof(FileSystemInfo);
    printf("Expected filesystem size: %zu bytes.\n", filesize);

    if (st.st_size != filesize) {
        printf("Adjusting file size...\n");
        if (ftruncate(fileDescriptor, filesize) == -1) {
            perror("Failed to truncate file");
            close(fileDescriptor);
            exit(EXIT_FAILURE);
        }
        // Necesitas asegurarte de que fstat se actualice después de ftruncate
        if (fstat(fileDescriptor, &st) == -1 || st.st_size != filesize) {
            perror("Failed to verify file size after adjustment");
            close(fileDescriptor);
            exit(EXIT_FAILURE);
        }
        printf("File size adjusted to %ld bytes.\n", st.st_size);
    }

    printf("Mapping file to memory.\n");
    FileSystemInfo* fs = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (fs == MAP_FAILED) {
        perror("Failed to map file to memory");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    }
    printf("File mapped successfully.\n");

    if (st.st_size == 0) {
        printf("Initializing new filesystem structure.\n");
        initialize_filesystem(fs);
    }

    currentDir = fs;
    printf("Filesystem initialization complete.\n");

    close(fileDescriptor); // Buena práctica cerrar el descriptor de archivo una vez que ya no se necesita
}




void cleanup() {

    size_t filesize = FILESYSTEM_SIZE * sizeof(FileSystemInfo);

    if (msync(fs, filesize, MS_SYNC) == -1) {
        perror("msync");
        exit(EXIT_FAILURE);
    }

    if (munmap(fs, filesize) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    close(fileDescriptor);
}

void changeDirectory (const char* newDir){
    int directorioACambiar;

    char* fullPathString = buildFullPath(newDir);
    if(fullPathString==NULL){
        return;
    }

    directorioACambiar = exists(fullPathString);

    if(directorioACambiar==-1){
        printf("Path doesn't exist.\n");
        free(fullPathString);
        return;
    }

    fs[directorioACambiar].last_access = time(0);

    currentDir = &fs[directorioACambiar];

    free(fullPathString);
}

int createDir(const char* filename){
    int emptyBlock;
    int lastBlock;
	char* baseName = strchr(filename, '/') + 1; // Obtiene solo el nombre base del path

    if(strchr(baseName, '/') != NULL){
        printf ("Illegal character in the dir to build.\n");
        return -1;
    }

    char* fullPathString = buildFullPath(filename);
    if(fullPathString==NULL){
        return -1;
    }

    if(exists(fullPathString)!=-1){
        printf("Path already exists.\n");
        free(fullPathString);
        return -1;
    }

    emptyBlock = nextEmptyBlock(fs);
    if(emptyBlock==-1){
        printf("File system is full.\n");
        free(fullPathString);
        return -1;
    }

    lastBlock = lastUsedBlock(fs);
    assert(fs[lastBlock].siguiente == -1);

    fs[lastBlock].siguiente = emptyBlock;

    fs[emptyBlock].hasData = -1;
    strcpy(fs[emptyBlock].path, fullPathString);
    fs[emptyBlock].siguiente = -1;
    fs[emptyBlock].creation_time = time(0);
    fs[emptyBlock].last_access = time(0);
    fs[emptyBlock].last_modification = time(0);
    fs[emptyBlock].uid = getuid();
    fs[emptyBlock].gid = getgid();
    fs[emptyBlock].mode = S_IFDIR | 0755;
    fs[emptyBlock].nlink = 2;

    free(fullPathString);
    
    return 0;
}

void borrar(const char* absolutePath){
    int actual = 0;
    int posterior = fs[0].siguiente;

    while(posterior != -1 && strcmp(fs[posterior].path,absolutePath)!=0){
        actual = posterior;
        posterior = fs[posterior].siguiente;
    }

    if (posterior == -1) {
        printf("Path not found.\n");
        return;
    }

    fs[actual].siguiente = fs[posterior].siguiente;
    
    if(fs[posterior].hasData!=-1){
        // Limpiar fichero asociado a fs[posterior].hasData
    }

    memset(fs[posterior].path, 0, LONGEST_FILENAME);
    fs[posterior].siguiente = -1;
    fs[posterior].creation_time = time(NULL);
    fs[posterior].last_access = time(NULL);
    fs[posterior].last_modification = time(NULL);
    fs[posterior].uid = getuid();
    fs[posterior].gid = getgid();
    fs[posterior].mode = 0644;
    fs[posterior].nlink = 0;
}

void removeDir(const char* filename){
    char* fullPathString = buildFullPath(filename);
    
    if(fullPathString==NULL){
        return;
    }

    if(strcmp(filename,"/")==0){
        printf("Cannot remove root directory.\n");
        free(fullPathString);
        return;
    }

    int saveExist = exists(fullPathString);
    if(saveExist == -1){
        printf("Directory doesn't exist.\n");
        free(fullPathString);
        return;
    }

    for (int i = 0; i < FILESYSTEM_SIZE; i++) {
        if (strlen(fs[i].path) != 0 && isPrefix(fullPathString, fs[i].path)==0) {
            printf("Se va a borrar: %s\n",fs[i].path);
            borrar(fs[i].path);
        }
    }

    free(fullPathString);
    printf("Directory and its content has been removed successfully.\n");
}
