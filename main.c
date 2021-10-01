/* CONSTANTES */
#define TAMSTRING = 250

/* BIBLIOTECAS */
#include <stdio.h>
#include <stdlib.h>
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

int main()
{
    char comando[TAMSTRING];       //variavel que armazena o comando do usuário
    char tabela[2048];
    int diretorioAtual = 0;
    short int sair = 0;      //flag para manter o loop de escrita de comandos rodando
    MetaDados metaDados;

    inicializaArquivo();
    pegaTabela(tabela);
    pegaMetadados(&metaDados);

    //Laco de execução
    printf("%d", primeiraPosicaoDisponivel(tabela));

    do{
        printf(":\\>");
        leTexto(comando, TAMSTRING);
        fflush(stdin);
        detectaComando(comando, diretorioAtual, tabela, &sair);
    }while(!sair);

    return 0;
}
