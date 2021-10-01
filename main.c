#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main()
{
    char comando[250];       //variavel que armazena o comando do usu�rio
    char tabela[2048];
    int diretorioAtual = 0;
    short int sair = 0;      //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;

    inicializaArquivo();
    pegaTabela(tabela);
    pegaMetadados(&metaDados);

    //Laco de execu��o
    printf("%d", primeiraPosicaoDisponivel(tabela));

    do{
        printf(":\\>");
        scanf("%s", comando);
        fflush(stdin);
        detectaComando(comando, diretorioAtual, tabela, &sair);
    }while(!sair);

    return 0;
}
