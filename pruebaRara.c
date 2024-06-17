#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LONGEST_FILENAME 256

char ultimoElemento[](const char *cadena) {
    
    if (strcmp(cadena, "/") == 0) {
        return NULL;
    }

    int longitud = strlen(cadena);
    int i = longitud - 1;

    while (i >= 0 && cadena[i] != '/') {
        i--;
    }
    
    char* resultado = malloc(sizeof(char) * (longitud-i));
    if (resultado == NULL) {
        perror("No se pudo asignar memoria");
        return NULL;
    }
    resultado[0]='\0';
    strncpy(resultado, cadena + i +1, longitud - i);

    return resultado;
}

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

int subdir_inmediato(const char* parent,const char* child) {
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);
    
    if(parent_len >= child_len){
		return -1;
	}
    
    if(isPrefix(parent,child)==-1){
		return -1;
	}	
    
    int cont=0;
    int i;

	for(i=parent_len-1;  i<child_len; i++ ){
		if(child[i]=='/'){
			cont++;
			if(cont>1){
				return -1;
			}
		}
	}

    return 0;
}


int main() {

    printf("Resultado: .%s.\n", ultimoElemento("/dir1"));

    return 0;
}
