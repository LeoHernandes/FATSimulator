#include "estruturas.h"

void leTexto(char texto[], int tamanhoTexto);

void inicializaArquivo();

void detectaComando(char comando[], int diretorioAtual, char tabela[], short int* sair);

int pegaMetadados(MetaDados* metaDados);

void pegaTabela(char tabela[]);

int primeiraPosicaoDisponivel(char tabela[]);
