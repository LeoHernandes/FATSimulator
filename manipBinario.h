int inicializaBinario();

int pegaMetadados(MetaDados* metaDados);

int pegaCluster(char ponteiroCluster, NodoCluster* cluster, MetaDados metaDados);

int primeiraPosicaoDisponivel(char *clusterDisponivel, MetaDados metaDados);

int alteraTabelaFat(char valor, char ponteiroCluster, MetaDados metaDados);

int adicionaFilho(char pai, char filho, MetaDados metaDados);

int removeFilho(char pai, char filho, MetaDados metaDados);

int insereNodoCluster(NodoCluster nodoCluster, char ponteiroCluster, MetaDados metaDados);

int modificaNodoCluster(NodoCluster nodoCluster, char ponteiroCluster, MetaDados metaDados);

int encontraCaminho(ListaStrings *listaComandos, char diretorioAtual, char subdir, MetaDados metaDados);

int removeArquivo(char clusterArquivo, MetaDados metaDados);
