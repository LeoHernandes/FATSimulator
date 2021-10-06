#include "estruturas.h"

void leTexto(char texto[], int tamanhoTexto);

void inicializaArquivo();

void detectaComando(char comando[], int *dirAtual, char tabela[], short int* sair);

int pegaMetadados(MetaDados* metaDados);

int pegaTabela(char tabela[]);

int primeiraPosicaoDisponivel(char tabela[]);

NodoCluster pegaCluster(int ponteiroCluster);

char* stringEntrada(FILE* fp, size_t tamanho);
