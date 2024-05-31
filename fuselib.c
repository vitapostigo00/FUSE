#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "fuseHeaders.h"
#include "fuselibUtilities.c"


extern elementoTabla* globalTable;
extern char* currentPath;

int initEmptyFilesystem(){
    globalTable = (elementoTabla*) malloc(sizeof(elementoTabla));
    if (globalTable == NULL) return 1;
    globalTable -> path = malloc(sizeof(char)*2);

    strcpy(globalTable -> path,"/");

    globalTable -> data = NULL;
    globalTable -> next = NULL;

    currentPath = (char*) malloc(sizeof(char)*LONGESTPATHSIZE);

    if(currentPath == NULL){
        free(globalTable);
        return 1;
    }
    else{
        strcpy(currentPath,"/");
    }

    return 0;
}


int initFromBin(char* myFile) {
    if(initEmptyFilesystem()==1){
        return 1;
    }

    FILE *file;
    char firstChar;
    char numberStr[256];  // Buffer para almacenar el número
    int index;

    // Intenta abrir el archivo en modo binario de lectura
    file = fopen(myFile, "rb");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        return -1;  // Error al abrir el archivo
    }

    // Lee y procesa bloques hasta encontrar un bloque que empieza con '?'
    while (1) {
        if (fread(&firstChar, sizeof(char), 1, file) != 1) {
            printf("Error o fin del archivo alcanzado inesperadamente.\n");
            fclose(file);
            return -1;
        }

        if (firstChar == '?') {
            printf("Encontrado el caracter de terminacion '?'.\n");
            break;
        } else if (firstChar != '|') {
            printf("Formato de archivo inesperado, se esperaba '|', se encontro: %c\n", firstChar);
            fclose(file);
            return -1;
        }

        // Inicializa el buffer y el índice para cada nuevo bloque
        memset(numberStr, 0, sizeof(numberStr));
        index = 0;
        
        // Leer caracteres hasta encontrar '/'
        while (fread(&firstChar, sizeof(char), 1, file) == 1 && firstChar != '/') {
            if (index < sizeof(numberStr) - 1) {
                numberStr[index++] = firstChar;
            }
        }
        numberStr[index] = '\0';

        if (firstChar != '/') {
            printf("No se encontro el caracter '/' despues del numero.\n");
            fclose(file);
            return -1;
        }

        long numChars = atol(numberStr);

        // Reservar memoria y leer la cantidad de caracteres especificada
        char *data = malloc(numChars + 1);
        if (data == NULL) {
            printf("No se pudo reservar memoria.\n");
            fclose(file);
            return -1;
        }
        if (fread(data, sizeof(char), numChars, file) != numChars) {
            printf("No se pudo leer la cantidad esperada de caracteres.\n");
            free(data);
            fclose(file);
            return -1;
        }

        data[numChars] = '\0';
        
        int aborpt = createRawEntry(data);
        if(aborpt==1){
            printf("Fallo en el formato de entrada: \n");
            return 1;
        }
        

        free(data);
        
    }

    // Cierra el archivo
    fclose(file);
    return 0;  // Éxito
}

void cleanFileSystem(){
    elementoTabla* next;
    while(globalTable!=NULL){
        next = globalTable -> next;
        free(globalTable -> path);
        if(globalTable -> data != NULL){
            free(globalTable -> data ->binario);
            free(globalTable -> data);
        }
        free(globalTable);
        globalTable = next;
    }
    printf("FileSystem has been cleaned. Closing.");
}

void exitFileSystem(){
    //Primero los datos al bin
    cleanFileSystem();
}

void totalsize(){
    int contador = 0;
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL ){
        printf("Nombre: %s\n",copia->path);
        copia = copia -> next;
        contador++;
    }
    printf("Totasize: %i\n",contador);
}

int createRawEntry(char* newEntry){

    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(newEntry)+2));
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return 1;
    }

    strcpy(newString,"/"); 
    strcat(newString, newEntry);
    

    // Encontrar el último elemento en la tabla global
    elementoTabla* toAppend = globalTable;
    while (toAppend->next != NULL){
        toAppend = toAppend->next;
    }

    // Crear un nuevo elemento para el nuevo directorio
    toAppend->next = malloc(sizeof(elementoTabla));
    if (toAppend->next == NULL) {
        perror("Error al asignar memoria para nuevo elemento de la tabla");
        free(newString);
        return 1;
    }
    toAppend = toAppend->next;

    // Asignar la ruta al nuevo elemento
    toAppend->path = malloc(sizeof(char)*(strlen(newString)+1));
    if (toAppend->path == NULL) {
        perror("Error al asignar memoria para la ruta del nuevo elemento");
        free(toAppend->next);
        free(newString);
        return 1;
    }
    strcpy(toAppend->path, newString);
    free(newString);

    // Inicializar los otros campos del nuevo elemento
    toAppend->data = NULL;
    toAppend->next = NULL;

    return 0;
}


int createDir(char* newDir){
    // Verificar errores previos
    char* msgError = checksPrevios(newDir);   
    if(msgError != NULL){
        printf("%s\n", msgError);
        return 1;
    }

    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(newDir)+2));
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return 1;
    }

    strcpy(newString, currentPath);
    strcat(newString, newDir);
    strcat(newString, "/");

    // Verificar si el directorio ya existe
    if (pathExists(newString) != NULL){
        printf("El elemento a crear ya existe\n");
        free(newString);
        return 1;
    }

    // Encontrar el último elemento en la tabla global
    elementoTabla* toAppend = globalTable;
    while (toAppend->next != NULL){
        toAppend = toAppend->next;
    }

    // Crear un nuevo elemento para el nuevo directorio
    toAppend->next = malloc(sizeof(elementoTabla));
    if (toAppend->next == NULL) {
        perror("Error al asignar memoria para nuevo elemento de la tabla");
        free(newString);
        return 1;
    }
    toAppend = toAppend->next;

    // Asignar la ruta al nuevo elemento
    toAppend->path = malloc(sizeof(char)*(strlen(newString)+1));
    if (toAppend->path == NULL) {
        perror("Error al asignar memoria para la ruta del nuevo elemento");
        free(toAppend->next);
        free(newString);
        return 1;
    }
    strcpy(toAppend->path, newString);
    free(newString);

    // Inicializar los otros campos del nuevo elemento
    toAppend->data = NULL;
    toAppend->next = NULL;

    return 0;
}

char* ls(){ 
    char* retorno = malloc(sizeof(char) * LONGESTPATHSIZE); // Reservar memoria para retorno
    
    if (retorno == NULL) {
        perror("Error al asignar memoria para retorno");
        return NULL;
    }

    retorno[0] = '\0'; // Inicializar retorno como una cadena vacía

    elementoTabla* copy = globalTable -> next; // No necesitas hacer un casting innecesario aquí

    while (copy != NULL) {
        if (strcmp(currentPath, copy -> path)!=0 && subdir_inmediato(currentPath, copy->path)) {
            strcat(retorno, ultimoComponente(copy->path));
            strcat(retorno, "   ");
        }
        copy = copy->next;
    }

    printf("%s\n", retorno);

    return retorno;
}

int guardarDatos(char* filename, char* data, int size) {
    // Verificar errores previos
    char* msgError = checksPrevios(filename);   
    if(msgError != NULL){
        printf("%s\n", msgError);
        return 1;
    }

    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(filename)+1));
    newString[0]='\0';
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return 1;
    }

    strcpy(newString, currentPath);
    strcat(newString, filename);

    // Verificar si el directorio ya existe
    if (pathExists(newString) != NULL){
        printf("El elemento a crear ya existe\n");
        free(newString);
        return 1;
    }

    // Encontrar el último elemento en la tabla global
    elementoTabla* toAppend = globalTable;
    while (toAppend->next != NULL){
        toAppend = toAppend->next;
    }

    // Crear un nuevo elemento para el nuevo directorio
    toAppend -> next = malloc(sizeof(elementoTabla));
    if (toAppend->next == NULL) {
        perror("Error al asignar memoria para nuevo elemento de la tabla");
        free(newString);
        return 1;
    }
    toAppend = toAppend->next;

    // Asignar la ruta al nuevo elemento
    toAppend -> path = malloc(sizeof(char)*(strlen(newString)+1));
    if (toAppend->path == NULL) {
        perror("Error al asignar memoria para la ruta del nuevo elemento");
        free(toAppend->next);
        free(newString);
        return 1;
    }
    strcpy(toAppend->path, newString);
    free(newString);

    toAppend->next = NULL;

    toAppend -> data = (TFiles*) malloc(sizeof(TFiles));

    if (toAppend -> data == NULL) {
        printf("Error en la asignación de memoria.\n");
        //mirar qué liberar
        return 1;
    }

    toAppend-> data -> size = size;

    // Inicializar los otros campos del nuevo elemento
    toAppend -> data -> binario = (char *) malloc(size);

    if (toAppend -> data -> binario == NULL) {
        fprintf(stderr, "Error en la asignación de memoria para datos.\n");
        //mirar qué liberar
        return 1;
    }
    memcpy(toAppend -> data -> binario, data, size);

    return 0;
}

void pwd(){
    printf("%s\n",currentPath);
}


void changeDirectory(char* newDir){
    if(strcmp(newDir,"..")==0){
        if(strcmp(currentPath,"/")==0){
            printf("Ya esta en el directorio superior, no se puede subir.\n");
        }else{
            remove_last_element();
            printf("Directorio cambiado.\n");
        }
        return;
    }
    // Crear la ruta completa del nuevo directorio
    char* newString = malloc(sizeof(char)*(strlen(currentPath)+strlen(newDir)+2));
    newString[0]='\0';
    if (newString == NULL) {
        perror("Error al asignar memoria para newString");
        return;
    }

    strcpy(newString, currentPath);
    strcat(newString, newDir);
    strcat(newString, "/");

    elementoTabla* savePath = pathExists(newString);
    free(newString);
    if (savePath != NULL){
        strcpy(currentPath,savePath->path);
        printf("Directorio cambiado\n");
    }else{
        printf("El directorio no existe\n");
    }

}

void copiarDesdeArchivo(const char* filename, char* newFile){
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Lee los datos del archivo
    char *buffer = (char *) malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error en la asignación de memoria para buffer.\n");
        fclose(file);
        return;
    }
    fread(buffer, 1, file_size, file);
    fclose(file);

    // Inserta los datos en la lista
    guardarDatos(newFile, buffer, file_size);

    free(buffer);
    
}

int devolverArchivo(char* nuevoArchivo,char* archivoEnFUSe){
    char* nombreAbuscar = malloc(sizeof(char)*(strlen(currentPath)+strlen(archivoEnFUSe)+1));
    strcpy(nombreAbuscar,currentPath);
    strcat(nombreAbuscar,archivoEnFUSe);

    elementoTabla* copia = pathExists(nombreAbuscar);

    if (copia == NULL){
        printf("El elemento a devolver no se ha encontrado!\n");
        free(nombreAbuscar);
        return 1;
    }

    FILE *file = fopen(nuevoArchivo, "wb");  // Abre el archivo en modo escritura binaria
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo para escribir.\n");
        return 1;
    }

    fwrite(copia->data->binario, 1, copia->data->size, file);  // Escribe los datos binarios en el archivo

    fclose(file);  // Cierra el archivo
    return 0;
}

/*
Se pueden dar 4 casos:
    from: fichero    to: fichero        -> cambiar nombre                           Este ya esta listo
    from: fichero    to: directorio     -> cambiar path                             Este ya esta listo
    from: directorio to: fichero        -> no es posible <-                         Este ya esta listo
    from: directorio to: directorio     -> cambiar directorio y todos sus hijos     Este ya esta listo

    Directorio es cuando y solo cuando:
    path termina en /       o 
    path es exactamente ..

    tengo que acordarme de usar la función: cambiarHijos() a ver si funciona.

*/
void renombrar(const char* from,const char* to){
    if(to[strlen(to)-1]!='/'&&strcmp(to,"..")!=0){//Si se da este caso, to es fichero
        //Controlamos que no sea from: directorio to: fichero
        if(from[strlen(from)-1]=='/'&&strcmp(from,"..")!=0){perror("No se puede from: directorio to: fichero.\n");return;}
        //Llegados aqui, suponemos que es from:fichero to:fichero
        char* aux = absoluteFromRelative(from);
        elementoTabla* copyPath = pathExists(aux);
        if(copyPath==NULL){
            perror("El fichero a cambiar de nombre no se ha podido encontrar.\n");
            return;
        }
        char* copiaTo = malloc(sizeof(char)*(strlen(to)+1));
        strcpy(copiaTo,to);
        char* errores = checksPrevios(copiaTo);//Para no pasarle const char* como argumento
        free(copiaTo);
        if(errores!=NULL){  //Si contiene caracteres prohibidos, abortamos
            perror(errores);
            return;
        }
        remove_last_elementArg(aux);
        if(strcmp(absoluteFromRelative(from),aux)==0){
            free(copyPath -> path);
            copyPath -> path = malloc(sizeof(char)*(strlen("/")+strlen(to)+1));
            if(copyPath -> path==NULL){
                perror("No se ha podido reservar memoria./n");
                free(aux);
                return;
            }
            copyPath -> path[0] = '\0';
            strcpy(copyPath -> path,"/");
            strcat(copyPath -> path,to);
            free(aux);
            return;
        }
        free(copyPath -> path);
        copyPath -> path = malloc(sizeof(char)*(strlen(aux)+strlen(to)+1));
        if(copyPath -> path==NULL){
            perror("No se ha podido reservar memoria./n"); free(aux);
        }
        strcpy(copyPath -> path,aux);
        free(aux);
        strcat(copyPath -> path,to);
        printf("Fichero renombrado, nueva ruta:%s\n",copyPath -> path);
        return;
    }
    else{//To es un directorio, implementar los casos correspondientes...
        if(from[strlen(from)-1]!='/' && strcmp(from,"..")!=0){      //from es fichero
            //Convertir to en absoluto, guardar from (si existe) con un nuevo path construido con absolute(to) + from
            char* fromCopy = absoluteFromRelative(from);
            elementoTabla* aMoverFrom = pathExists(fromCopy);
            if(aMoverFrom==NULL){
                free(fromCopy);
                printf("No se ha podido encontrar el elemento a mover.");
                return;
            }
            
            //Aqui sabemos que to es directorio y el fichero de from existe. Vemos si to existe también.
            char* toCopy = absoluteFromRelative(to);

            if(strcmp(toCopy,to)==0){
                free(toCopy);
                toCopy = strdup("/\0");
            }

            elementoTabla* aMoverTo = pathExists(toCopy);
            if(aMoverTo==NULL){
                free(fromCopy);
                free(toCopy);
                printf("No se ha podido encontrar el directorio a donde se quiere mover el elemento.\n");
                return;
            }

            //Aqui ambos existen, tenemos que hacer un nuevo path de la forma: toCopy+from
            free(aMoverFrom -> path);
            aMoverFrom -> path = malloc(sizeof(char)*(strlen(toCopy)+strlen(from)+1));
            aMoverFrom -> path[0] = '\0';
            strcpy(aMoverFrom -> path,toCopy);
            strcat(aMoverFrom -> path,from);
            //Listo, liberamos punteros y retornamos.
            free(fromCopy);
            free(toCopy);
            return;
        }
        else{//from: directorio to: directorio -> cambiar directorio y todos sus hijos
            char* fromCopy = absoluteFromRelative(from);
            elementoTabla* aMoverFrom = pathExists(fromCopy);
            if(aMoverFrom==NULL){
                free(fromCopy);
                printf("No se ha podido encontrar el elemento a mover.");
                return;
            }

            char* toCopy = absoluteFromRelative(to);
            if(strcmp(toCopy,to)==0){   //no se si este if hace falta.
                free(toCopy);
                toCopy = strdup("/\0");
            }

            elementoTabla* aMoverTo = pathExists(toCopy);
            if(aMoverTo==NULL){
                free(fromCopy);
                free(toCopy);
                printf("No se ha podido encontrar el directorio a donde se quiere mover el elemento.\n");
                return;
            }

            char* copiaHijos = strdup(toCopy);
            strcat(copiaHijos,from);
            cambiarHijos(fromCopy,copiaHijos);

            free(copiaHijos);
            free(aMoverFrom -> path);

            aMoverFrom -> path = malloc(sizeof(char)*(strlen(toCopy)+strlen(from)+1));

            strcpy(aMoverFrom -> path, toCopy);
            strcat(aMoverFrom -> path, from);
            return;
        }
    }
    return;

}

void rmfile(char* filename){

    elementoTabla* copia = (elementoTabla*) globalTable;

    while(copia -> next != NULL && strcmp(copia -> next -> path, filename)!= 0){
        copia = copia -> next;
    }

    if(copia -> next == NULL){
        printf("No se ha podido encontrar el fichero");
        return;
    }

    printf("Node found: %s\n",copia -> next -> path);


}


int main(int argc, char **argv) {

    int initialization = initFromBin("filesystem.bin");

    if(initialization == 0){
        printf("Filesystem propperly mounted\n");
        /*
        ls();
        changeDirectory("dir1");
        createDir("dir33");
        changeDirectory("..");
        renombrar("dir1/","dir2/");
        ls();
        changeDirectory("dir2");
        ls();
        changeDirectory("dir1");
        ls();
        */
        
    }else{
        printf("Error at init, aborpting.\n");
        return 1;
    }
    
    cleanFileSystem();
    return initialization;
}
