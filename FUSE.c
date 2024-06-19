#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <sys/statfs.h>
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
int fs_truncate(const char *path, off_t newsize);
static int fs_rename(const char *from, const char *to);
static int fs_statfs(const char *path, struct statvfs *stbuf);


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
    .getxattr= fs_getxattr,
    .truncate= fs_truncate,
    .rename  = fs_rename,
    //.statfs  = fs_statfs,
};

static const char *fileSystemData = "filesystem.bin";


//~ static int fs_statfs(const char *path, struct statvfs *stbuf){
	//~ memset(stbuf, 0, sizeof(struct statvfs));
	//~ stbuf->f_bsize = BLOCKSIZE;  // Tamaño de bloque
    //~ stbuf->f_frsize = BLOCKSIZE; // Tamaño de fragmento
    //~ stbuf->f_blocks = DATASYSTEM_SIZE; // Total de bloques
    //~ stbuf->f_bfree = bloqueslibres();  // Bloques libres
    //~ stbuf->f_bavail =bloqueslibres(); // Bloques disponibles para usuarios no privilegiados
    //~ stbuf->f_files = FILESYSTEM_SIZE;   // Total de inodos
    //~ stbuf->f_ffree = nodoslibres();    // Inodos libres
    //~ stbuf->f_favail = nodoslibres();   // Inodos disponibles para usuarios no privilegiados
    //~ stbuf->f_fsid = 33;    // Identificador del sistema de archivos
    //~ stbuf->f_namemax = LONGEST_FILENAME;    // Máximo número de caracteres en un nombre de archivo

    //~ return 0;
//~ }

//~ static int fs_statfs(const char *path, struct statvfs* stbuf) {
    //~ stbuf->f_bsize  = 4096; // block size
    //~ stbuf->f_frsize = 4096; // fragment size
    //~ stbuf->f_blocks = 1024; // blocks

    //~ return 0; // assume no errors occurred, just return 0
//~ } 

// Función para obtener atributos de un archivo o directorio
static int fs_getattr(const char *path, struct stat *stbuf) {
    printf("fs_getattr: Path = %s\n", path);
    memset(stbuf, 0, sizeof(struct stat));
	char* fullpath = buildFullPath(path);
	int idx= exists(fullpath);
	free(fullpath);
	printf("Index: %i\n", idx);
	
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = fs[0].mode;
        stbuf->st_nlink = fs[0].nlink + 2;
        stbuf->st_atime = fs[0].last_access;
        stbuf->st_mtime = fs[0].last_modification;
        stbuf->st_ctime = fs[0].creation_time;
		stbuf->st_uid	= fs[0].uid;
		stbuf->st_gid	= fs[0].gid;
		stbuf->st_size  = sizeof(FileSystemInfo);
		stbuf->st_blocks= 0;
    } else if((idx != -1) && (fs[idx].hasData == -1)){
		stbuf->st_mode = fs[idx].mode;
        stbuf->st_nlink = fs[idx].nlink + 2;
        stbuf->st_atime = fs[idx].last_access;
        stbuf->st_mtime = fs[idx].last_modification;
        stbuf->st_ctime = fs[idx].creation_time;
		stbuf->st_uid= fs[idx].uid;
		stbuf->st_gid= fs[idx].gid;
		stbuf->st_size  = sizeof(FileSystemInfo);
		stbuf->st_blocks= 0;
	} else if ((idx != -1) && (fs[idx].hasData != -1)){
		stbuf->st_mode = fs[idx].mode;
        stbuf->st_nlink = 1;
        stbuf->st_atime = fs[idx].last_access;
        stbuf->st_mtime = fs[idx].last_modification;
        stbuf->st_ctime = fs[idx].creation_time;
		stbuf->st_uid   = fs[idx].uid;
		stbuf->st_gid   = fs[idx].gid;
		stbuf->st_size  = ds[fs[idx].hasData].totalSize;
		stbuf->st_blocks= ((ds[fs[idx].hasData].totalSize)/BLOCKSIZE) + 1;
	} else {
		return -ENOENT;
	} 

    return 0;
}

// Función para leer un directorio
static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
	(void) fi;
	char* fullpath = buildFullPath(path);
	int idx= exists(fullpath);
	free(fullpath);
	
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
    int result = createFile(path, "");
    if (result == -1) {
        printf("fs_create: Failed to create file.\n");
        return -EPERM;
    }
    return 0;
}

static int fs_rename(const char *from, const char *to) {
    //Debe existir el elemento a mover
    printf("From: %s\n", from);
    printf("To: %s\n", to);
    
    char* absoluteFrom = buildFullPath(from);
    int idxFrom = exists(absoluteFrom);
    if(idxFrom == -1){
        free(absoluteFrom);
        return -ENOENT;
    }
    int isDir = fs[idxFrom].hasData;

    //Primero miramos si es mover a otro directorio o simplemente renombrar:
    char* absoluteTo = buildFullPath(to);
    int idxTo = exists(absoluteTo);
    if(idxTo == -1){
        //Rename
        int tamano = (strlen(to)+strlen(from)+1);
        if(tamano >= LONGEST_FILENAME){
            printf("Path demasiado largo.");
            free(absoluteFrom);
            free(absoluteTo);
            return -ENAMETOOLONG;
        }
        char* newDir = malloc(sizeof(char)*tamano);
        strcpy(newDir,from);
        strcat(newDir,to);
        int futureIdx = exists(newDir);
        if(futureIdx != -1){
            free(absoluteFrom);
            free(absoluteTo);
            free(newDir);
            printf("El entry ya existe\n");
            return -EEXIST;
        }
        strcpy(fs[idxFrom].path,newDir);
        if(isDir==-1){
			int i;
            for(i=1; i<FILESYSTEM_SIZE;i++){
                reemplazar_prefijo(fs[i].path, absoluteFrom, newDir);
            }
        }
        free(absoluteFrom);
        free(absoluteTo);
        free(newDir);
        return 0;
    }


    if(fs[idxTo].hasData!=-1){
        printf("No puedes mover un elemento a un archivo.");
        free(absoluteFrom);
        free(absoluteTo);
        return -ENOTDIR;
    }

    int tamano = (strlen(absoluteTo)+strlen(from)+1);
    if(tamano >= LONGEST_FILENAME){
        printf("Path demasiado largo.");
        free(absoluteFrom);
        free(absoluteTo);
        return -ENAMETOOLONG;
    }

    char* newDir = malloc(sizeof(char)*tamano);

    char* newFromCopia = malloc(sizeof(char)*LONGEST_FILENAME);

    ultimoElemento(from,newFromCopia);

    strcpy(newDir,absoluteTo);
    strcat(newDir,newFromCopia);

    int futureIdx = exists(newDir);
    if(futureIdx != -1){
        free(absoluteFrom);
        free(absoluteTo);
        free(newDir);
        free(newFromCopia);
        printf("El entry ya existe\n");
        return -ENOENT;
    }

    strcpy(fs[idxFrom].path,newDir);

    if(isDir==-1){
		int i;
        for(i=1; i<FILESYSTEM_SIZE;i++){
            reemplazar_prefijo(fs[i].path, absoluteFrom, newDir);
        }
    }
    
    free(absoluteFrom);
    free(absoluteTo);
    free(newDir);
    free(newFromCopia);
    return 0;
}

static int fs_unlink(const char *path){
    printf("fs_unlink: Path = %s\n", path);
    deleteElement(path);
    return 0;
}

int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("fs_read: Path = %s\n", path);
	char* fullpath = buildFullPath(path);
	int idx= exists(fullpath);
	free(fullpath);
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
		memcpy(buf,temp, strlen(temp));
		free(temp);
	} else{
		size=0;
	}
    
    return size;
}

int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    //Hay que mirar qué se hace con fi...
	printf("fs_write: Path = %s\n", path);
	
	char* fullpath = buildFullPath(path);
	int idx= exists(fullpath);
	free(fullpath);
	if(idx==-1){
		printf("fs_write: File not found.\n");
        return -ENOENT;	
	}
    
    if(fs[idx].hasData==-1){
		printf("fs_write: Not a file.\n");
        return -EISDIR;
	}

    if(borrarFile(fs[idx].hasData)==-1){
        return -EIO;
    }

    fs[idx].last_access = time(0);          
    fs[idx].last_modification = time(0);
	
	char* newbuf=malloc(sizeof (char) * (size+1));
	memcpy(newbuf,buf,size);
	newbuf[size]='\0';
    fs[idx].hasData = escribirDesdeBuffer(newbuf);
	free(newbuf);
	
    return size;
}

//Implementada esta función para controlar algunos operadores, sin embargo, la lógica está ya implementada en el write
int fs_truncate(const char *path, off_t newsize){
	char* fullpath= buildFullPath(path);
	int idx= exists(fullpath);
	if(idx == -1){
		return -ENOENT;
	}
	
	//ds[fs[idx].hasData].totalSize=newsize;
	free(fullpath);
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
	char* fullpath = buildFullPath(path);
	int idx= exists(fullpath);
	free(fullpath);
	if(idx==-1){
		printf("fs_open: File not found.\n");
        return -ENOENT;	
	}
	
	if(fs[idx].hasData==-1){
		printf("fs_open: Not data.\n");
        return -EISDIR;
	}
	
	if (fi->flags & O_TRUNC) {
        if (truncate(path, 0) == -1) {
            close(fd);
            return -errno;
        }
    }
		
    fi->fh = idx;  // Guardamos file descriptor en fi
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
