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

    if(inicializaArquivo()){
        if(pegaTabela(tabela)){
            do{
                NodoCluster dir = pegaCluster(diretorioAtual);
                printf("%s:\\>", dir.nome);
                m = (char*) stringEntrada(stdin, TAMSTRING);
                fflush(stdin);
                detectaComando(m, p, tabela, &sair);
            }while(!sair);
        }else{
            printf("Nao foi possivel ler a tabela FAT\n");
        }
    }else{
        printf("Nao foi possivel inicializar o arquivo de disco");
    }

    return 0;
}
