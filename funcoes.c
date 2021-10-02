#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"

void leTexto(char texto[], int tamanhoTexto)
//funcao que recebe string e controla o tamanho dela
{
    char dummy[tamanhoTexto + 1]; // com um caractere a mais do que o texto
    fflush(stdin);
    fgets(dummy, sizeof(dummy), stdin);
    // O último caractere tem que ser '\n' para estar correto:
    while(dummy[strlen(dummy) -1] != '\n')
    {
        printf("\nMaximo de %d caracteres, digite novamente\n-> ", tamanhoTexto - 1);
        fflush(stdin);
        fgets(dummy, sizeof(dummy), stdin); // le caracteres novamente
    }
    dummy[strlen(dummy) - 1]= '\0'; // sempre precisa substituir o '\n'
    strcpy(texto, dummy); // transfere conteudo digitado sem o '\n'
}

void inicializaArquivo(){
    MetaDados metaDados = {TAMTABELA, TAMCLUSTER, 0, 1}; //Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
    FILE *arq;                                //ponteiro para o arquivo
    //int bytesCluster = 0;
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

        for(int j = 0; j < TAMTABELA; j++){
            fwrite(&zero, sizeof(char), 1, arq);
        }
        fwrite("\n", sizeof(char), 1, arq);

        //Laco que a criacao dos 256 clusters
        for(int m = 0; m < TAMTABELA; m++){
            //Laco que controla a criacao de um cluster com 32KB, todos com 0
            for(int i = 0; i < TAMCLUSTER; i++){
            fwrite(&zero, sizeof(char), 1, arq);
            }
            fwrite("\n", sizeof(char), 1, arq);
        }

        fseek(arq, sizeof(MetaDados), SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&valor255, sizeof(char), 1, arq);

        //Criacao do cluster root
        fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 , SEEK_SET);
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

void pegaOperacaoNome(char comando[], char** operacao, char** nome){
/* Dado um comando do usuário, pega a operacao e o possível nome do diretório ou arquivo fornecido */
    *operacao = strtok(comando, " ");
    *nome = strtok(NULL, " ");

}

void pegaTabela(char tabela[]){
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r");

    if (arq == NULL){ // Se houve erro na abertura{
        printf("Problemas na abertura do arquivo\n");
    }else{
        fseek(arq, sizeof(MetaDados)+1, SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        for(int i = 0; i < TAMTABELA; i++){
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

int mkDir(char* nome, int clusterPai, int cluster, char tabela[]){
/* Cria um diretório no primeiro cluster disponível dado o diretório atual
 * Retorna 1 caso seja realizado com sucesso
 * Retorna 0 caso a criação falhe */
    //Ponteiro para o arquivo
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");
    int i = 0;
    NodoCluster novo = {nome, "", 0, NULL};

    //Marca o primeiro cluster dispnível como ocupado e escreve no arquivo(talvez vire uma função)
    tabela[cluster] = 255;
    fseek(arq, sizeof(MetaDados) + 1, SEEK_SET);
    fwrite(tabela, sizeof(char) * TAMTABELA, 1, arq);

    //Posiciona o cursor na linha do primeiro cluster disponível que será preenchido pelo novo diretório
    fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 + ((TAMCLUSTER + 1) * cluster), SEEK_SET);
    fwrite("\n", sizeof(char), 1, arq);

    //Escreve o cluster no arquivo
    i = fwrite(&novo, sizeof(NodoCluster), 1, arq);
    if(i == 0){
        fclose(arq);
        return 0;
    }
    fclose(arq);
    return 1;
}

void detectaComando(char comando[], int diretorioAtual, char tabela[], short int* sair){
/* Detecta os possíveis comandos exigidas pelo usuário,
 * separando a operação do possível nome de diretórios e arquivos */
    char *operacao = NULL, *nome = NULL;


    pegaOperacaoNome(comando, &operacao, &nome);

    if(strcmp(operacao, "MKFILE") == 0){
        printf("Arquivo Criado!\n");
    }else if(strstr(operacao, "MKDIR") != NULL){
        if(nome != NULL && mkDir(nome, diretorioAtual, primeiraPosicaoDisponivel(tabela), tabela)){
            printf("Diretorio Criado!\n");
        }else{
            printf("Erro ao criar o diretorio\n");
        }
    }else if(strstr(operacao, "DIR") != NULL){
        printf("Mostrar arquivos e diretorios\n");
    }else if(strcmp(operacao, "CD") == 0){
        printf("Mudar o direorio\n");
    }else if(strcmp(operacao, "RM") == 0){
        printf("Deletar arquivo/direorio\n");
    }else if(strstr(operacao, "EDIT") != NULL){
        printf("Editar arquivo\n");
    }else if(strcmp(operacao, "MOVE") == 0){
        printf("Mover diretorio/arquivo\n");
    }else if(strcmp(operacao, "RENAME") == 0){
        printf("Renomear arquivo/diretorio\n");
    }else if(strcmp(operacao, "SAIR") == 0){
        *sair = 1;
    }else{
        printf("Comando nao reconhecido.\n");
    }
}
