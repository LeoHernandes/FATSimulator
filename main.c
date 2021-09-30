#include <stdio.h>
#include <stdlib.h>
//#include "estruturas.h"
#include "funcoes.h"
int main()
{
    char comando[250];
    char tabela[2048];
    int diretorioAtual = 0;

    inicializaArquivo();
    MetaDados metaDados = pegaMetadados();
    pegaTabela(tabela);

    //Laço de execução
    printf("%d", primeiraPosicaoDisponivel(tabela));
    while(!strstr(comando, "SAIR")){
    printf(":\>");
    scanf("%s", &comando);
    fflush(stdin);
    detectaComando(comando, diretorioAtual, tabela);

    }
    return 0;
}
