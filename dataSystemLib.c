#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "fuseHeaders.h"

extern DataSystemInfo *ds;
extern size_t dataFilesize;
extern int dataFd;
extern struct stat dataSt;


void initialize_datasystem() {
    for (int i = 0; i < DATASYSTEM_SIZE; i++) {
	    ds[i].firstDataBlock = -1;
        ds[i].currentBlockSize = -1;
	    ds[i].totalSize = 0;
	    ds[i].siguiente = -1;
    }
}

void init_datasystem(const char *filename) {

    dataFd = open(filename, O_RDWR | O_CREAT, 0666);
    if (dataFd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fstat(dataFd, &dataSt) == -1) {         //Se ha cambiado de dstat a fstat, comprobar esto
        perror("dstat");
        exit(EXIT_FAILURE);
    }

    dataFilesize = DATASYSTEM_SIZE * sizeof(DataSystemInfo) + DATASYSTEM_SIZE * BLOCKSIZE;
    if (dataSt.st_size != dataFilesize) {       //Mirar si es tb st.st_size o dataSt.dataSt_size
        if (ftruncate(dataFd, dataFilesize) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }

    ds = mmap(NULL, DATASYSTEM_SIZE * sizeof(DataSystemInfo), PROT_READ | PROT_WRITE, MAP_SHARED, dataFd, 0);
    if (ds == MAP_FAILED) {
        perror("mmap ds");
        exit(EXIT_FAILURE);
    }

    if (dataSt.st_size == 0) {                  //Mirar si es tb st.st_size o dataSt.dataSt_size
        initialize_datasystem();
    }

}

int primerElementoLibre(){
    int i;
    for(i = 0; i < DATASYSTEM_SIZE; i++){
        if(ds[i].firstDataBlock == -1){
            return i;
        }
    }
    return -1;
}

//Devuelve 1 si hay espacio para el numero de bloques necesario, 0 si no.
int hayEspacio(int numBloques){
    int i;
    int contador = 0;
    for(i = 0; i < DATASYSTEM_SIZE; i++){
        if(ds[i].firstDataBlock==-1){
            contador++;
            if(contador == numBloques){
                return 1;
            }
        }
    }
    return 0;
}

int copiarFichero(int primBloque,FILE* archivo,long tamano,int blockNumToWrite){
    int i;
    int j;
    int currentBlock = primBloque;
    unsigned char byte = 0;

    for(i=0 ; i < blockNumToWrite; i++){

        ds[currentBlock].firstDataBlock = primBloque;
        
        if(i == blockNumToWrite-1){
            ds[currentBlock].currentBlockSize = tamano%BLOCKSIZE;
        }else{
            ds[currentBlock].currentBlockSize = BLOCKSIZE;
        }

        ds[currentBlock].totalSize = tamano; 

        if(currentBlock==-1){
            //Borrar
            printf("Noooo");
            return -1;
        }

        for(j=0; j < BLOCKSIZE; j++){
            fseek(archivo, i * BLOCKSIZE + j, SEEK_SET);
            fread(&ds[currentBlock].data[j], sizeof(unsigned char), 1, archivo);
        }

        if(i != blockNumToWrite - 1){
            ds[currentBlock].siguiente = primerElementoLibre();
        }

        currentBlock = primerElementoLibre();
    }

    return 0;
}



int insertData(char* filename){

    FILE* archivo = fopen(filename, "rb");
    long tamano = 0;

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return -1;
    }

    fseek(archivo, 0, SEEK_END);  // Mover el puntero al final del archivo
    tamano = ftell(archivo);      // Obtener la posición actual del puntero, que es el tamaño del archivo

    int blockNumToWrite = tamano/(BLOCKSIZE)+1;
    
    if(!hayEspacio((tamano/(BLOCKSIZE)+1))){
        printf("No hay espacio.\n");
        return -1;
    }

    int primBloque = primerElementoLibre();

    if(primBloque==-1){
        printf("Filesystem is full");
        return -1;
    }

    //Copiado es 0 cuando falla y 1 si va bien
    int copiado = copiarFichero(primBloque,archivo,tamano,blockNumToWrite);

    fclose(archivo);

    if(copiado == 0){
        return primBloque;
    }

    return -1;

}

char* cat(int data) {

    if(ds[data].firstDataBlock == -1){
        printf("Archivo no inicializado.\n");
        return;
    }

    if (ds[data].firstDataBlock != data) {
        printf("No es el primer bloque, del archivo. Primer bloque: %i\n",ds[data].firstDataBlock);
        return;
    }

    char* retorno = malloc(sizeof(char)* (ds[data].totalSize + 1));
    if (retorno == NULL) {
        printf("No se ha podido reservar espacio\n");
        return;
    }

    int currentBlock = data;
    size_t offset = 0;

    while (currentBlock != -1) {
        memcpy(retorno + offset, ds[currentBlock].data, ds[currentBlock].currentBlockSize);
        offset += ds[currentBlock].currentBlockSize;
        currentBlock = ds[currentBlock].siguiente;
    }
    strcat(retorno,"\0");

    return retorno;
}

//Hay que pasarle por dónde empieza el archivo a escribir...
void escribirArchivoBinario(const char* nombreArchivo, int data, size_t tamano) {
    if (ds[data].firstDataBlock != data) {
        printf("No es el primer bloque...\n");
        return;
    }

    char* contenido = malloc(ds[data].totalSize);
    if (contenido == NULL) {
        printf("No se ha podido reservar espacio\n");
        return;
    }

    int currentBlock = data;
    size_t offset = 0;

    while (currentBlock != -1) {
        memcpy(contenido + offset, ds[currentBlock].data, ds[currentBlock].currentBlockSize);
        offset += ds[currentBlock].currentBlockSize;
        currentBlock = ds[currentBlock].siguiente;
    }

    FILE *archivo = fopen(nombreArchivo, "wb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    size_t escrito = fwrite(contenido, sizeof(char), tamano, archivo);
    if (escrito != tamano) {
        perror("Error al escribir en el archivo");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);
    free(contenido);
    printf("El contenido se escribió correctamente en el archivo %s\n", nombreArchivo);
}


void borrarFile(int data){
    if(ds[data].firstDataBlock == -1){
        printf("Archivo no inicializado.\n");
        return;
    }

    if (ds[data].firstDataBlock != data) {
        printf("No es el primer bloque, pruebe a borrar: %i\n",ds[data].firstDataBlock);
        return;
    }
    int dataCopy = data;
    int siguiente;
    do{
        ds[dataCopy].firstDataBlock = -1;
        ds[dataCopy].currentBlockSize = -1;
	    ds[dataCopy].totalSize = 0;
        siguiente = ds[dataCopy].siguiente;
	    ds[dataCopy].siguiente = -1;
        dataCopy = siguiente; 
    }
    while(ds[dataCopy].siguiente != -1);

    printf("Se han borrado los datos!\n");

}

size_t sizeOfFile(int data){
    return (size_t) (ds[data].totalSize+1);
}
