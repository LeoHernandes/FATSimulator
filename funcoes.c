#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"



void inicializaArquivo(){
//Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
MetaDados metaDados = {256, 32000, 0, 1};
//ponteiro para o arquivo
FILE *arq;
    int bytesCluster = 0;
    char zero = 0;
    char valor255 = 255;
    NodoCluster root = {"root", ".TXT", 0, NULL};
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
        //La�o que a cria��o dos 256 clusters
        for(int m = 0; m < 255; m++){
            //La�o que controla a cria��o de um cluster com 32KB, todos com 0
            for(int i = 0; i < 32000; i++){
            fwrite(&zero, sizeof(char), 1, arq);
            }
            fwrite("\n", sizeof(char), 1, arq);
        }
        fseek(arq, sizeof(MetaDados), SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&valor255, sizeof(char), 1, arq);
        //Cria��o do cluster root
        fseek(arq, sizeof(MetaDados)+257, SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&root, sizeof(NodoCluster), 1, arq);

    }
    fclose(arq);
}

MetaDados pegaMetadados(){
    FILE *arq;
    arq = fopen("ArqDisco.bin", "rb+");
    MetaDados metaDados;
    if (arq == NULL){ // Se houve erro na abertura{
     printf("Problemas na abertura do arquivo\n");
     return;
     }else{
    fread(&metaDados, sizeof(MetaDados), 1, arq);
    }
    fclose(arq);
    return metaDados;
}

void detectaComando(char comando[], int diretorioAtual, char tabela[]){
    int retorno = 0;
    if(strstr(comando, "MKFILE") != NULL){
        printf("Arquivo Criado!\n");
    }else if(strstr(comando, "MKDIR") != NULL){
        printf("Diretorio Criado!\n");

    }else if(strstr(comando, "DIR") != NULL){
        printf("Mostrar arquivos e diretorios\n");

    }else if(strstr(comando, "CD") != NULL){
        printf("Mudar o direorio\n");

    }else if(strstr(comando, "RM") != NULL){
        printf("Deletar arquivo/direorio\n");
    }else if(strstr(comando, "EDIT") != NULL){
        printf("Editar arquivo\n");
    }else if(strstr(comando, "MOVE") != NULL){
        printf("Mover diretorio/arquivo\n");

    }else if(strstr(comando, "RENAME") != NULL){
        printf("Renomear arquivo/diretorio\n");
    }else if(strstr(comando, "Sair") != NULL){

    }else{
        printf("Comando nao reconhecido.\n");
    }

}

void pegaTabela(char tabela[]){
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r");
    if (arq == NULL){ // Se houve erro na abertura{
     printf("Problemas na abertura do arquivo\n");
     return;
     }else{
    fseek(arq, sizeof(MetaDados)+1, SEEK_SET);
    fwrite("\n", sizeof(char), 1, arq);
    for(int i = 0; i < 256; i++){
        fread(&tabela[i], sizeof(char),1, arq);
    }
    }
    fclose(arq);

}


int primeiraPosicaoDisponivel(char tabela[]){
    int controle = 0;
    for(int i = 0; i < 256; i++){
        if(tabela[i] == 0){
            return i;
        }
    }
    return 0;
}

