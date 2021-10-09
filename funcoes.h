#include "estruturas.h"

void leTexto(char texto[], int tamanhoTexto);

int inicializaArquivo();

void detectaComando(char comando[], int *dirAtual, char tabela[], short int* sair);

int pegaMetadados(MetaDados* metaDados);

int pegaTabela(char tabela[]);

int primeiraPosicaoDisponivel(char tabela[]);

int pegaCluster(int ponteiroCluster, NodoCluster* cluster);

char* stringEntrada(FILE* fp, size_t tamanho);
