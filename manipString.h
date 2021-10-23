void inicializaCaminho(char** caminho);

char* stringEntrada(FILE* fp, size_t tamanho);

void pegaOperacaoNome(char comando[], char** operacao, char** nome, char** resto);

void voltaCaminho(char** caminho);

void avancaCaminho(char **caminho, ListaStrings *listaComandos);

void reconstroiCaminho(char **caminho, ListaStrings *listaComandos);

