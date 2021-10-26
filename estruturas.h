/*------------*/
/* CONSTANTES */
/*------------*/

#define TAMCLUSTER  32
#define TAMTABELA   254
#define INITABELA   8
#define INITCLUSTER 262
#define REALLOCSIZE 16
#define TAMNOME     32
#define TAMEXTENSAO 3

/*------------*/
/* ESTRUTURAS */
/*------------*/

/* Lista simplesmente encadeada para armazenar os diretorios acessados em sequencia */
typedef struct TipoListaStrings{
    char *comando;
    struct TipoListaStrings *prox;
}ListaStrings;

/* Árvore armazenando distribuição de arquivos e pastas num cluster */
typedef struct TipoCluster{
    char nome[TAMNOME + 1];
    char extensao[TAMEXTENSAO + 1];
    char pai;
    char marcador;
}NodoCluster;

/* Estrutura com os metadados necessários */
typedef struct TipoMetaDados{
    short int tamIndice;
    short int tamCluster;  //em KB
    short int initIndice;  //em byte
    short int initCluster; //em byte
}MetaDados;
