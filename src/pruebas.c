#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "fuselib.c"

int main(int argc, char **argv) {
    const char* initBin = "FUSEINIT/newbin.bin";
    int initialization = initFromBin(initBin);

    if(initialization == 0){
        printf("Filesystem propperly mounted\n");
        devolverArchivo("sanguijuela.mp4","videazo");
        //copiarDesdeArchivo("arcade.mp4","videazo");
        mostrarTodo();
    }else{
        printf("Error at init, aborpting.\n");
        return 1;
    }

    const char* persistBin = "FUSEINIT/newbin.bin";
    exitFileSystem(persistBin);
    return initialization;
    
}