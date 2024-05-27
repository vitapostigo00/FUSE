#include <stdio.h>
#include <string.h>

/*
void prueba(){
    char* original = "Hola, Hola";
    int start = 4; // El índice desde donde quieres empezar a copiar, 0-based
    int length = strlen(original) - start;

    // Crear un buffer para la subcadena
    char subcadena[length + 1]; // +1 para el terminador nulo

    // Copiar la subcadena usando strncpy
    strncpy(subcadena, &original[start], length);

    // Asegurarse de que la subcadena esté terminada en nulo
    subcadena[length] = '\0';

    // Imprimir la subcadena
    printf("Subcadena:|%s\n", subcadena);
    
}*/


int main() {

    char* paco = "hola\0";

    if(paco[4] == "\0"){
        printf("si");
    }else{
        printf("no");
    }

    printf("%lu",strlen(paco));


    return 0;

}
