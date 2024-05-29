#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

// Función para obtener solo el nombre del último elemento de la ruta
char* get_last_component(char* path) {
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

int main() {
    char* paths[] = {"/home/user/Documentos/", "/home/user/Documentos/juan", NULL};

    for (int i = 0; paths[i] != NULL; i++) {
        char* last_component = get_last_component(paths[i]);
        if (last_component) {
            printf("El último componente de '%s' es: '%s'\n", paths[i], last_component);
            free(last_component);  // Liberar la memoria del resultado
        }
    }

    return 0;
}