#include "estruturas.h"

int inicializaArquivo();

void inicializaCaminho(char** caminho);

void detectaComando(char comando[], char** caminho, int *dirAtual, char tabela[], short int* sair, MetaDados metaDados);

int pegaMetadados(MetaDados* metaDados);

int pegaTabela(char tabela[], MetaDados metaDados);

int primeiraPosicaoDisponivel(char tabela[], MetaDados metaDados);

char* stringEntrada(FILE* fp, size_t tamanho);

int pegaCluster(int ponteiroCluster, NodoCluster* cluster, MetaDados metaDados);
