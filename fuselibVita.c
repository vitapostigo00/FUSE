#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include "fuseHeaders.h"

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

elementoTabla* pathExists(char* path){
    elementoTabla* copia = (elementoTabla*) globalTable;
    while(copia != NULL && strcmp(copia -> path, path) != 0){
        copia = copia -> next;
    }
    return copia; //Devuelve NULL si no existe
}
////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////

char* checksPrevios(char* newDir){
    if(strchr(newDir, '/')){
        return "The new directory can't contain the: / sign.";
    }
    if(strchr(newDir, '?')){
        return "The new directory can't contain the: ? sign.";
    }
    if(strlen(newDir) + strlen(currentPath) >= LONGESTPATHSIZE - 1){
        return "Path is too long for the given dir to create.";
    }
    
    return NULL;
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


int subdir_inmediato(const char* parent,const char* child) {
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    if (parent[parent_len - 1] == '/') {
        parent_len--;
    }

    // Asegurarse de que 'child' comienza con 'parent' y que el siguiente caracter es '/'
    if (strncmp(parent, child, parent_len) == 0 && child[parent_len] == '/') {
        // Verificar que solo hay un nivel de directorio de diferencia
        const char* rest = child + parent_len + 1;
        // Verificar que no hay más slashes después del primer nivel
        return (strchr(rest, '/') == NULL || strchr(rest, '/') == rest + strlen(rest) - 1);
    }

    return 0;
}

char* ultimoComponente(char* path) {
    // Hacer una copia de la ruta para no modificar la original
    char* path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("Error al duplicar la ruta");
        return NULL;
    }

    // Utilizar basename para obtener el último componente de la ruta
    char* last_component = basename(path_copy);

    // Hacer otra copia para retornar, porque path_copy será liberada
    char* result = strdup(last_component);
    free(path_copy);  // Liberar la memoria de la copia original

    return result;
}

char* ls(){ 
    char* retorno = malloc(sizeof(char) * LONGESTPATHSIZE); // Reservar memoria para retorno
    
    if (retorno == NULL) {
        perror("Error al asignar memoria para retorno");
        return NULL;
    }

    retorno[0] = '\0'; // Inicializar retorno como una cadena vacía

    elementoTabla* copy = globalTable->next; // No necesitas hacer un casting innecesario aquí

    while (copy != NULL) {
        if (subdir_inmediato(currentPath, copy->path)) {
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


    printf("NewString: %s\n",newString);
    fflush(stdout);

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


int main(int argc, char **argv) {

    int initialization = initFromBin("filesystem.bin");

    if(initialization == 0){

        copiarDesdeArchivo("portaTruco.mp3","portatruco");
        copiarDesdeArchivo("arcade.mp4","arcade");


        printf("Filesystem propperly mounted\n");

        createDir("Dir48");
        createDir("dir33");
        createDir("dir59");

        ls();

        devolverArchivo("arcadia.mp4","arcade");
        devolverArchivo("temazo.mp3","portatruco");

        

    }else{
        printf("Error at init, aborpting.\n");
    }

    return initialization;
}
