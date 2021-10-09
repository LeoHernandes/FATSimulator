#include "estruturas.h"

int inicializaArquivo();

void detectaComando(char comando[], int *dirAtual, char tabela[], short int* sair, MetaDados metaDados);

int pegaMetadados(MetaDados* metaDados);

int pegaTabela(char tabela[], MetaDados metaDados);

int primeiraPosicaoDisponivel(char tabela[], MetaDados metaDados);

int pegaCluster(int ponteiroCluster, NodoCluster* cluster, MetaDados metaDados);

char* stringEntrada(FILE* fp, size_t tamanho);
