#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LONGEST_FILENAME 64

char* ultimoElemento(const char *cadena) {
    
    if (strcmp(cadena, "/") == 0) {
        return NULL;
    }

    char* resultado = malloc(sizeof(char) * LONGEST_FILENAME);
    if (resultado == NULL) {
        perror("No se pudo asignar memoria");
        return NULL;
    }

    int longitud = strlen(cadena);
    int i = longitud - 1;

    while (i >= 0 && cadena[i] != '/') {
        i--;
    }
    strncpy(resultado, cadena + i + 1, longitud - i - 1);

    resultado[longitud - i - 1] = '\0';

    return resultado;
}

int main() {
    const char* albacete1 = "/dir1/juan/sardinas";
    const char* albacete2 = "/sardinas";

    char* copia1 = ultimoElemento(albacete1);
    char* copia2 = ultimoElemento(albacete2);

    if (copia1 != NULL) {
        printf("Copia1: %s\n", copia1);
        free(copia1);
    } else {
        printf("No se pudo obtener el último elemento de albacete1.\n");
    }

    if (copia2 != NULL) {
        printf("Copia2: %s\n", copia2);
        free(copia2);
    } else {
        printf("No se pudo obtener el último elemento de albacete2.\n");
    }

    return 0;
}
