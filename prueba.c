#include <stdio.h>

int main() {
    int x = 10;       // Una variable entera
    int *ptr1 = &x;   // ptr1 apunta a x

    // Declaración del segundo puntero
    int *ptr2 = ptr1; // ptr2 apunta a la misma dirección que ptr1

    // Imprimir los valores y las direcciones
    printf("Valor de x: %d\n", x);
    printf("Dirección de x: %p\n", (void*)&x);
    printf("Valor de ptr1: %p, lo que apunta ptr1: %d\n", (void*)ptr1, *ptr1);
    printf("Valor de ptr2: %p, lo que apunta ptr2: %d\n", (void*)ptr2, *ptr2);

    // Cambiar el valor a través de ptr2
    *ptr2 = 20;

    // Verificar los valores después de la modificación
    printf("Después de modificar a través de ptr2:\n");
    printf("Valor de x: %d\n", x);
    printf("Valor de ptr1: %p, lo que apunta ptr1: %d\n", (void*)ptr1, *ptr1);
    printf("Valor de ptr2: %p, lo que apunta ptr2: %d\n", (void*)ptr2, *ptr2);

    return 0;
}
