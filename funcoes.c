#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"
#include "estruturas.h"


void inicializaArquivo(){
//Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
MetaDados metaDados = {256, 32000, 0, 4};
//ponteiro para o arquivo
FILE *arq;
    arq = fopen("ArqDisco.bin", "w+");
    if (arq == NULL){
        printf("Problemas na criacao do arquivo\n");
        return;
    }else{
        //fwrite(&metaDados, sizeof(MetaDados), 1, arq);
        fprintf(arq, "%d%d%d%d", metaDados.tamIndice, metaDados.tamCluster, metaDados.initIndice, metaDados.initCluster);
        fwrite("\n", sizeof(char), 1, arq);
        //Laço que a criação dos 256 clusters
        for(int m = 0; m < 256; m++){
            //Laço que controla a criação de um cluster com 32KB
            for(int i = 0; i < (sizeof(char)*32000); i++){
            fputs("0", arq);
            }
            fwrite("\n", sizeof(char), 1, arq);
        }
    }
    fclose(arq);
}


void detectaComando(char comando[], int diretorioAtual){
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
        printf("Comando não reconhecido.\n");
    }

}
