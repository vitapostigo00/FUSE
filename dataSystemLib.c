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



void initialize_datasystem() {
    for (int i = 0; i < DATASYSTEM_SIZE; i++) {
        ds[i].firstDataBlock = -1;
        ds[i].currentBlockSize = -1;
        ds[i].totalSize = -1;
        memset(ds[i].data, 0, BLOCKSIZE-1);
        ds[i].siguiente = -1;
    }
    printf("DataSystem initialized.\n");
}


void init(const char *filename, FileSystemInfo **fs, size_t *filesize, int *fd, struct stat *st) {
    printf("Opening filesystem storage file: %s\n", filename);
    *fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (*fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    printf("File opened successfully.\n");

    if (fstat(*fd, st) == -1) {
        perror("Failed to stat file");
        exit(EXIT_FAILURE);
    }
    printf("File size obtained: %ld bytes.\n", st->st_size);

    *filesize = FILESYSTEM_SIZE * sizeof(FileSystemInfo);
    printf("Expected filesystem size: %zu bytes.\n", *filesize);
    if (st->st_size != *filesize) {
        printf("Adjusting file size...\n");
        if (ftruncate(*fd, *filesize) == -1) {
            perror("Failed to truncate file");
            exit(EXIT_FAILURE);
        }
        printf("File size adjusted.\n");
    }

    printf("Mapping file to memory.\n");
    *fs = mmap(NULL, *filesize, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if (*fs == MAP_FAILED) {
        perror("Failed to map file to memory");
        exit(EXIT_FAILURE);
    }
    printf("File mapped successfully.\n");

    if (st->st_size == 0) {
        printf("Initializing new filesystem structure.\n");
        initialize_filesystem(*fs);
    }

    currentDir = *fs;
    printf("Filesystem initialization complete.\n");
}