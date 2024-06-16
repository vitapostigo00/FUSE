#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fuseHeaders.h"

// Prototipos de funciones
static int fs_getattr(const char *path, struct stat *stbuf);
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int fs_mkdir(const char *path, mode_t mode);
static int fs_rmdir(const char *path);
static void fs_destroy(void *userdata);

// Estructura de operaciones FUSE
static struct fuse_operations fs_oper = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .mkdir = fs_mkdir,
    .rmdir = fs_rmdir,
    .destroy = fs_destroy,
};

static const char *fileSystemData = "filesystem.bin";
FileSystemInfo* fs = NULL;
int fileDescriptor = 0;


// Función para obtener atributos de un archivo o directorio
static int fs_getattr(const char *path, struct stat *stbuf) {
    printf("fs_getattr: Path = %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));
	
	printf("Dir to show: %s\n", fs[1].path);
	fflush(stdout);
	
	if(fs==NULL){
		printf("Albacete");
		fflush(stdout);
	}
	
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    int index = exists(path);
    if (index == -1) {
        return -ENOENT;
    }

    return 0;
}

// Función para leer un directorio
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("fs_readdir: Path = %s\n", path);
	if(fs==NULL){
		printf("Albacete");
		fflush(stdout);
	}
	
    if (strcmp(path, "/") == 0) {
        if (filler(buf, ".", NULL, 0) != 0 || filler(buf, "..", NULL, 0) != 0) {
            return -ENOMEM;
        }
        // Añadir todas las entradas
        for (int i = 0; i < FILESYSTEM_SIZE; i++) {
            if (fs[i].siguiente != -1) { // Revisa si la entrada es válida
                const char *name = fs[i].path + 1; // Omitir el primer '/' para las entradas
                if (filler(buf, name, NULL, 0) != 0) {
                    return -ENOMEM;
                }
            }
        }
    }

    return 0;
}

// Función para crear un directorio
static int fs_mkdir(const char *path, mode_t mode) {
    printf("fs_mkdir: Path = %s\n", path);

    if (exists(path) != -1) {
        printf("fs_mkdir: Directory already exists.\n");
        return -EEXIST;
    }

    // Asignar valores apropiados a un nuevo directorio
    int result = createDir(path);
    if (result == -1) {
        printf("fs_mkdir: Failed to create directory.\n");
        return -EPERM;
    }

    return 0;
}


// Función para eliminar un directorio
static int fs_rmdir(const char *path) {
    printf("fs_rmdir: Path = %s\n", path);

    if (exists(path) == -1) {
        return -ENOENT;
    }

    for (int i = 0; i < FILESYSTEM_SIZE; i++) {
        if (strlen(fs[i].path) != 0 && isPrefix(path, fs[i].path) == 0) {
            return -ENOTEMPTY;
        }
    }

    removeDir(path);
    return 0;
}

// Función para destruir el sistema de archivos
static void fs_destroy(void *userdata) {
    printf("fs_destroy: Destroying file system\n");
    FileSystemInfo *fs = (FileSystemInfo*)userdata;
    printFileSystemState("salida");
    cleanup(fs, FILESYSTEM_SIZE * sizeof(FileSystemInfo), fileno(fopen(fileSystemData, "r")));
    printf("fs_destroy: File system destroyed\n");
}

// Función principal
int main(int argc, char *argv[])
{
    // Verificar el número mínimo de argumentos y que los últimos dos no sean opciones (comienzan con '-')
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-')) {
        fprintf(stderr, "Uso incorrecto de los parámetros. Debe ser: ./programa <archivo_datos> <punto_montaje>\n");
        return 1; // Termina el programa si los parámetros no son correctos
    }

	init(argv[argc-2]);
    
    // Ajustar los argumentos para FUSE
    argv[argc-2] = argv[argc-1]; // Mueve el punto de montaje al lugar del fichero
    argv[argc-1] = NULL;         // Elimina el último argumento para ajustar a FUSE
    argc--;

    // Iniciar FUSE
    return fuse_main(argc, argv, &fs_oper, fs);
}
