/*------------*/
/* CONSTANTES */
/*------------*/

#define TAMCLUSTER 32000
#define TAMTABELA  256

/*------------*/
/* ESTRUTURAS */
/*------------*/

/* Lista simplesmente encadeada para armazenar todos os filhos (pastas e arquivos) de um nodo */
typedef struct TipoListaFilhos{
    char filho;
    struct TipoListaFilhos* prox;
}ListaFilhos;

/* Árvore armazenando distribuição de arquivos e pastas num cluster */
typedef struct TipoCluster{
    char nome[50];
    char extensao[3];
    char inicio;
    char pai;
    struct TipoListaFilhos *filhos;
}NodoCluster;

/* Estrutura com os metadados necessários */
typedef struct TipoMetaDados{
    short int tamIndice;
    short int tamCluster; //em KB
    short int initIndice; //em byte
    short int initCluster; //em byte
}MetaDados;
