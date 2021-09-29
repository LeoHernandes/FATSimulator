#include <stdio.h>
#include <stdlib.h>
#include "estruturas.h"
#include "funcoes.h"
int main()
{
    int diretorioAtual = 0;
    char comando[250];
    int sair = 0;
    inicializaArquivo();
    while(!strstr(comando, "SAIR")){
    printf(":\>");
    scanf("%s", &comando);
    fflush(stdin);
    detectaComando(comando, diretorioAtual);

    }
    return 0;
}
