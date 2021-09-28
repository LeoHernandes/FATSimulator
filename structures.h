typedef struct TipoListaFilhos{
    struct TipoDiretorio* filho;
    struct TipoListaFilhos* prox;
}ListaFilhos;

typedef struct TipoDiretorio{
    char* nome;
    struct TipoDiretorio* pai;
    struct TipoListaFilhos* filhos;
}NodoDiretorio;
