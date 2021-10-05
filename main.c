/* CONSTANTES */
#define TAMSTRING 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main()
{
    char tabela[TAMTABELA], *m;
    int diretorioAtual = 0;
    int *p = &diretorioAtual;
    short int sair = 0;            //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;


    inicializaArquivo();
    pegaTabela(tabela);
    pegaMetadados(&metaDados);

    do{
    NodoCluster dir = pegaCluster(diretorioAtual);
        printf("%s:\\>", dir.nome);
        m = (char*) inputString(stdin, 10);
        fflush(stdin);
        detectaComando(m, p, tabela, &sair);

    }while(!sair);

    return 0;
}
