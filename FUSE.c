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
static int fs_unlink(const char *path);
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int fs_open(const char *path, struct fuse_file_info *fi);
static int fs_release(const char *path, struct fuse_file_info *fi);
static int fs_getxattr(const char *path, const char *name, char *value, size_t size);


// Estructura de operaciones FUSE
static struct fuse_operations fs_oper = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .mkdir   = fs_mkdir,
    .rmdir   = fs_rmdir,
    .destroy = fs_destroy,
    .create  = fs_create,
    .unlink  = fs_unlink,
    .read    = fs_read,
    .write   = fs_write,
    .open    = fs_open,
    .release = fs_release,
    .getxattr= fs_getxattr;
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
    deleteElement(path);
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

int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    int result = createFile(path);
    if (result == -1) {
        printf("fs_create: Failed to create file.\n");
        return -EPERM;
    }
    return 0;
}

static int fs_unlink(const char *path){
    printf("fs_unlink: Path = %s\n", path);
    deleteElement(path);
    return 0;
}

int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("fs_unlink: Path = %s\n", path);
	int idx= exists(path);
	if(idx==-1){
		printf("fs_read: File not found.\n");
        return -ENOENT;	
	}
	
	if(fs[idx].hasData==-1){
		printf("fs_read: Not a file.\n");
        return -EISDIR;
	}
	
	if(ds[fs[idx].hasData].firstDataBlock != fs[idx].hasData){
		return -EIO;
	}
    size_t tam = sizeOfFile(fs[idx].hasData);
    if(offset < tam){
		if((offset + size) > tam){
			size = tam - offset;
		}
		char* temp= cat(fs[idx].hasData);
		memcpy(buf,temp + offset, size);
		free(temp);
	} else{
		size=0;
	}
    
    return size;
}

int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

    //Hay que mirar qué se hace con fi...
	printf("fs_unlink: Path = %s\n", path);
	int idx = exists(path);
	if(idx==-1){
		printf("fs_write: File not found.\n");
        return -ENOENT;	
	}
    
    if(fs[idx].hasData==-1){
		printf("fs_write: Not a file.\n");
        return -EISDIR;
	}

    //Llegados aqui, suponemos que tenemos un fichero con datos.
    char* actualData = cat(fs[idx].hasData);

    if(borrarFile(fs[idx].hasData)==-1){
        free(actualData);
        return -EIO;
    }

    fs[idx].last_access = time(0);          
    fs[idx].last_modification = time(0);

    if(offset>=strlen(actualData)){//Si el offset pasa del tamagno del string, se borra y se ponen solo los datos nuevos
        free(actualData);
        fs[idx].hasData = escribirDesdeBuffer(buf);
        return strlen(buf);
    }

    char* newString = malloc(sizeof(char)*(size+1));

    newString[0] = '\0';

    strcpy(newString,actualData + offset);

    free(actualData);

    strcat(newString,buf);

    newString[size] = '\0';

    fs[idx].hasData = escribirDesdeBuffer(newString);

    free(newString);

    return size;

}

static int fs_open(const char *path, struct fuse_file_info *fi) {
	int idx= exists(path);
	if(idx!=-1){
		printf("fs_open: File not found.\n");
        return -ENOENT;	
	}
	
	if(fs[idx].hasData==-1){
		printf("fs_open: Not data.\n");
        return -EISDIR;
	}
	
	int desc;
    desc = open(path, fi->flags);
    if (desc == -1)
        return -errno;  

    fi->fh = desc;  // Guardamos file descriptor en fi
    return 0;
}

static int fs_release(const char *path, struct fuse_file_info *fi) {
    close(fi->fh);  // Cerramos archivo
    return 0;
}

static int fs_getxattr(const char *path, const char *name, char *value, size_t size) {
    return -ENOTSUP;  // No tenemos atributos extendidos
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
    init_datasystem("dataSystem.bin");
    // Ajustar los argumentos para FUSE
    argv[argc-2] = argv[argc-1]; // Mueve el punto de montaje al lugar del fichero
    argv[argc-1] = NULL;         // Elimina el último argumento para ajustar a FUSE
    argc--;

    // Iniciar FUSE
    return fuse_main(argc, argv, &fs_oper, fs);
}
