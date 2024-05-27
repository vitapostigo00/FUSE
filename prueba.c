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


void remove_first_n_chars(char *str, int n) {
    int length = strlen(str);

    // Verificar que n no exceda la longitud del string
    if (n > length) {
        n = length;
    }

    // Desplazar los caracteres restantes hacia el inicio del string
    for (int i = 0; i < length - n + 1; i++) {
        str[i] = str[i + n];
    }
}

int main() {
    char original[] = "Hola, mundo";
    int n = 5; // Número de caracteres a eliminar

    printf("Original: %s\n", original);
    remove_first_n_chars(original, n);
    printf("Modificado: %s\n", original);

    return 0;
}