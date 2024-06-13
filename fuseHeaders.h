#ifndef FUSEHEADERS_H
#define FUSEHEADERS_H

#define LONGEST_FILENAME 64  // Tamaño de path más largo permitido
#define FILESYSTEM_SIZE 1024 // Número de entradas en el sistema de archivos

typedef struct info {
    char path[LONGEST_FILENAME];
    int siguiente;
    time_t creation_time;
    time_t last_access;
    time_t last_modification;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    nlink_t nlink;
    int hasData;
} FileSystemInfo;

FileSystemInfo* currentDir;

#endif