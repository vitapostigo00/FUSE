#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LONGEST_FILENAME 256

int isPrefix(char* prefix, char* secondChain){
    if(strlen(prefix) > strlen(secondChain)){
        return -1;
    }
    int i;
    for(i = 0; i < strlen(prefix); i++){
        if(prefix[i] != secondChain[i]){
            return -1;
        }
    }
    return 0;
}

// Función para reemplazar el prefijo de `cadena` con `nuevo_prefijo`
void reemplazar_prefijo(char *cadena,  char *prefijo,  char *nuevo_prefijo) {
    // Verificar si `prefijo` es realmente un prefijo de `cadena`
    if (isPrefix(prefijo, cadena)==0) {
        // Calcular el tamaño de la parte de la cadena después del prefijo
        size_t tamano_prefijo = strlen(prefijo);
        size_t tamano_nuevo_prefijo = strlen(nuevo_prefijo);
        size_t tamano_restante = strlen(cadena) - tamano_prefijo;

        // Mover la parte restante de la cadena hacia adelante
        memmove(cadena + tamano_nuevo_prefijo, cadena + tamano_prefijo, tamano_restante + 1);

        // Copiar el nuevo prefijo en la posición correcta
        memcpy(cadena, nuevo_prefijo, tamano_nuevo_prefijo);
    }
}

int main() {
    char cadena1[LONGEST_FILENAME] = "/Dir1/Dir12/";
    char cadena2[LONGEST_FILENAME] = "/Dir1/Dir12/DirPrueba/";
    char *nuevo_prefijo = "/Dir12/";

    reemplazar_prefijo(cadena2, cadena1, nuevo_prefijo);

    printf("Resultado: %s\n", cadena2);

    return 0;
}
