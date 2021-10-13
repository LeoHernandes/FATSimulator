/* CONSTANTES */
#define TAMSTRING 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main(){
    char tabela[TAMTABELA], *input, *caminho;
    char diretorioAtual = 0;
    char *ponteiroDir = &diretorioAtual;
    short int sair = 0;            //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;

    if(inicializaArquivo()){
        if(pegaMetadados(&metaDados) && pegaTabela(tabela, metaDados)){ //se foi possivel carregar a tabela e os metadados
            inicializaCaminho(&caminho);
            do{
                printf("%s:\\>", caminho);
                input = (char*) stringEntrada(stdin, TAMSTRING);
                fflush(stdin);
                detectaComando(input, &caminho, ponteiroDir, tabela, &sair, metaDados);
            }while(!sair);
        }else{
            printf("Nao foi possivel ler as informacoes necessarias\n");
        }
    }else{
        printf("Nao foi possivel inicializar o arquivo de disco");
    }

    return 0;
}
