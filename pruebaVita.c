#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int is_immediate_subdir(const char* parent, const char* child) {
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    // Eliminar el slash final de 'parent' si existe
    if (parent[parent_len - 1] == '/') {
        parent_len--;
    }

    // Asegurarse de que 'child' comienza con 'parent' y que el siguiente caracter es '/'
    if (strncmp(parent, child, parent_len) == 0 && child[parent_len] == '/') {
        // Verificar que solo hay un nivel de directorio de diferencia
        char* rest = child + parent_len + 1;
        // Verificar que no hay más slashes después del primer nivel
        return (strchr(rest, '/') == NULL || strchr(rest, '/') == rest + strlen(rest) - 1);
    }

    return 0;
}

int main() {
    char* parent_path = "/";
    char* child_path = "/Dir33/";

    if (is_immediate_subdir(parent_path, child_path)) {
        printf("'%s' es un subdirectorio inmediato de '%s'\n", child_path, parent_path);
    } else {
        printf("'%s' NO es un subdirectorio inmediato de '%s'\n", child_path, parent_path);
    }

    return 0;
}