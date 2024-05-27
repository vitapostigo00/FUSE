#include <stdio.h>
#include <string.h>

unsigned long miTam(char* laChain){

    return strlen(laChain);
}



int main() {
    // Declarar un puntero a char
    char *miCadena;

    // Asignar una cadena al puntero
    miCadena = "Holi\0 juanijuani";

    printf("Longitud: %lu\n", miTam(miCadena));

}
