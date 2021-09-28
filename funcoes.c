#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"


void inicializaArquivo(){
//ponteiro para o arquivo
FILE *arq;
    arq = fopen("ArqDisco.bin", "w+");
    if (arq == NULL){
        printf("Problemas na criacao do arquivo\n");
        return;
    }
    for(int m = 0; m < 256; m++){
        for(int i = 0; i < (sizeof(char)*32000); i++){
        fputs("0", arq);
    }
    }
    fclose(arq);
}
