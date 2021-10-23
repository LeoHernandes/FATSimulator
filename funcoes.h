#include "estruturas.h"

int inicializaBinario();

void inicializaCaminho(char** caminho);

void detectaComando(char comando[], char** caminho, char *dirAtual, short int* sair, MetaDados metaDados);

int pegaMetadados(MetaDados* metaDados);

int primeiraPosicaoDisponivel(char tabela[], MetaDados metaDados);

char* stringEntrada(FILE* fp, size_t tamanho);
