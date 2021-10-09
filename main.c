/* CONSTANTES */
#define TAMSTRING 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
#include "funcoes.h"

int main(){
    char tabela[TAMTABELA], *m;
    int diretorioAtual = 0;
    int *p = &diretorioAtual;
    short int sair = 0;            //flag para manter o loop de escrita de comandos rodando
    NodoCluster dir;
    MetaDados metaDados;

    if(inicializaArquivo()){
        if(pegaMetadados(&metaDados) && pegaTabela(tabela, metaDados)){ //se foi possivel carregar a tabela e os metadados
            do{
                pegaCluster(diretorioAtual, &dir, metaDados);
                printf("%s:\\>", dir.nome);
                m = (char*) stringEntrada(stdin, TAMSTRING);
                fflush(stdin);
                detectaComando(m, p, tabela, &sair, metaDados);
            }while(!sair);
        }else{
            printf("Nao foi possivel ler as informacoes necessarias\n");
        }
    }else{
        printf("Nao foi possivel inicializar o arquivo de disco");
    }

    return 0;
}
