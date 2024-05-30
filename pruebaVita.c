#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RUTAS 100
#define MAX_LONGITUD 256

void reemplazar_rutas(const char *rutas[], int num_rutas, const char *directorio_a_reemplazar, const char *nueva_ruta, char rutas_actualizadas[][MAX_LONGITUD]) {
    size_t longitud_directorio = strlen(directorio_a_reemplazar);

    for (int i = 0; i < num_rutas; i++) {
        if (strncmp(rutas[i], directorio_a_reemplazar, longitud_directorio) == 0 &&
            (rutas[i][longitud_directorio] == '/' || rutas[i][longitud_directorio] == '\0')) {
            char *pos = strstr(rutas[i], directorio_a_reemplazar);
            if (pos) {
                size_t longitud_antes = pos - rutas[i];
                strncpy(rutas_actualizadas[i], rutas[i], longitud_antes);
                strcpy(rutas_actualizadas[i] + longitud_antes, nueva_ruta);
                strcpy(rutas_actualizadas[i] + longitud_antes + strlen(nueva_ruta), pos + strlen(directorio_a_reemplazar));
            }
        } else {
            strcpy(rutas_actualizadas[i], rutas[i]);
        }
    }
}

int main() {
    // Listado de todas las rutas en el sistema de archivos
    const char *rutas[MAX_RUTAS] = {
        "/aa/yy/zz",
        "/xx/ya/zz/file1",
        "/xx/yy/file2",
        "/xx/yy",
        "/xx/aa/bb",
        "/xx/yy/zz/subdir/file3"
    };
    int num_rutas = 6;  // Número de rutas en la lista

    // Directorio a reemplazar y la nueva ruta
    char directorio_a_reemplazar[MAX_LONGITUD] = "/xx/yy";
    char nueva_ruta[MAX_LONGITUD] = "/new/path";

    // Array para almacenar las rutas actualizadas
    char rutas_actualizadas[MAX_RUTAS][MAX_LONGITUD];

    // Reemplazar las rutas
    reemplazar_rutas(rutas, num_rutas, directorio_a_reemplazar, nueva_ruta, rutas_actualizadas);

    // Mostrar las rutas actualizadas
    printf("Rutas actualizadas:\n");
    for (int i = 0; i < num_rutas; i++) {
        printf("%s\n", rutas_actualizadas[i]);
    }

    return 0;
}
