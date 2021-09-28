/* Lista simplesmente encadeada para armazenar todos os filhos (pastas e arquivos) de um nodo */
typedef struct TipoListaFilhos{
    struct TipoDiretorio* filho;
    struct TipoListaFilhos* prox;
}ListaFilhos;

/* Árvore armazenando distribuição de arquivos e pastas num cluster */
typedef struct TipoCluster{
    char* nome;
    char* extensao;
    struct TipoDiretorio* pai;
    struct TipoListaFilhos* filhos;
}NodoCluster;
