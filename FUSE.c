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


// Función para obtener atributos de un archivo o directorio
static int fs_getattr(const char *path, struct stat *stbuf) {
    printf("fs_getattr: Path = %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));
	int idx= exists(path);
	printf("Index: %i\n", idx);
	
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = fs[0].mode;
        stbuf->st_nlink = fs[0].nlink + 2;
        stbuf->st_atime = fs[0].last_access;
        stbuf->st_mtime = fs[0].last_modification;
        stbuf->st_ctime = fs[0].creation_time;
		stbuf->st_uid= fs[0].uid;
		stbuf->st_gid= fs[0].gid;
    } else if((idx != -1) && (fs[idx].hasData == -1)){
		stbuf->st_mode = fs[idx].mode;
        stbuf->st_nlink = fs[idx].nlink + 2;
        stbuf->st_atime = fs[idx].last_access;
        stbuf->st_mtime = fs[idx].last_modification;
        stbuf->st_ctime = fs[idx].creation_time;
		stbuf->st_uid= fs[idx].uid;
		stbuf->st_gid= fs[idx].gid;
	} else if ((idx != -1) && (fs[idx].hasData != -1)){
		stbuf->st_mode = fs[idx].mode;
        stbuf->st_nlink = 1;
        stbuf->st_atime = fs[idx].last_access;
        stbuf->st_mtime = fs[idx].last_modification;
        stbuf->st_ctime = fs[idx].creation_time;
		stbuf->st_uid= fs[idx].uid;
		stbuf->st_gid= fs[idx].gid;
	} else {
		return -ENOENT;
	} 

    return 0;
}

// Función para leer un directorio
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
	(void) fi;
	int idx= exists(path);
	
	if(idx==-1){
		printf("No se encuentra");
		return -ENOENT;
	}
	
	if (filler(buf, ".", NULL, 0) != 0) {
		return -ENOMEM;
	}
	
	if(filler(buf, "..", NULL, 0) != 0){
		return -ENOMEM;
	}
	
    if (strcmp(path, "/") == 0) {
        // Añadir todas las entradas
        for (int i = 1; i < FILESYSTEM_SIZE; i++) {
            if (subdir_inmediato(path, fs[i].path)==0) { // Revisa si la entrada es válida
				char resultado[LONGEST_FILENAME];
				ultimoElemento(fs[i].path, resultado);
                if (filler(buf, resultado, NULL, 0) != 0) {
                    return -ENOMEM;
                }
            }
        }
    } else if(fs[idx].hasData == -1){
        for (int i=0; i < FILESYSTEM_SIZE; i++){
            if (subdir_inmediato(path, fs[i].path)==0) { // Revisa si la entrada es válida
				char resultado[LONGEST_FILENAME];
				ultimoElemento(fs[i].path, resultado);
                if (filler(buf, resultado, NULL, 0) != 0) {
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
