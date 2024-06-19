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

DataSystemInfo *ds;
size_t dataFilesize;
int dataFd;
struct stat dataSt;

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

    if (fstat(dataFd, &dataSt) == -1) {       
        perror("fstat");
        exit(EXIT_FAILURE);
    }

    dataFilesize = DATASYSTEM_SIZE * sizeof(DataSystemInfo);
    if (dataSt.st_size != dataFilesize) {
        if (ftruncate(dataFd, dataFilesize) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }

    ds = mmap(NULL, dataFilesize, PROT_READ | PROT_WRITE, MAP_SHARED, dataFd, 0);
    if (ds == MAP_FAILED) {
        perror("mmap ds");
        exit(EXIT_FAILURE);
    }

    if (dataSt.st_size == 0) {
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

int copiarSinCheck(int primBloque,const char* contenido, int size){
    int iterador = primBloque;

    if(ds[primBloque].firstDataBlock!=primBloque){
        printf("Error, datos inconsistentes.\n");
        return -1;
    }
    int bloqueAEscribir = primerElementoLibre();
    if(bloqueAEscribir==-1){
        printf("FileSystem Is full.\n");
        borrarFile(primBloque);
        return -1;
    }

    int tamTotal = ds[iterador].totalSize+size;
    while(ds[iterador].siguiente!=-1){
        ds[iterador].totalSize=tamTotal;
        iterador = ds[iterador].siguiente;
    }
    //Anexionamos el siguiente bloque
    ds[iterador].siguiente = bloqueAEscribir;

    //Declaramos los datos del bloque
    ds[bloqueAEscribir].firstDataBlock = primBloque;
    ds[bloqueAEscribir].currentBlockSize = size;
    ds[bloqueAEscribir].totalSize = tamTotal;
    memcpy(ds[bloqueAEscribir].dat,contenido,size);
    ds[bloqueAEscribir].siguiente = -1;
    return 0;
}

int copiarStream(int primBloque,const char* dataStream, unsigned long tamano,int blockNumToWrite){
    int i;
    int j;
    int currentBlock = primBloque;
	printf("blockNumtoWrite: %i\n", blockNumToWrite);
	fflush(stdout);
	printf("tamano: %lu\n", tamano);
	fflush(stdout);
    for(i=0 ; i < blockNumToWrite; i++){

        ds[currentBlock].firstDataBlock = primBloque;
        
        if(i == blockNumToWrite-1){
            ds[currentBlock].currentBlockSize = tamano%BLOCKSIZE;
            printf("ds[currentBlock].currentBlockSize: %i\n",ds[currentBlock].currentBlockSize);
            fflush(stdout);
        }else{
            ds[currentBlock].currentBlockSize = BLOCKSIZE;
        }

        ds[currentBlock].totalSize = tamano; 

        if(currentBlock==-1){
            printf("Error en la gestion, el archivo no ha sido guardado\n");
            borrarFile(primBloque);
            return -1;
        }

        for(j=0; j < BLOCKSIZE; j++){
            ds[currentBlock].dat[j] = dataStream[i * BLOCKSIZE + j];
        }

        if(i != blockNumToWrite - 1){
            ds[currentBlock].siguiente = primerElementoLibre();
        }

        currentBlock = primerElementoLibre();
    }

    return 0;
}

int createEmpty(){
	int bloque=primerElementoLibre();
	ds[bloque].firstDataBlock= bloque;
	ds[bloque].currentBlockSize= 0;
	ds[bloque].totalSize=0;
	ds[bloque].dat[0]= '\0';
	ds[bloque].siguiente=-1;
	return bloque;
}

int escribirDesdeBuffer(const char* dataStream, unsigned long tamano){
    
    if(tamano==0){
		return createEmpty();
	}
    printf("tamanodesdebuffer: %lu\n", tamano);
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
    int copiado = copiarStream(primBloque,dataStream,tamano,blockNumToWrite);
    if(copiado == 0){
        return primBloque;
    }

    return -1;

}

int copiarFichero(int primBloque,FILE* archivo,long tamano,int blockNumToWrite){
    int i;
    int j;
    int currentBlock = primBloque;

    for(i=0 ; i < blockNumToWrite; i++){

        ds[currentBlock].firstDataBlock = primBloque;
        
        if(i == blockNumToWrite-1){
            ds[currentBlock].currentBlockSize = tamano%BLOCKSIZE;
        }else{
            ds[currentBlock].currentBlockSize = BLOCKSIZE;
        }

        ds[currentBlock].totalSize = tamano; 

        if(currentBlock==-1){
            printf("Error en la gestion, el archivo no ha sido guardado\n");
            borrarFile(primBloque);
            return -1;
        }

        for(j=0; j < BLOCKSIZE; j++){
            fseek(archivo, i * BLOCKSIZE + j, SEEK_SET);
            fread(&ds[currentBlock].dat[j], sizeof(unsigned char), 1, archivo);
        }

        if(i != blockNumToWrite - 1){
            ds[currentBlock].siguiente = primerElementoLibre();
        }

        currentBlock = primerElementoLibre();
    }

    return 0;
}

int insertData(const char* filename){

    FILE* archivo = fopen(filename, "rb");
    long tamano = 0;

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return -1;
    }

    fseek(archivo, 0, SEEK_END);
    tamano = ftell(archivo);

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

char* cat(int dat) {

    if(ds[dat].firstDataBlock == -1){
        printf("Archivo no inicializado.\n");
        return NULL;
    }

    if (ds[dat].firstDataBlock != dat) {
        printf("No es el primer bloque, del archivo. Primer bloque: %i\n",ds[dat].firstDataBlock);
        return NULL;
    }

    char* retorno = malloc(sizeof(char)* (ds[dat].totalSize + 1));
    if (retorno == NULL) {
        printf("No se ha podido reservar espacio\n");
        return NULL;
    }

    int currentBlock = dat;
    size_t offset = 0;

    while (currentBlock != -1) {
        memcpy(retorno + offset, ds[currentBlock].dat, ds[currentBlock].currentBlockSize);
        offset += ds[currentBlock].currentBlockSize;
        currentBlock = ds[currentBlock].siguiente;
    }
    strcat(retorno,"\0");

    return retorno;
}

//Hay que pasarle por dónde empieza el archivo a escribir...
void escribirArchivoBinario(const char* nombreArchivo, int dat, size_t tamano) {
    if (ds[dat].firstDataBlock != dat) {
        printf("No es el primer bloque...\n");
        return;
    }

    char* contenido = malloc(ds[dat].totalSize);
    if (contenido == NULL) {
        printf("No se ha podido reservar espacio\n");
        return;
    }

    int currentBlock = dat;
    size_t offset = 0;

    while (currentBlock != -1) {
        memcpy(contenido + offset, ds[currentBlock].dat, ds[currentBlock].currentBlockSize);
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


int borrarFile(int dat){
    if(ds[dat].firstDataBlock == -1){
        printf("Archivo no inicializado.\n");
        return -1;
    }

    if (ds[dat].firstDataBlock != dat) {
        printf("No es el primer bloque, pruebe a borrar: %i\n",ds[dat].firstDataBlock);
        return -1;
    }
    int dataCopy = dat;
    int siguiente;
    do{
        ds[dataCopy].firstDataBlock = -1;
        ds[dataCopy].currentBlockSize = -1;
	    ds[dataCopy].totalSize = 0;
        siguiente = ds[dataCopy].siguiente;
	    ds[dataCopy].siguiente = -1;
        dataCopy = siguiente; 
    }
    while((dataCopy !=-1) && (ds[dataCopy].siguiente != -1));
	
	return 0;
}

size_t sizeOfFile(int dat){
    return (size_t) (ds[dat].totalSize+1);
}
