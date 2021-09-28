/* Lista simplesmente encadeada para armazenar todos os filhos (pastas e arquivos) de um nodo */
typedef struct TipoListaFilhos{
    struct TipoDiretorio* filho;
    struct TipoListaFilhos* prox;
}ListaFilhos;

/* �rvore armazenando distribui��o de arquivos e pastas num cluster */
typedef struct TipoCluster{
    char* nome;
    char* extensao;
    struct TipoDiretorio* pai;
    struct TipoListaFilhos* filhos;
}NodoCluster;

/* Estrutura com os metadados necess�rios */
typedef struct TipoMetaDados{
    short int tamIndice;
    short int tamCluster; //em KB
    short int initIndice; //em byte
    short int initCluster; //em byte
}MetaDados;
