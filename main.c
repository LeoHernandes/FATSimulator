/* CONSTANTES */
#define TAMSTRING 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main(){
    char tabela[TAMTABELA], *input;
    int diretorioAtual = 0;
    int *ponteiroDir = &diretorioAtual;
    short int sair = 0;            //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;
    NodoCluster dir;

    if(inicializaArquivo()){
        if(pegaMetadados(&metaDados) && pegaTabela(tabela, metaDados)){ //se foi possivel carregar a tabela e os metadados
            do{
                pegaCluster(diretorioAtual, &dir, metaDados);
                printf("%s:\\>", dir.nome);
                input = (char*) stringEntrada(stdin, TAMSTRING);
                fflush(stdin);
                detectaComando(input, ponteiroDir, tabela, &sair, metaDados);
            }while(!sair);
        }else{
            printf("Nao foi possivel ler as informacoes necessarias\n");
        }
    }else{
        printf("Nao foi possivel inicializar o arquivo de disco");
    }

    return 0;
}
