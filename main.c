/* CONSTANTES */
#define TAMSTRING 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main()
{
    char comando[TAMSTRING];       //variavel que armazena o comando do usuário
    char tabela[TAMTABELA];
    int diretorioAtual = 0;
    short int sair = 0;            //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;

    inicializaArquivo();
    pegaTabela(tabela);
    pegaMetadados(&metaDados);

    do{
        printf(":\\>");
        leTexto(comando, TAMSTRING);
        fflush(stdin);
        detectaComando(comando, diretorioAtual, tabela, &sair);
    }while(!sair);

    return 0;
}
