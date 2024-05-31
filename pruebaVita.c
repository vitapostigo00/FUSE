#include <stdio.h>
#include <string.h>

#define MAX_DIRECCIONES 100
#define MAX_LONGITUD 256

// Función para reemplazar una parte específica de la dirección
void reemplazar_direccion(char direcciones[][MAX_LONGITUD], int num_direcciones, const char *vieja_parte, const char *nueva_parte, const char *comprobar_parte) {
    for (int i = 0; i < num_direcciones; i++) {
        if (strstr(direcciones[i], comprobar_parte) != NULL) {
            char nueva_direccion[MAX_LONGITUD];
            char *pos = strstr(direcciones[i], vieja_parte);
            if (pos != NULL) {
                int indice = pos - direcciones[i];
                strncpy(nueva_direccion, direcciones[i], indice);
                nueva_direccion[indice] = '\0';
                strcat(nueva_direccion, nueva_parte);
                strcat(nueva_direccion, pos + strlen(vieja_parte));
                strcpy(direcciones[i], nueva_direccion);
            }
        }
    }
}

int main() {
    char direcciones[MAX_DIRECCIONES][MAX_LONGITUD] = {
        "/users/documentos/musica/",
        "/users/documentos/fotos/",
        "/users/documentos/videos/",
        "/users/otros/musica/"
    };
    int num_direcciones = 4;
    
    const char *vieja_parte = "musica";
    const char *nueva_parte = "audios";
    const char *comprobar_parte = "documentos";
    
    reemplazar_direccion(direcciones, num_direcciones, vieja_parte, nueva_parte, comprobar_parte);
    
    for (int i = 0; i < num_direcciones; i++) {
        printf("%s\n", direcciones[i]);
    }
    
    return 0;
}
