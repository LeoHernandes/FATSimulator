#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"


void inicializaArquivo(){
    MetaDados metaDados = {256, 32000, 0, 1}; //Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
    FILE *arq;                                //ponteiro para o arquivo
    int bytesCluster = 0;
    char zero = 0;
    char valor255 = 255;
    NodoCluster root = {"root", "", 0, NULL};

    arq = fopen("ArqDisco.bin", "r+b");

    if (arq == NULL){
        printf("Problemas na criacao do arquivo\n");
        return;
    }else{
        fwrite(&metaDados, sizeof(MetaDados), 1, arq);
        fwrite("\n", sizeof(char), 1, arq);

        for(int j = 0; j < 256; j++){
            fwrite(&zero, sizeof(char), 1, arq);
        }
        fwrite("\n", sizeof(char), 1, arq);

        //Laco que a criacao dos 256 clusters
        for(int m = 0; m < 255; m++){
            //Laco que controla a criacao de um cluster com 32KB, todos com 0
            for(int i = 0; i < 32000; i++){
            fwrite(&zero, sizeof(char), 1, arq);
            }
            fwrite("\n", sizeof(char), 1, arq);
        }

        fseek(arq, sizeof(MetaDados), SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&valor255, sizeof(char), 1, arq);

        //Criacao do cluster root
        fseek(arq, sizeof(MetaDados)+257, SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&root, sizeof(NodoCluster), 1, arq);
    }
    fclose(arq);
}

int pegaMetadados(MetaDados* metaDados){
/* Carrega os metadados do arquivo de disco se possível
 * Devolve 1 se a operação for feita com sucesso
 * Devolve 0 se a operação falhar */
    FILE *arq;
    arq = fopen("ArqDisco.bin", "rb+");

    //Se houve erro na abertura
    if (arq == NULL){
        printf("Problemas na abertura do arquivo\n");
        return 0;
    }else{
        fread(metaDados, sizeof(MetaDados), 1, arq);
        fclose(arq);
    }

    return 1;
}

void detectaComando(char comando[], int diretorioAtual, char tabela[], short int* sair){
    if(strcmp(comando, "MKFILE") == 0){
        printf("Arquivo Criado!\n");
    }else if(strcmp(comando, "MKDIR") == 0){
        printf("Diretorio Criado!\n");
    }else if(strcmp(comando, "DIR") == 0){
        printf("Mostrar arquivos e diretorios\n");
    }else if(strcmp(comando, "CD") == 0){
        printf("Mudar o direorio\n");
    }else if(strcmp(comando, "RM") == 0){
        printf("Deletar arquivo/direorio\n");
    }else if(strstr(comando, "EDIT") != NULL){
        printf("Editar arquivo\n");
    }else if(strcmp(comando, "MOVE") == 0){
        printf("Mover diretorio/arquivo\n");
    }else if(strcmp(comando, "RENAME") == 0){
        printf("Renomear arquivo/diretorio\n");
    }else if(strcmp(comando, "SAIR") == 0){
        *sair = 1;
    }else{
        printf("Comando nao reconhecido.\n");
    }

}

void pegaTabela(char tabela[]){
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r");

    if (arq == NULL){ // Se houve erro na abertura{
        printf("Problemas na abertura do arquivo\n");
    }else{
        fseek(arq, sizeof(MetaDados)+1, SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        for(int i = 0; i < 256; i++){
            fread(&tabela[i], sizeof(char),1, arq);
        }
        fclose(arq);
    }

}


int primeiraPosicaoDisponivel(char tabela[]){
    int i = 0;

    while(i < 256 && tabela[i] != 0)
        i++;

    if(i == 256)
        i = 0;

    return i;
}

