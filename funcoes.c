#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listaStrings.h"
#include "manipString.h"

/*****************************************************************************************************/
/*                                   FUNCOES AUXILIARES                                              */
/*****************************************************************************************************/

int inicializaBinario(){
/* Inicializa o arquivo que simula o disco caso ele não exista ainda.
 * Cria uma área de metadados de 8 bytes
 * Cria uma áre de 256 bytes, que armazena a tabela fat
 * Inicializa o cluster "root", tornando-o o principal.
 * Inicializa os 255 clusters restantes de 32KB cada
 * Retorno:
 *      1 caso seja inicializado com sucesso
 *      0 caso haja alguma falha
 */
    int i, j;
    char zero = 0, FF = 255;
    FILE *arq;                                                             //ponteiro para o arquivo
    MetaDados metaDados = {TAMTABELA, TAMCLUSTER, INITABELA, INITCLUSTER}; //Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
    NodoCluster root = {"root", "", 'a', '*'};

    if((arq = fopen("ArqDisco.bin", "rb")) == NULL){        //se o arquivo não existe ainda
        if((arq = fopen("ArqDisco.bin", "a+b")) == NULL){  //se houve problema na abertura do arquivo
            printf("Erro na criacao do arquivo!\n");
            return 0;
        }

        //criacao dos metadados
        fwrite(&metaDados, sizeof(MetaDados), 1, arq);      //escreve os metadados

        //criacao da tabela FAT
        fwrite(&FF, sizeof(char), 1, arq);                  //preenche a primeira posição da tabela FAT para o diretório root
        for(i = 0; i < TAMTABELA - 1; i++)
            fwrite(&zero, sizeof(char), 1, arq);            //preenche o restante da tabela

        //Laco que faz a criacao dos 256 clusters
        fwrite(&root, sizeof(NodoCluster), 1, arq);         //armazena o root no primeiro cluster
        for(i = 0; i < ((TAMCLUSTER * 1000) - sizeof(NodoCluster)); i++)
            fwrite(&zero, sizeof(char), 1, arq);

        for(i = 0; i < TAMTABELA - 1; i++){
            for(j = 0; j < (TAMCLUSTER * 1000); j++)
                fwrite(&zero, sizeof(char), 1, arq);   //armazena os outros 255 clusters
        }

        fclose(arq);
        if(ferror(arq)){  //se houver erro em qualquer parte da escrita
            printf("Erro no preenchimento do arquivo!");
            return 0;
        }
    }
    return 1;
}

int pegaMetadados(MetaDados* metaDados){
/* Carrega os metadados do arquivo de disco se possível
 * Entrada:
 *      Ponteiro para estrutura de metadados para preencher
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 se a operação for feita com sucesso
 *      0 se a operação falhar
 */
    FILE *arq;
    arq = fopen("ArqDisco.bin", "rb+");

    //Se houve erro na abertura
    if (arq == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //se houve erro na leitura
    if(fread(metaDados, sizeof(MetaDados), 1, arq) != 1){
        printf("Erro na leitura do arquivo\n");
        fclose(arq);
        return 0;
    }
    fclose(arq);
    return 1;
}

int pegaCluster(char ponteiroCluster, NodoCluster* cluster, MetaDados metaDados){
/* Dado um inteiro, que representa o ponteiro de um cluster, retorna o cluster para o qual o ponteiro aponta.
 * Entrada:
 *      Char que representa o ponteiro para a linha em que o cluster está
 *      Ponteiro para NodoCluster para preenche-lo com o cluster apontado
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1, caso a leitura seja feito com sucesso
 *      0, caso haja falha na leitura
 */
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * ponteiroCluster), SEEK_SET);
    fread(cluster, sizeof(NodoCluster), 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro a leitura do cluster no arquivo\n");
        return 0;
    }

    return 1;
}

int primeiraPosicaoDisponivel(char *clusterDisponivel, MetaDados metaDados){
/* Retorna o ponteiro(linha) do pirmeiro cluster disponível
 * Entrada:
 *      Ponteiro para char que vai armazenar o ponteiro do cluster disponivel
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso tenha achado posicao diponivel
 *      0 caso nao tenha posicao disponivel ou houve problema com o arquivo
 */
    int aux = 255;
    short int iterador = -1;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo!\n");
        return 0;
    }

    fseek(arq, metaDados.initIndice, SEEK_SET);  //vai ate a tabela FAT

    while(iterador < metaDados.tamIndice && aux != 0 && aux != 254){
        aux = fgetc(arq);
        iterador++;                                  //itera ate achar posicao disponivel ou chegar no final da tabela
    }
    fclose(arq);
    if(!ferror(arq)){                               //se nao houve problema com o arquivo
        if(iterador == metaDados.tamIndice)         //e se chegou no final da tabela
            return 0;                               //sinaliza que nao ha posicao disponivel

        *clusterDisponivel = iterador;              //caso contrario, devolve o cluster disponivel
        return 1;                                   //sinaliza que houve sucesso
    }
    printf("Erro na leitura do arquivo!\n");
    return 0;
}

int alteraTabelaFat(char valor, char ponteiroCluster, MetaDados metaDados){
/* Atualiza a tabela no arquivo dado seu indice e o novo valor
 * Entrada:
 *      Char com o valor a ser escrito na tabela
 *      Char que representa o cluster a ser atualizado na tabela
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso escreva com sucesso.
 *      0 caso haja falha na escrita.
 */
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo!\n");
        return 0;
    }

    fseek(arq, metaDados.initIndice + ponteiroCluster, SEEK_SET);      //vai ate o indice correto da tabela FAT
    fwrite(&valor, sizeof(char), 1, arq);                              //sobrescreve com o novo valor

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita do arquivo!\n");
        return 0;
    }
    return 1;
}

int adicionaFilho(char pai, char filho, MetaDados metaDados){
/* Insere um ponteiro, da tabela fat, na lista de filhos do cluster "pai" na primeira posicao disponivel
 * Entrada:
 *      char, que representa o ponteiro(linha) do cluster pai
 *      char, que representa o ponteiro(linha) do cluster filho.
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1, caso haja sucesso na escrita da nova lista de filhos
 *      0, caso falhe na escrita
 */
    int aux = 0;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro do arquivo no cluster onde será inserido um filho
    fseek(arq, metaDados.initCluster +((metaDados.tamCluster * 1000) * pai), SEEK_SET);

    //Percorre todos os caracteres do cluster até encontrar o marcador '*'
    while(aux != '*'){
        aux = fgetc(arq);
    }

    //Percorre, a partir do marcador '*', até encontrar o primeira posição livre para inserir o ponteiro do filho
    while(aux != 0 && aux != 254){
        aux = fgetc(arq);
    }

    //insere o ponteiro do filho na lista de filhos do pai
    fseek(arq, -1, SEEK_CUR);
    fwrite(&filho, sizeof(char), 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita do arquivo\n");
        return 0;
    }
    return 1;
}

int removeFilho(char pai, char filho, MetaDados metaDados){
/* Retira um ponteiro, da tabela fat, da lista de filhos do cluster "pai" substituindo por 'B'
 * Entrada:
 *      char, que representa o ponteiro(linha) do cluster pai
 *      char, que representa o ponteiro(linha) do cluster filho.
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1, caso haja sucesso na escrita da nova lista de filhos
 *      0, caso falhe na escrita
 */
    char aux = 0, B = 254;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro do arquivo no cluster onde será removido um filho
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * pai), SEEK_SET);

    //Percorre todos os caracteres do cluster até encontrar o marcador '*'
    while(aux != '*'){
        aux = fgetc(arq);
    }

    //Percorre, a partir do marcador '*', até encontrar o ponteiro do filho para remover
    while(aux != filho){
        aux = fgetc(arq);
    }

    fseek(arq, -1, SEEK_CUR);
    fwrite(&B, sizeof(char), 1, arq);
    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita do arquivo\n");
        return 0;
    }
    return 1;

}

int insereNodoCluster(NodoCluster nodoCluster, char ponteiroCluster, MetaDados metaDados){
/* Escreve um novo cluster dado o ponteiro pro cluster disponivel
 * Entrada:
 *      Estrutura NodoCluster que vai ser gravada
 *      Char representando o ponteiro para a posicao disponivel
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso haja sucesso na gravacao
 *      0 caso haja falha
 */
    int i;
    char zero = 0;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro do arquivo no cluster onde será inserido um filho
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * ponteiroCluster), SEEK_SET);

    fwrite(&nodoCluster, sizeof(NodoCluster), 1, arq);
    for(i = 0; i < (metaDados.tamCluster * 1000 - sizeof(NodoCluster)); i++)
        fwrite(&zero, sizeof(char), 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita dos clusters\n");
        return 0;
    }
    return 1;
}

int modificaNodoCluster(NodoCluster nodoCluster, char ponteiroCluster, MetaDados metaDados){
/* Altera um cluster existente dado o ponteiro pro cluster disponivel
 * Entrada:
 *      Estrutura NodoCluster que vai ser gravada
 *      Char representando o ponteiro para a posicao disponivel
 * Retorno:
 *      1 caso haja sucesso na gravacao
 *      0 caso haja falha
 */
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro do arquivo no cluster onde será inserido um filho
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * ponteiroCluster), SEEK_SET);

    if(fwrite(&nodoCluster, sizeof(NodoCluster), 1, arq)){      //escreve cluster modificado
        fclose(arq);
        return 1;
    }else{
        fclose(arq);
        return 0;
    }
}

int encontraCaminho(ListaStrings *listaComandos, char diretorioAtual, char subdir, MetaDados metaDados){
/* Dado o diretorio atual e um caminho, testa se o caminho existe e seta o diretorio atual para o caminho final
 * Entrada:
 *      Uma lista de strings, que representa o caminho de um diretório/arquivo
 *      Inteiro que representa o diretório em que foi realizado a operação
 *      Char que representa um subdiretorio para a funcao ser chamada recursivamente
 *      MetaDados metadados do disco que serão utilizados para realizar as operações com o fseek.
 * Retorno:
 *      Inteiro positivo ou zero caso o caminho tenha sido encontrado
 *      -1 caso o caminho nao tenha sido encontrado
 */
    int aux = 0;
    long i = 0;
    FILE *arq;
    NodoCluster dir;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return -1;
    }

    //Posiciona o ponteiro no cluster atual
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * subdir), SEEK_SET);

    //Encontra o marcador de inicio da lista de filhos
    while(aux != '*'){
        aux = fgetc(arq);
    }
    aux = fgetc(arq);

    if(aux == 0){                           //Se o primeiro filho for igual a zero, a pasta pai não tem filho
        fclose(arq);                        //Fecha o arquivo e retorna 0, indicando erro.
        return -1;
    }else{                                  //Se não, busca se há uma pasta/arquivo com o nome solicitado no caminho indicado
        while(aux != 0){
            if(aux != 254){                 //Se nao for um filho removido
                i = ftell(arq);              //guarda a posição atual na lista de filhos
                if(!pegaCluster(aux, &dir, metaDados)){    //pega o cluster indicado pelo filho
                    printf("Erro na leitura do cluster\n");
                    fclose(arq);
                    return -1;
                }
                //Verifica se o nome da pasta/arquivo encontrada na lista de filhos possui o mesmo nome do caminho solicitado
                if(!strcmp(strtok(listaComandos->comando, "."), dir.nome)){
                    //Se o caminho nao possui extensao e cluster tambem nao possui
                    if(!strcmp(dir.extensao, "")){
                        fclose(arq);
                        if(listaComandos->prox == NULL){   //Se nao tem mais caminho, retorna o cluster atual
                            return aux;
                        }else{
                            return encontraCaminho(listaComandos->prox, diretorioAtual, aux, metaDados); //senao chama a função recursivamente
                        }
                    //Se o caminho possui extensao e o cluster tambem
                    }else if(strcmp(dir.extensao, "") != 0){
                        fclose(arq);
                        if(listaComandos->prox == NULL){   //Se nao tem mais caminho, retorna o cluster atual
                            return aux;
                        }else{                             //Se existe mais caminhos, encerra a funcao com erro
                            printf("Tentativa de acesso a um filho de um arquivo\n");
                            return -1;
                        }
                    }
                }
                fseek(arq, i, SEEK_SET);
            }
            aux = fgetc(arq);
        }
        fclose(arq);
        if(ferror(arq))
            printf("Erro na leitura do arquivo\n");
        return -1;
    }
}

int removeArquivo(char clusterArquivo, MetaDados metaDados){
/* Dado um cluster que e' o local inicial de um arquivo .txt, marca como removido na tabela FAT
 * todos os possiveis endereços que sao ocupados pelo arquivo
 * Entrada:
 *      Char que representa o ponteiro para o cluster a ser removido
 *      Metadados para auxliar na busca dentro do arquivo binario
 * Retorno:
 *      1 caso haja sucesso na remocao
 *      0 caso haja alguma falha
 */
    char aux, FE = 254;
    FILE* arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    fseek(arq, sizeof(char) * (metaDados.initIndice + clusterArquivo), SEEK_SET); //vai ate o indice a ser removido

    do{
        aux = fgetc(arq);
        fseek(arq, -1, SEEK_CUR);
        fwrite(&FE, sizeof(char), 1, arq);  //marcamos como apagado
        fseek(arq, sizeof(char) * (metaDados.initIndice + aux), SEEK_SET); //vai ate o proximo indice sequencial ao arquivo
    }while(aux != -1 && aux != 254 && aux != 0); //enquanto nao chegamos no final do arquivo ou estamos no caminho errado

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na remocao do arquivo!\n");
        return 0;
    }
    return 1;
}

/*****************************************************************************************************/
/*                                   FUNCOES PRINCIPAIS                                              */
/*****************************************************************************************************/

int mkDir(char* nome, char clusterPai, char cluster, MetaDados metaDados){
/* Cria um diretorio no primeiro cluster disponível dado o diretorio atual
 * Entrada:
 *      String com o nome do diretorio dado pelo usuario
 *      Ponteiro (char) para o pai do diretório a ser criado
 *      Ponteiro (char) para o cluster diponível
 *      Tabela FAT que armazena os ponteiros dos discos
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso seja realizado com sucesso
 *      0 caso a criação falhe
 */
    NodoCluster novo = {"", "", 'a', '*'};
    ListaStrings *lsNome;
    lsNome = NULL;

    if(strcmp(nome, "") != 0){
        lsNome = inserirLSEStrings(lsNome, nome);
        if(encontraCaminho(lsNome, clusterPai, clusterPai, metaDados) != -1){
            apagaLSE(lsNome);
            printf("Ja existe um diretorio com esse nome.\n");
            return 0;
        }
         apagaLSE(lsNome);
    }
    //Cria o novo Cluster
    strcpy(novo.nome, nome);
    novo.pai = clusterPai;


    if(!insereNodoCluster(novo, cluster, metaDados)){   //Se houve erro ao inserir o novo cluster
        printf("Erro ao escrever o novo cluster\n");
        return 0;
    }

    if(!adicionaFilho(clusterPai, cluster, metaDados)){ //se houve erro ao adicionar um filho na lista de filhos
        printf("Erro ao atualizar o diretorio pai\n");
        return 0;
    }

    if(!alteraTabelaFat(255, cluster, metaDados)){      //Marca o primeiro cluster disponivel como ocupado e escreve no arquivo
        printf("Erro ao atualizar a tabela\n");
        return 0;
    }

    return 1;
}

int dir(char pai, MetaDados metaDados){
/* Lista todos os subdiretorios e arquivos dado um diretorio principal
 * Entrada:
 *      Char, que representa o ponteiro da pasta que queremos exibir os arquivos e diretórios
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1, caso consigamos ler com sucesso do arquivo
 *      0, caso haja falha na leitura
 */
    long i = 0;
    int temFilho = 0;
    int aux = 0;
    FILE *arq;
    NodoCluster cluster;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o cursor do arquivo no cluster que desejamos listar as informações
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * pai), SEEK_SET);

    //percorre o cluster até encontrar o marcador '*'
    while(aux != '*'){
        aux = fgetc(arq);
    }

    aux = fgetc(arq);

    if(pai != 0)
        printf("..\n");            //printa apenas uma vez os dois pontos caso nao esteja no root

   //Percorre a lista de filhos até o final e printa os nomes na tela.
        while(aux != 0){
            if(aux != 254){ //se nao for um filho removido
                temFilho = 1;
                //guarda a posição do filho atual
                i = ftell(arq);
                //Pega o cluster que está na lista de filhos
                if(!pegaCluster(aux, &cluster, metaDados)){
                    fclose(arq);
                    return 0;
                }
                printf("%s", cluster.nome);
                //se tem extensao, printa tambem
                if(strcmp(cluster.extensao, "")) printf(".%s", cluster.extensao);
                //retorna para a lista de filhos
                printf("\n");
                fseek(arq, i, SEEK_SET);
            }
            aux = fgetc(arq);
        }
    fclose(arq);
    if(temFilho == 0){
        printf("<vazio>\n");
    }
    if(ferror(arq)){
        printf("Nao foi possivel ler todos os subdiretorios\n");
        return 0;
    }
    return 1;
}

int cd(ListaStrings *listaComandos, char *diretorioAtual, MetaDados metaDados){
/* Muda o diretorio atual para algum subdiretorio
 * Entrada:
 *      Uma lista de strings, que representa o caminho de um diretório/arquivo
 *      Inteiro que representa o diretório em que foi realizado a operação
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso o caminho tenha sido encontrado
 *      0 caso o caminho nao tenha sido encontrado
 */
    int ponteiroCluster;
    NodoCluster dir;
    ListaStrings *aux;

    //Verifica se a lista de comando é vazia
    if(listaComandos == NULL){
        return 0;
    }
    pegaCluster(*diretorioAtual, &dir, metaDados);

    if(strcmp(listaComandos->comando, "..") == 0){     //Verifica se o comando digitado foi o de "voltar"
        if(*diretorioAtual != 0){                      //Se o diretório atual é diferente do root, volta para a pasta "pai"
            *diretorioAtual = dir.pai;
            return 1;
        }                                              //Senao, fecha o arquivo e retorna 1, indicando erro.
        return 0;
    }else if(strcmp(listaComandos->comando, "root") == 0){ //se o comando comecar com 'root'
        *diretorioAtual = 0;                               //coloca o usuario no primeiro diretorio
        aux = listaComandos->prox;                         //comeca a lista de comandos pelo segundo comando
    }else{
        aux = listaComandos;                               //caso contrario apenas navega pelo caminho dado
    }
    //Se nao for o comando '..', percorre recursivamente pelas pastas
    if(aux != NULL){
        ponteiroCluster = encontraCaminho(aux, *diretorioAtual, *diretorioAtual, metaDados);
        if(ponteiroCluster != -1){
            pegaCluster(ponteiroCluster, &dir, metaDados);
            if(!strcmp(dir.extensao, "")){  //se nao for um arquivo
                *diretorioAtual = ponteiroCluster;
                return 1;
            }else{
                printf("Nao e possivel entrar num arquivo\n");
                return 0;
            }
        }
        return 0;
    }else{          //se o comando era apenas /root/, volta para o primeiro diretorio
        *diretorioAtual = 0;
        return 1;
    }

    printf("Erro na leitura do cluster\n");
    return 0;
}

int mkFile(char* nome, char* extensao, char clusterPai, char cluster, MetaDados metaDados){
/* Cria um arquivo no primeiro cluster disponível dado o diretório atual
 * Entrada:
 *      String com o nome do arquivo dado pelo usuario
 *      String com a extensao do arquivo
 *      Ponteiro (char) para o pai do diretório a ser criado
 *      Ponteiro (char) para o cluster diponível
 *      Tabela FAT que armazena os ponteiros dos discos
 *      Metadados para auxiliar na busca no arquivo
 * Retorno:
 *      1 caso seja realizado com sucesso
 *      0 caso a criação falhe
 */
    NodoCluster novo = {"", "", 'a', '*'};
    ListaStrings *lsNome;
    lsNome = NULL;

    lsNome = inserirLSEStrings(lsNome, nome);

    if(encontraCaminho(lsNome, clusterPai, clusterPai, metaDados) != -1){
        apagaLSE(lsNome);
        printf("Ja existe um arquivo com esse nome.\n");
        return 0;
    }
     apagaLSE(lsNome);
    //Cria o novo Cluster
    strcpy(novo.nome, nome);
    strcpy(novo.extensao, extensao);
    novo.pai = clusterPai;

    if(!insereNodoCluster(novo, cluster, metaDados)){   //Se houve erro ao inserir o novo cluster
        printf("Erro ao escrever o novo cluster\n");
        return 0;
    }

    if(!adicionaFilho(clusterPai, cluster, metaDados)){ //Se houve erro ao adicionar um filho na lista de filhos
        printf("Erro ao atualizar o diretorio pai\n");
        return 0;
    }

    if(!alteraTabelaFat(255, cluster, metaDados)){      //Marca o primeiro cluster disponivel como ocupado e escreve no arquivo
        printf("Erro ao atualizar a tabela\n");
        return 0;
    }

    return 1;
}

int rm(ListaStrings *listaCaminho, char *diretorioAtual, char *diretorioSolicitado, MetaDados metaDados){
/*  Função que trata os primeiros casos em relação ao caminho que pode ser passado: Caminho nulo e root
 *  Entrada:
 *      Uma lista de strings que representa o caminho do cluster que o usuário quer remover
 *      Char* que representa o diretporio atual no momento da chamada da função
 *      Char* que representa o ponteiro que será alterado caso seja encontrada um cluster com o caminho solicitado
 *      MetaDados metadados do disco que serão utilizados para realizar as operações com o fseek.
 *  Retorno:
 *      1 caso o caminho tenha sido encontrado
 *      0 caso o caminho nao tenha sido encontrado
 */
    int ponteiroCluster;
    ListaStrings *aux;

    if(listaCaminho == NULL){                                //Verifica se o caminho recebido é vazio
        return 0;                                            //Se sim, retorna 0, indicando erro
    }else if(strcmp(listaCaminho->comando, "root") == 0){    //Verifica se o caminho solicitado é o root
        if(listaCaminho->prox != NULL){                      //se for um caminho maior que apenas 'root'
            aux = listaCaminho->prox;                        //coloca o inicio do caminho na segunda string da lista
            *diretorioAtual = 0;                             //coloca o usuario no root para evitar que ele esteja em cluster deletado
        }else{
            return 0;                                        //Se for apenas 'root', nao deleta
        }
    }else{
        aux = listaCaminho;
    }

    //Se nenhum dos casos anteriores for "pego", encontra o cluster a ser deletado
    ponteiroCluster = encontraCaminho(aux, *diretorioAtual, *diretorioAtual, metaDados);
    if(ponteiroCluster != -1){
        *diretorioSolicitado = ponteiroCluster;
        return 1;
    }
    return 0;

}

int removeCluster(char cluster, MetaDados metaDados){
/*  Função que remove uma sub-árvore de cluster do disco
 *  Entrada:
 *          Char que representa o ponteiro, da tabela FAT, que desejamos remover
 *          MetaDados metadados do disco que serão utilizados para realizar as operações com o fseek.
 *  Retorno:
 *      1 caso a função seja realizada com sucesso
 *      0 caso o caminho tenha dado algum erro durante a execução da função
 */
    int aux = 0;
    FILE *arq;
    NodoCluster dir;

    pegaCluster(cluster, &dir, metaDados);  //Pega o cluster atual
    if(strcmp(dir.extensao, "") != 0){      //Se tiver extensao
        removeArquivo(cluster, metaDados);  //Apaga todo o arquivo
        return 1;                           //Encerra esta chamada
    }

    alteraTabelaFat(254, cluster, metaDados); //Marca o cluster, na tabela FAT, como 'disponível'

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o cursor do arquivo no cluster que vamos remover
    fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * cluster), SEEK_SET);

    while(aux != '*'){              //Posiciona o cursor na lista de filhos do cluster
        aux = fgetc(arq);
    }

    aux = fgetc(arq);                                   //Verifica se o cluster tem filhos
    if(aux == 0){                                       //Se não tem filhos
        fclose(arq);                                    //Fecha arquivo
        return 1;
    }else{                                              //Se o cluster tem filhos
        while(aux != 0){
            if(aux != 254){                             //Se nao for um filho removido
                removeCluster(aux, metaDados);          //Chama a função recursivamente para a sub-árvore do filho
            }
            aux = fgetc(arq);                           //pega o proximo filho
        }
        fclose(arq);
        return 1;
    }
}

int move(ListaStrings *origem, ListaStrings *destino, char *diretorioAtual, MetaDados metaDados){
/* Dado um caminho de origem e um caminho de destino, movimenta a pasta ou arquivo de local se possivel
 * Entrada:
 *      Lista de Strings representando o caminho de origem
 *      Lista de Strings representando o caminho de destino
 *      Char representando o ponteiro para o cluster atual
 *      MetaDados para auxiliar nos calculos do fseek
 * Retorno:
 *      1 caso haja sucesso no movimento de diretorio/arquivo
 *      0 caso haja uma falha
 */
    char dirDestino, dirRemovido;
    FILE *arq;
    ListaStrings *aux;
    NodoCluster clusterOrigem, clusterDestino;

    //Se nao for, abre o arquivo
    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    if(strcmp(origem->comando, "root") == 0){ //Verifica se o caminho comecar com 'root/'
        *diretorioAtual = 0;                  //Coloca o usuario no primeiro diretorio
        aux = origem->prox;                   //Comeca a lista de comandos pelo segundo comando
        if(aux == NULL){                      //Verifica se o caminho de destinho possui, no mínimo, mais um cluster
            printf("Nao e possivel mover a pasta root.\n"); //Se não possui, informa ao usuário que não é possível mover a pasta root
            return 0;
        }
    }else{                                    //Se o caminho não começa por root/
        aux = origem;                         //Começa a busca pelo caminho indicado pelo usuário
    }

    dirRemovido = encontraCaminho(aux, *diretorioAtual, *diretorioAtual, metaDados); //Percorre o caminho
    if(dirRemovido != -1){                                      //Verifica se o caminho de fato existe.
        pegaCluster(dirRemovido, &clusterOrigem, metaDados);    //Pega o cluster a ser movido
        if(strcmp(destino->comando, "root") == 0){              //Verifica se o caminho comeca com 'root/'
            *diretorioAtual = 0;                                //Se sim, coloca o usuario no primeiro diretorio
            dirDestino = 0;
            aux = destino->prox;                                //comeca a lista de comandos pelo segundo comando
        }else{                                                  //Se o caminho não começa com "root/"
            aux = destino;                                      //caso contrario apenas navega pelo caminho dado
        }

        if(aux != NULL){                                        //Se o destino nao e' a pasta root
            dirDestino = encontraCaminho(aux, *diretorioAtual, *diretorioAtual, metaDados); //Pega o cluster onde será inserido o filho
            *diretorioAtual = 0;                                //Coloca o usuario no diretorio root
        }
        pegaCluster(dirDestino, &clusterDestino, metaDados);
        if(dirDestino!= -1){                                             //Verifica se o caminho de fato existe
            if(dirDestino == dirRemovido){
                printf("Nao e possivel mover uma pasta para ela mesmo.\n");
                return 0;
            }
            if(!strcmp(clusterDestino.extensao, "")){
                removeFilho(clusterOrigem.pai, dirRemovido, metaDados);      //Se sim, remove o filho do cluster origem
                adicionaFilho(dirDestino, dirRemovido, metaDados);           //Insere o filho removido do clsuter origem no cluster destino
                clusterOrigem.pai = dirDestino;                              //Modifica o pai do cluster movimentado
                modificaNodoCluster(clusterOrigem, dirRemovido, metaDados);  //Escreve o cluster modificado na sua posicao
                fclose(arq);
                return 1;
            }else{//Se o destino for um arquivo de texto, informa ao usuário que não é possível realizar essa operação.
                printf("Nao e possivel mover algo para um arquivo de texto.\n");
                return 0;
            }
        }else{                                                           //Se o destino não for encontrado, informa ao usuário
            printf("Destino nao encontrado\n");
            fclose(arq);
            return 0;
        }
    }

    printf("Origem nao encontrada\n");
    fclose(arq);
    return 0;
}

int reName(ListaStrings *listaCaminho, char* novoNome, char* diretorioAtual, MetaDados metaDados){
/* Renomeia um arquivo/diretorio dado seu caminho no disco se possivel
 * Entrada:
 *      Lista de Strings representando o caminho do arquivo/diretorio
 *      String com o novo nome do cluster
 *      Char representando o ponteiro para o diretorio atual
 *      MetaDados para auxiliar na pesquisa no arquivo
 * Retorno:
 *      1 caso haja sucesso de troca de nome
 *      0 caso haja alguma falha
 */
    int ponteiroCluster;
    ListaStrings *aux;
    NodoCluster cluster;

    if(!strcmp(listaCaminho->comando, "root")){     //Se for comando iniciado com /root
        *diretorioAtual = 0;                        //Coloca o usuario no diretorio inicial
        aux = listaCaminho->prox;                   //Comeca a busca a partir do segundo elemento
    }else{
        aux = listaCaminho;                         //Senao, comeca a busac pelo primeiro elemento da lista
    }

    if(aux != NULL){                                                //Se existe mais comandos
        ponteiroCluster = encontraCaminho(aux, *diretorioAtual, *diretorioAtual, metaDados); //Encontra o cluster procurado
        if(ponteiroCluster != -1){                                                           //Se encontrou
            pegaCluster(ponteiroCluster, &cluster, metaDados);                               //Pega o cluster
            strcpy(cluster.nome, strtok(novoNome, "."));                                     //Modifica o nome
            return(modificaNodoCluster(cluster, ponteiroCluster, metaDados));                //Grava o cluster modificado
        }else{                                                                               //Se nao encontrou
            printf("Caminho nao encontrado\n");                                              //Avisa ao usuario
            return 0;                                                                        //Retorna o erro
        }
    }else{                                                          //Se nao existe mais comandos, tentou modificar o root
        printf("Nao e possivel renomear o root\n");                 //Avisa ao usuario
        return 0;                                                   //Retorna o erro
    }
}

int edit(char* texto, char clusterArquivo, MetaDados metaDados){
/* Insere um texto dentro de um arquivo do tipo .txt
 * Entrada:
 *      char* variável que armazena o texto que será inserido no arquivo
 *      char ponteiro que representa o cluster do arquivo que será modificado
 * Retorna:
 *      1 caso a operação seja realizada com sucesso
 *      0 caso ocorra um erro ao realizar a operação
 */
    int indexTexto = 0, indexCluster = sizeof(NodoCluster);
    char proximoCluster, atualCluster, zero = 0;
    FILE *arq;
    NodoCluster dir;

    /* TEXTO DUMMY DE 32000 BYTES APROXIMADAMENTE */
    //texto = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur convallis sapien eu libero dictum fermentum. Nulla maximus dignissim justo, id sagittis velit tincidunt non. Ut sed accumsan sem, ut rhoncus dolor. In sit amet suscipit sem, id dictum nisl. Pellentesque at sem lacinia, consequat massa a, viverra arcu. Curabitur eget sagittis diam. Maecenas ac ante neque. Nam fermentum lacus arcu, vitae accumsan augue porta in. Aliquam facilisis aliquet nisi, eget auctor nulla vulputate id. Aenean ultrices, nisl id fermentum laoreet, ex diam tincidunt nisl, at sagittis tellus ex ut magna. Mauris elementum sit amet urna non imperdiet. Donec id velit vitae lectus congue ornare. Pellentesque tristique suscipit interdum. Integer luctus iaculis imperdiet. Duis et sapien odio. Nulla scelerisque tempor massa, ac faucibus ligula malesuada id.Curabitur convallis felis lorem, nec tristique diam molestie sed. Phasellus mollis fermentum lacus sed elementum. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi atpurus nec nulla iaculis tempus. Donec molestie turpis neque, id fringilla mi viverra id. Cras ex est, hendrerit eu turpis pretium, vehicula egestas odio. Nullam a gravida urna. Curabitur in finibus sem, non bibendum risus. Donec tempor vitae ligula quis aliquet. Nullam eget est a dui placerat sollicitudin et a nulla. Nam facilisis volutpat lectus nec sodales. Aliquam at vestibulum eros.Proin pharetra facilisis turpis quis iaculis. Donec dignissim egestas lectus, non ullamcorper orci pretium non. Vivamus vel sodales augue. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Quisque feugiat, odio interdum faucibus laoreet, magna metus consectetur nisi, sit amet ultrices dui eros elementum neque. Nunc laoreet at sapien vitae mollis. Morbi porta massa nibh, ac dictum nulla elementum feugiat. Vivamus ultricies tortor convallis, semper lacus vel, consequat nibh. Nulla eleifend felis nec lobortis sagittis. Etiam quam nisi, ullamcorper sed ex id, gravida sagittis quam. Nunc eu auctor nisi. Praesent pellentesque, magna at sodales pellentesque, dui elit vulputate erat, eu vulputate enim turpis ac nisl. Proin vitae ipsum eu dolor lacinia lobortis. Sed nec semper sapien. Nullam lacus augue, porttitor et turpis ac, aliquet malesuada justo.Suspendisse id tristique quam. Nulla in erat eleifend, aliquet ex sit amet, molestie magna. Curabitur dolor eros, fermentum quis sagittis et, tristique eu purus. Sed pellentesque nisi sit amet metus luctus ornare. Integer iaculis mollis lorem, a tincidunt mauris iaculis eget. Nunc lobortis ante eget velit tristique, id convallis mi blandit. Nam euismod interdum enim, sed viverra nibh iaculis eu. Morbi ex nunc, blandit porta efficitur sit amet, dictum ut neque. Quisque porttitor neque ac risus iaculis, vel pharetra mi rhoncus. Sed hendrerit sodales nulla, in dictum velit pellentesque eu. Praesent massa felis, tincidunt eu ornare a, placerat eu dui. Maecenas tortor nulla, fermentum non pellentesque vel, ornare a dui. Quisque eu est libero. Pellentesque varius sodales nibh vitae fermentum.Vivamus tincidunt, erat eu faucibus congue, lacus turpis efficitur enim, at auctor lacus lacus nec ligula. Nam at turpis egestas, varius leo id, convallis elit. Etiam at sem augue. In ac arcu ornare, vehicula elit in, hendrerit lorem. In ut porta ipsum, in porta odio. Aenean risus augue, pulvinar non pharetra at, porta eu dolor. Sed varius mattis leo, at pellentesque turpis elementum ac.Mauris efficitur feugiat ex, a egestas metus. Proin orci dui, convallis sit amet malesuada at, suscipit id nulla. Curabitur ullamcorper, erat non porta vulputate, ipsum sem venenatis orci, quis auctor diam orci sit amet sem. Phasellus non iaculis mauris. In malesuada ligula nulla, vitae sollicitudin lectus egestas ut. Cras id convallis ante. Curabitur sit amet tristique turpis. Nam auctor sollicitudin velit, vel ullamcorper massa porta eget. Nam volutpat sapien quis lacinia tempus. Vestibulum est est, dictum et turpis a, venenatis mollis ante. Suspendisse quis ex dictum, finibus purus in, bibendum urna. Maecenas eu dui eu neque maximus maximus. Maecenas vehicula, nunc sed accumsan auctor, neque augue imperdiet dolor, id consequat felis erat ut mauris. Praesent iaculis vulputate elit, non semper sapien volutpat id. Donec nec blandit quam, id viverra odio.Nam congue ex sed risus interdum aliquam. Aliquam erat volutpat. Proin interdum ipsum in aliquet ullamcorper. Quisque commodo aliquam tortor, id elementum elit consectetur non. Praesent faucibus sagittis sagittis. Praesent in imperdiet augue. Nulla mattis felis at orci efficitur vulputate. Nulla tempor orci lacus, ut fringilla tortor scelerisque in. Phasellus pulvinar, dolor at rhoncus iaculis, enim magna mollis risus, at molestie libero nunc id ipsum. Morbi ultrices elementum enim aliquam vehicula. Donec vitae lacinia felis. Phasellus scelerisque eros sit amet justo vehicula, non aliquet sapien pretium. Integer finibus vel diam blandit viverra. Praesent pulvinar ipsum in lacus mattis, eget fringilla augue porttitor.Fusce sed purus eu purus malesuada mattis placerat et ipsum. Donec tortor augue, dignissim vel gravida sit amet, feugiat in lorem. Nam ultrices est ut tortor convallis, eu blandit nulla sagittis. Donec quis nunc eu ex scelerisque sagittis. Nunc egestas metus sed erat aliquet, et imperdiet nunc euismod. Aliquam ultricies tristique enim ut malesuada. Pellentesque a ante ut dui lobortis efficitur at at mi. Pellentesque scelerisque nisi a dolor elementum, at consectetur ipsum ornare. Nunc nec metus quis libero vehicula porta ut at ex. Suspendisse potenti. Sed eget venenatis risus.Maecenas fermentum, metus eget hendrerit faucibus, eros est laoreet velit, at gravida ipsum metus a eros. Pellentesque diam nibh, aliquam sit amet laoreet at, posuere at enim. Cras viverra, dolor a interdum ultrices, metus risus semper nulla, quis aliquam nisl lorem in nunc. Duis vel euismod sem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam id fermentum magna, eget semper nulla. Suspendisse magna diam, ultricies non odio a, vestibulum congue lorem. Nunc ipsum orci, volutpat a tortor nec, pulvinar pellentesque sapien. Nullam ultrices bibendum massa, viverra tincidunt ante sodales eget.Cras porttitor nibh a felis egestas finibus. Donec dapibus maximus sagittis. Nam ullamcorper ullamcorper convallis. Praesent eget faucibus libero, et maximus ligula. Curabitur eleifend, orci vitae facilisis vulputate, nisl enim iaculis libero, quis gravida lacus magna suscipit mauris. Curabitur efficitur dui interdum dictum suscipit. Maecenas facilisis efficitur est, quis interdum mauris tincidunt non. Sed eu odio ante. Morbi aliquam porta nunc, id finibus neque condimentum id. Nam pulvinar mi id elementum dictum. Maecenas pretium dolor malesuada erat bibendum, quis efficitur risus iaculis.Sed augue mi, bibendum non tempus vitae, vulputate sit amet arcu. In non nisl finibus, sodales ex quis, faucibus eros. Quisque luctus lectus placerat, feugiat leo malesuada, elementum enim. Nulla facilisi. Nunc vel venenatis augue. Quisque dui nisi, pretium quis mattis a, posuere ac felis. Vestibulum ut diam id nisi fermentum pretium. Maecenas commodo maximus aliquam. Morbi venenatis tortor ligula, quis efficitur arcu tincidunt eget. Aliquam accumsan euismod turpis, eget eleifend turpis rhoncus quis. Pellentesque auctor nulla tellus, eu finibus magna dapibus in. Curabitur orci turpis, semper et fermentum id, pretium id elit. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean semper neque id volutpat tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Sed in fermentum odio. Donec eget pretium ex. Donec egestas a mauris non pulvinar. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Integer ac sapien sed turpis gravida elementum. Aenean facilisis eros ac aliquet maximus. Proin ac auctor sapien, sit amet laoreet urna. Ut in accumsan lacus.Cras varius vitae diam non pulvinar. In dignissim, neque posuere interdum eleifend, ipsum arcu rutrum neque, id bibendum felis nulla et massa. Aenean eget placerat nisi. Sed nec faucibus magna. Vestibulum finibus volutpat venenatis. Vestibulum at facilisis leo. Ut luctus libero arcu, vel feugiat metus vulputate ac. Nam sit amet pharetra mi, ac molestie neque. Donec malesuada interdum nisi sit amet viverra. Mauris feugiat convallis est. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Curabitur a semper ipsum. Vivamus porta risus nec ex commodo bibendum. Proin vitae faucibus ex. Curabitur feugiat, est quis facilisis molestie, leo lorem placerat urna, vel euismod risus diam semper ipsum. Cras posuere id ante eget ultrices. Fusce vulputate, ante non suscipit dictum, nibh sem posuere lacus, at auctor nulla metus a odio. Vestibulum in ex nisl. Integer ut tellus tristique, feugiat orci sed, malesuada turpis. Sed sed tempor tellus, facilisis bibendum leo. Sed porta ac justo ac tincidunt.Morbi dignissim ex nibh, in suscipit urna ullamcorper ullamcorper. Nullam suscipit diam metus, sed euismod purus placerat a. Quisque eleifend semper nibh, et pulvinar nulla imperdiet ac. Proin euismod massa sit amet congue consequat. Quisque id ligula vel erat consequat consectetur. Integer eget purus sed mi accumsan bibendum. Phasellus lacinia ex eleifend tincidunt maximus. Etiam ultrices hendrerit purus, eu volutpat eros imperdiet id. Integer varius mi vel neque dignissim aliquet. Curabitur ac finibus odio. Mauris odio libero, consectetur ac congue nec, aliquet et ipsum. Aenean imperdiet lacus in odio porta aliquet. Pellentesque nec imperdiet nibh. Vestibulum placerat faucibus felis at accumsan. Sed et dignissim mi, eu dapibus nulla.Pellentesque volutpat feugiat vulputate. Vestibulum ac purus ac leo laoreet fermentum. Quisque eros augue, commodo non sem vel, convallis malesuada mauris. Vivamus ac ante eu mauris volutpat ornare eget et ex. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent nec placerat tellus, a dignissim magna. Nulla ut dui laoreet, blandit orci pretium, lacinia risus. Nullam bibendum euismod massa nec vestibulum. In sollicitudin sem massa. Morbi dapibus nisi diam, eget ullamcorper felis scelerisque in. Phasellus gravida dignissim est, eu eleifend lorem viverra at. Nam rhoncus, est placerat interdum commodo, neque diam consequat ante, vitae gravida nibh leo a nunc.Fusce at sapien diam. Pellentesque mollis, sem ac viverra fermentum, velit orci cursus odio, in rutrum odio nisi a metus. Phasellus maximus euismod mattis. Nullam magna urna, tempus tincidunt laoreet id, volutpat vel leo. Nunc vehicula placerat nulla, et tristique justo efficitur eu. Nulla interdum fermentum nisi eget egestas. Pellentesque a nulla eget tortor fringilla accumsan. Proin non tempor ligula. Sed commodo malesuada odio, ut vulputate arcu auctor in. Nunc tristique laoreet elit et lacinia. Aliquam erat volutpat. Phasellus libero purus, iaculis nec enim sed, finibus faucibus ante. Aenean dignissim lectus a ipsum blandit finibus.Vestibulum posuere dictum est non venenatis. Praesent quis libero at arcu aliquet laoreet a vel tortor. Donec dignissim arcu augue, vitae aliquam ante rutrum in. Fusce sollicitudin libero nunc, quis gravida lorem ullamcorper in. Etiam vulputate interdum enim vitae aliquet. Vestibulum eu luctus dui. Maecenas pulvinar id quam et aliquam. Aliquam in diam at nulla tristique ultrices efficitur quis massa. Nullam ultricies lobortis magna, non pharetra libero ornare et. In vitae mi ante.Duis suscipit et nisi eget mollis. Integer vel nisi eget quam luctus aliquam non vehicula est. Sed lacinia mi dolor, eu mollis est dictum quis. Integer magna felis, auctor tempus laoreet ac, tincidunt sit amet felis. Integer tristique congue viverra. Sed quis sem massa. Quisque vitae massa augue. Pellentesque semper, odio vitae ultricies semper, felis quam euismod velit, at tempus est dui a ligula. Aenean diam diam, fringilla tincidunt tempor malesuada, dictum vitae dui. Ut sed efficitur purus. Aenean ultricies sollicitudin risus ut auctor. Mauris sagittis bibendum ornare. Interdum et malesuada fames ac ante ipsum primis in faucibus.Fusce dolor massa, porta in sagittis sed, tristique pharetra lectus. Phasellus justo leo, posuere a magna id, suscipit lobortis felis. Sed et leo vitae mauris dignissim rhoncus. Suspendisse a erat ut felis viverra pretium ut id dolor. In vitae ullamcorper risus, non condimentum massa. Curabitur a tempor turpis, ac egestas turpis. Ut sodales ac diam ac fringilla. Aliquam erat volutpat. Aenean nec molestie ipsum. Curabitur vehicula eget libero sit amet eleifend. In dictum sodales urna ut maximus. Morbi at leo nec magna aliquet egestas. Ut viverra ornare nisl, et rhoncus elit rhoncus quis. Duis nulla metus, blandit vel cursus id, semper a nibh. Aenean eget facilisis magna. Nunc nisl purus, pharetra quis ex convallis, bibendum eleifend dui.Mauris cursus odio sit amet dui feugiat, imperdiet tincidunt odio finibus. Nulla ac pharetra urna. Duis gravida mauris sem, sed pulvinar dui sodales id. Ut lobortis tristique leo et ultrices. In auctor tempus metus maximus fringilla. Integer placerat dui enim, sed maximus mauris porta nec. In hac habitasse platea dictumst. Interdum et malesuada fames ac ante ipsum primis in faucibus. Quisque sagittis elit accumsan, fringilla velit at, ornare purus. Curabitur id bibendum dolor, vel vulputate dolor.Donec non scelerisque nisi. Ut ac mattis augue. Curabitur id commodo ante. Cras lobortis tempus erat non commodo. Pellentesque aliquet augue ac ex aliquam ultrices. Aenean efficitur porttitor leo, quis vestibulum quam tempus id. Nulla facilisi. Praesent velit risus, rutrum in ipsum nec, posuere auctor enim. In quis dictum risus, quis ullamcorper massa. Vivamus egestas fringilla quam, viverra ornare sem hendrerit nec.Aenean vestibulum elit ornare justo consequat, hendrerit congue nunc bibendum. Quisque at odio ut risus vehicula ullamcorper. Vestibulum fringilla ex libero, ac porta justo dictum eu. Praesent sit amet sapien lectus. Donec ut ultrices magna, nec tincidunt massa. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Mauris ullamcorper dui a imperdiet pharetra. Vivamus mauris nunc, rutrum eget turpis eu, scelerisque porttitor est. Etiam eget odio elit. Quisque urna diam, posuere eget efficitur quis, tincidunt non nulla. Nunc nec massa mauris.Proin porta, elit et fringilla pretium, massa erat finibus libero, laoreet pulvinar leo sem quis ligula. Donec purus arcu, pharetra nec convallis eu, eleifend ut urna. Donec urna leo, placerat eget vulputate sit amet, lacinia in lorem. Sed eget diam vel nisl volutpat convallis eget quis neque. Praesent gravida elementum neque, ac suscipit dolor tempor et. Maecenas id iaculis purus. Quisque non erat pulvinar, vulputate quam pharetra, vehicula nisi. Pellentesque ac mauris quis sem laoreet blandit eget et ante. Aliquam eleifend volutpat sem, in vehicula libero rhoncus eget. Sed pharetra, eros vitae tempus auctor, ipsum neque feugiat elit, non sodales augue leo ut leo. Nam tristique justo ut ornare bibendum. Donec euismod, tellus non imperdiet malesuada, purus eros ornare est, euismod laoreet eros elit vel nulla. Pellentesque convallis elementum libero, dignissim malesuada lectus ullamcorper nec. Vestibulum at massa vitae leo posuere commodo. Suspendisse tempus vitae mi vitae bibendum. Donec suscipit tristique elit, et accumsan nunc tincidunt eget.Nam nec tincidunt erat, ac hendrerit dolor. Morbi orci erat, maximus quis tempor ac, mattis id ligula. Sed eu elementum risus. Praesent commodo ipsum ut sem ornare facilisis. Etiam quis dapibus purus. Aenean quis ex porta, faucibus tortor a, scelerisque turpis. Aenean a tempus urna. Integer sit amet leo felis. Proin elementum urna at viverra luctus. Aliquam pulvinar, orci eget iaculis tempor, diam libero posuere ex, ac tincidunt leo quam a elit. Suspendisse sit amet massa aliquam ipsum venenatis iaculis. Praesent eget quam lorem.Suspendisse potenti. Sed ac ante non elit fringilla dignissim vitae posuee tortor. Duis congue mattis justo vitae tristique. Nam vitae semper justo, vel tempor tortor. Etiam laoreet quam magna, mollis vulputate lorem lacinia vel. Quisque suscipit congue porta. Phasellus id felis id dolor convallis feugiat. Fusce finibus lobortis diam, quis lacinia nisi blandit eu. Morbi nec gravida magna.Donec aliquam libero tortor, vel sollicitudin dui tristique id. Nullam lacinia leo libero, vitae vestibulum justo semper et. Proin dictum maximus urna. Curabitur fringilla sit amet tortor id bibendum. Integer dignissim sapien vitae lacus sodales bibendum. Vestibulum sem magna, elementum quis eros id, feugiat convallis justo. In hendrerit porttitor justo vitae scelerisque.Aliquam malesuada massa ac risus molestie scelerisque. Cras fermentum commodo massa, non lobortis libero gravida sit amet. Nunc porta augue quis augue rutrum ultricies. Ut pulvinar sapien fringilla auctor consectetur. Maecenas non tincidunt nibh, quis venenatis arcu. Phasellus in ultrices sapien. Sed tempor tristique placerat. Aenean maximus, ipsum eu volutpat commodo, arcu nisl cursus dui, id molestie lectus magna non ex. In hac habitasse platea dictumst. Praesent id aliquet dolor.Integer lobortis mauris eget nibh tristique, vel ultrices tortor scelerisque. Phasellus sed neque turpis. Etiam et ornare libero, a imperdiet est. Sed interdum arcu a lectus sagittis faucibus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Curabitur nec ante a nisl eleifend scelerisque. Nam lorem est, finibus nec tortor dignissim, auctor cursus est. Duis fringilla ac nulla et aliquet. Aliquam blandit orci sed lectus egestas porttitor et et lectus. Nunc volutpat luctus nisl non efficitur. Fusce purus turpis, ultricies nec lacinia a, hendrerit vitae justo.Phasellus leo odio, dapibus sed est sed, dignissim interdum ante. Donec dignissim luctus nisl, et suscipit arcu consectetur vitae. Fusce vestibulum laoreet consequat. Donec dui orci, scelerisque eu efficitur in, condimentum eu dolor. Cras pulvinar nisi id nibh accumsan, ac tincidunt arcu tincidunt. Fusce et nisl non eros semper lobortis. Aenean vitae fermentum lorem, sed imperdiet nibh. Cras vitae blandit nisi. Cras vitae fermentum ipsum. Etiam porta lorem nec turpis varius pulvinar. Mauris luctus mauris tristique, ultrices dui quis, consectetur nunc.Cras sapien felis, vestibulum non justo ut, varius egestas nisi. Suspendisse maximus augue nunc, nec congue sem sollicitudin ultrices. Etiam ut maximus augue. Aenean gravida hendrerit augue cursus malesuada. Sed vitae dictum quam. In euismod pulvinar tellus eget ullamcorper. Interdum et malesuada fames ac ante ipsum primis in faucibus. Suspendisse at orci ex. Nam egestas lectus vel hendrerit pulvinar. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; In bibendum neque vestibulum nibh tempor tristique. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Pellentesque et risus a tellus cursus pretium. Sed vitae elit interdum tortor varius commodo eu in libero.Integer in elit ut lacus viverra vestibulum. Vivamus aliquam ullamcorper felis. Phasellus porttitor interdum ligula, sed consectetur nisl euismod eu. Duis quis tempus turpis. Mauris pellentesque velit a pellentesque mattis. Nam non ornare urna. Vestibulum lacus lorem, vehicula et est in, posuere vehicula est. Morbi felis nibh, interdum ut massa vitae, feugiat aliquet turpis. Maecenas commodo nibh urna, eget placerat lacus luctus eu. Morbi varius magna vel dignissim varius. Aliquam nunc erat, finibus vitae posuere sit amet, maximus a lectus. Fusce quam orci, aliquam eu efficitur sed, auctor in lorem. In et dolor id neque placerat luctus rhoncus in nunc. Nunc scelerisque risus non tincidunt vulputate. Aliquam arcu nulla, pharetra tempus sapien a, pharetra posuere augue.Interdum et malesuada fames ac ante ipsum primis in faucibus. Ut rutrum dictum orci, eu molestie risus tristique vitae. Nullam erat mauris, molestie et nulla eu, rutrum finibus dui. Nulla tortor tortor, rhoncus quis posuere sed, mollis at lacus. Sed varius commodo justo, vel aliquam nisl accumsan ut. In id lorem est. Quisque a nisl a nulla venenatis rhoncus. Integer nec tincidunt metus. Etiam sit amet neque dui. In aliquam, risus vel pharetra varius, odio velit laoreet purus, eget molestie eros lectus a ex.Maecenas auctor pretium ipsum, a mollis nulla laoreet nec. Sed ut turpis eleifend, dapibus felis nec, sagittis risus. Quisque in mauris dui. Nam in pharetra quam. Maecenas luctus hendrerit urna sit amet consectetur. Suspendisse in mattis lectus, vitae imperdiet nunc. Cras consequat mi ligula, non congue sem lacinia et. Vestibulum tristique fringilla aliquam. Etiam hendrerit, mi ut rhoncus laoreet, libero mauris auctor magna, sit amet varius diam ligula eget odio. In lacinia porttitor magna ac auctor.Vestibulum eleifend, odio quis venenatis cursus, mauris erat dignissim lorem, nec convallis lectus justo non lorem. Sed in dolor eget enim placerat dapibus et ac lorem. Interdum et malesuada fames ac ante ipsum primis in faucibus. Duis bibendum orci nec molestie posuere. In sit amet tortor blandit, viverra est quis, tempus ante. Morbi quis malesuada massa. Quisque sit amet nisi congue, auctor massa quis, hendrerit est. Praesent tincidunt ante in condimentum egestas. Vivamus accumsan molestie tempus. Mauris placerat consequat congue. Duis purus odio, fermentum ut sem ac, sollicitudin imperdiet libero. Suspendisse fringilla suscipit ante. Donec sit amet sagittis massa, non rutrum sapien. Fusce accumsan posuere nisl non placerat. Quisque a lectus ipsum.Vestibulum tincidunt urna neque, ac posuere risus dapibus ultrices. In aliquet dolor sed tempor accumsan. Cras eu dolor placerat felis iaculis ultricies sit amet vel sem. Morbi nec nibh a est tincidunt ultricies. Praesent dignissim tristique sapien. Sed porta, lorem in posuere finibus, ipsum ligula fringilla orci, in rutrum tellus ante id orci. Etiam in suscipit dui. Nullam ultrices neque sit amet elit lacinia consectetur et eu nulla. Nulla lacinia mi purus, at facilisis libero egestas at. Donec porta finibus magna, eget efficitur magna ultrices a. Donec gravida nibh vel sollicitudin interdum.Nulla id ex convallis, luctus purus quis, aliquet ligula. Quisque nec finibus lorem. Phasellus nec faucibus metus, in laoreet magna. Cras tortor elit, interdum vel facilisis nec, blandit a ex. Fusce eu massa convallis mi iaculis blandit. Quisque viverra mi non ante commodo iaculis. Cras tincidunt ornare velit, eu lobortis erat euismod eget. Vivamus dignissim mollis odio at accumsan. Nulla ut neque vehicula, tempor dui quis, aliquam ipsum. Curabitur convallis mi non auctor dictum. Mauris viverra commodo rutrum. Mauris scelerisque nibh ullamcorper ipsum congue viverra. Morbi sagittis scelerisque turpis, quis efficitur nibh. Maecenas in erat justo. Sed cursus tellus id tortor ullamcorper, et congue purus finibus.Vivamus nec leo commodo, sodales ligula vulputate, tempus ex. Fusce lacinia diam elit, ac lobortis enim pretium rutrum. Pellentesque condimentum justo vitae ipsum convallis finibus. Donec quis sollicitudin elit, sit amet hendrerit ipsum. Mauris consectetur lorem velit, non finibus lorem dapibus ac. Cras quis est ut risus tempus ultricies ut ut purus. Suspendisse cursus velit at porttitor sagittis. Mauris non magna vel dolor vehicula vestibulum. Morbi gravida mattis lectus, vitae imperdiet neque porta eget. Nullam euismod cursus leo accumsan tempor. Integer vulputate ante et ante pellentesque viverra. Donec at massa eu massa sodales tempus.Cras nec pharetra quam, non dapibus enim. Aenean vel nunc venenatis, condimentum orci quis, tincidunt odio. Cras euismod justo ut dui lobortis ornare. Vivamus fringilla, enim eu congue porta, orci quam dictum sem, ac efficitur velit est eu nulla. Nulla mi nibh, varius malesuada ornare nec, venenatis porttitor felis. Aenean id maximus quam, non pharetra mi. Quisque ullamcorper ex risus, et cursus nisi volutpat eget. Maecenas sapien lorem, varius vitae lorem sit amet, dapibus ullamcorper elit. Pellentesque convallis velit quis dui ullamcorper, non cursus lectus accumsan. Mauris faucibus nisi sit amet tristique placerat. Cras luctus diam ut magna ultricies, et feugiat mauris pulvinar. Quisque cursus imperdiet ultricies.Praesent sodales metus et augue pharetra, at vehicula nulla tristique. Integer et dictum dui. Aenean eget libero aliquam, fermentum justo id, mattis mauris. Mauris lobortis diam metus, ut iaculis orci convallis vel. Proin ante mauris, sagittis ac orci ac, tincidunt sagittis ligula. Vestibulum lobortis nisi a ligula laoreet iaculis. Sed dignissim dolor diam, eget lacinia est sagittis et. Morbi elementum sodales ex eu facilisis. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nam nec consequat turpis, ultrices ornare quam. Integer nunc tellus, porttitor nec dolor vel, elementum tincidunt mauris. Pellentesque varius feugiat ex mollis fermentum.Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Proin quis hendrerit neque. Etiam tempor dolor in tellus tincidunt, a gravida sem ultrices. Nunc placerat turpis felis, nec dictum est pretium eu. Maecenas ullamcorper at nulla vel elementum. Maecenas sed tincidunt dolor. Interdum et malesuada fames ac ante ipsum primis in faucibus. Aenean cursus ligula arcu, ut molestie dui eleifend nec. Praesent faucibus sem leo, sit amet placerat sapien porttitor in. Mauris viverra dignissim tempor. Ut at mi ligula. Mauris faucibus ullamcorper dui in luctus. Pellentesque a porttitor ligula. Vestibulum interdum, tellus eget consectetur consectetur, nulla urna consectetur sem, non efficitur eros nisi in quam.Sed finibus dapibus massa, quis commodo elit lobortis et. Nulla mollis finibus luctus. Aliquam erat volutpat. Donec lacus lacus, consectetur sit amet pulvinar nec, porttitor sed mauris. Aliquam elementum in sapien nec aliquam. Fusce non ex ut ex iaculis rutrum. Aenean viverra arcu non elit posuere aliquam. Aenean magna massa, euismod in diam sit amet, interdum consectetur massa. Vivamus in imperdiet elit, eget vestibulum sem. Aliquam interdum diam a nulla lacinia, quis pellentesque lorem ullamcorper. Nullam rutrum, est sagittis sodales sollicitudin, tellus neque vestibulum mi, eu molestie leo elit eget justo. Integer justo tellus, efficitur at commodo sed, tempus sed mauris. Quisque iaculis ut eros in pretium. Donec in bibendum mauris, quis eleifend erat. Pellentesque aliquam dignissim varius. Etiam gravida finibus mi a aliquam.Morbi malesuada elit vel maximus hendrerit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus condimentum porta elit. Aenean mollis tellus ut urna venenatis ornare. Maecenas at scelerisque ipsum. Morbi pharetra placerat dolor, sit amet fringilla nisl posuere scelerisque. Curabitur pharetra mi at sollicitudin interdum. Nunc egestas sodales fringilla. Morbi condimentum ornare lobortis. Donec sed sodales purus, id cursus mauris. Suspendisse sit amet molestie turpis.Duis ullamcorper sem at nisl vehicula ornare. Phasellus fermentum felis et eros hendrerit cursus. Vivamus mattis lectus nec erat cursus facilisis. Suspendisse sit amet ullamcorper lorem. Aliquam sodales neque at turpis convallis volutpat. Integer consequat pellentesque tellus, et vestibulum justo lacinia a. Sed pulvinar imperdiet cursus. Nunc odio tellus, gravida ut ex sit amet, eleifend tristique massa. Pellentesque pharetra vel est sed efficitur. Curabitur viverra mauris eget metus hendrerit, ut hendrerit est vestibulum.Integer vel aliquet enim. Praesent eu congue quam. Suspendisse hendrerit eros ut ante fringilla varius. Nunc tristique facilisis urna. Nullam eget urna eget orci iaculis scelerisque egestas id ipsum. Proin non turpis ipsum. Curabitur congue lorem rutrum ligula mollis, malesuada hendrerit enim rutrum. Sed tortor justo, aliquam ac odio eget, condimentum condimentum ligula. Mauris condimentum elit magna, at vehicula arcu vehicula pellentesque.Integer placerat velit faucibus diam consectetur laoreet. Vivamus pretium lectus ac consectetur iaculis. Nulla facilisi. Sed varius nibh eget lorem commodo, a porttitor purus ornare. Mauris dapibus nulla eu dolor tristique, quis malesuada sem porta. Sed id venenatis quam, non cursus eros. Vivamus placerat pretium ligula, vel fermentum ligula suscipit at. Morbi viverra vel nulla et porttitor. Quisque mollis ultrices lorem et venenatis. Sed consectetur est nisl, non egestas orci porttitor eu. Donec id erat nec ligula rutrum dapibus eget at enim.Vestibulum leo mi, tincidunt vel ante id, posuere cursus ligula. Aenean laoreet interdum lorem, eget condimentum purus feugiat a. Praesent sed tristique purus. Maecenas rutrum posuere diam, sed posuere est auctor vel. Sed lobortis odio quis dolor ultricies, vitae tincidunt ipsum molestie. Pellentesque molestie arcu quis hendrerit condimentum. Etiam iaculis eleifend vulputate. Donec lobortis eu quam ac dapibus. Morbi vel molestie nisi. Donec quis sagittis urna, congue aliquam velit.Duis interdum mollis vehicula. Aenean iaculis eros sit amet odio cursus placerat. Proin porttitor risus orci, nec porttitor diam facilisis a. Fusce cursus, nisi vitae imperdiet sodales, justo leo luctus velit, eu luctus orci velit aliquet eros. Nulla facilisi. Integer egestas maximus ex, quis malesuada tellus laoreet sit amet. Suspendisse lacinia neque ac imperdiet aliquet.Vivamus vitae enim ex. Donec vehicula luctus sapien, id ultricies libero sodales quis. Aenean at diam nisi. Cras suscipit at lacus sed volutpat. Vivamus eu gravida justo. Praesent id rutrum eros. Aenean vel orci elit. Nam venenatis metus luctus, dictum magna sed, volutpat neque. Praesent id arcu iaculis, pretium orci a, tincidunt urna. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Curabitur posuere ut nisl vel rhoncus. Nulla facilisi. Quisque sed efficitur lacus.Quisque a diam sagittis velit sagittis aliquam. Donec cursus, ligula non mollis efficitur, libero lorem luctus eros, et tempor dolor metus eget magna. Cras facilisis mollis mi a rhoncus. Mauris ligula arcu, posuere viverra mi a, lobortis semper augue. Aliquam vitae turpis et tellus vehicula pharetra quis quis ex. Quisque diam odio, condimentum sed sem sit amet, condimentum rutrum turpis. Phasellus ligula orci, volutpat a eros in, pharetra pharetra lectus.Aliquam condimentum vestibulum risus, eget commodo est cursus nec. Cras finibus, eros vitae molestie scelerisque, justo tellus efficitur erat, et faucibus ipsum felis eu justo. Curabitur tincidunt et diam sed molestie. Nulla ornare dui varius eros aliquam, blandit porttitor lectus convallis. Ut id lectus nisi. Phasellus tellus enim, hendrerit quis purus ut, tempor interdum tortor. Quisque pretium nunc neque, ut vehicula ante consectetur eu. Cras eget velit sit amet ipsum pharetra pretium id quis quam.Curabitur eget finibus neque. Praesent suscipit eros ex, eu tempor est facilisis nec. Donec congue risus mi, efficitur semper nunc convallis sit amet. Vestibulum at accumsan magna, eu maximus massa. Quisque aliquet ipsum est, vitae porttitor eros semper ac. Nunc at dui urna. Nunc tempor pulvinar felis ac dignissim. Nulla leo felis, blandit non lectus sed, ultrices lacinia mauris. Cras ultrices commodo commodo.Morbi molestie ante a sem efficitur hendrerit. Sed pretium dignissim erat in imperdiet. Proin non arcu non ante ullamcorper accumsan. Nulla eleifend tempus nunc, in venenatis ex finibus nec. Phasellus eget elit lorem. Etiam maximus erat et velit efficitur laoreet. Donec porttitor, turpis nec consectetur malesuada, massa velit lacinia ipsum, efficitur laoreet turpis tortor eu tellus. Fusce dignissim dolor nec aliquet lobortis. Vivamus sollicitudin justo ut finibus aliquam. Duis massa nunc.";
    /* TEXTO DUMMY DE 64000 BYTES APROXIMADAMENTE */
    texto = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur convallis sapien eu libero dictum fermentum. Nulla maximus dignissim justo, id sagittis velit tincidunt non. Ut sed accumsan sem, ut rhoncus dolor. In sit amet suscipit sem, id dictum nisl. Pellentesque at sem lacinia, consequat massa a, viverra arcu. Curabitur eget sagittis diam. Maecenas ac ante neque. Nam fermentum lacus arcu, vitae accumsan augue porta in. Aliquam facilisis aliquet nisi, eget auctor nulla vulputate id. Aenean ultrices, nisl id fermentum laoreet, ex diam tincidunt nisl, at sagittis tellus ex ut magna. Mauris elementum sit amet urna non imperdiet. Donec id velit vitae lectus congue ornare. Pellentesque tristique suscipit interdum. Integer luctus iaculis imperdiet. Duis et sapien odio. Nulla scelerisque tempor massa, ac faucibus ligula malesuada id.Curabitur convallis felis lorem, nec tristique diam molestie sed. Phasellus mollis fermentum lacus sed elementum. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi atpurus nec nulla iaculis tempus. Donec molestie turpis neque, id fringilla mi viverra id. Cras ex est, hendrerit eu turpis pretium, vehicula egestas odio. Nullam a gravida urna. Curabitur in finibus sem, non bibendum risus. Donec tempor vitae ligula quis aliquet. Nullam eget est a dui placerat sollicitudin et a nulla. Nam facilisis volutpat lectus nec sodales. Aliquam at vestibulum eros.Proin pharetra facilisis turpis quis iaculis. Donec dignissim egestas lectus, non ullamcorper orci pretium non. Vivamus vel sodales augue. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Quisque feugiat, odio interdum faucibus laoreet, magna metus consectetur nisi, sit amet ultrices dui eros elementum neque. Nunc laoreet at sapien vitae mollis. Morbi porta massa nibh, ac dictum nulla elementum feugiat. Vivamus ultricies tortor convallis, semper lacus vel, consequat nibh. Nulla eleifend felis nec lobortis sagittis. Etiam quam nisi, ullamcorper sed ex id, gravida sagittis quam. Nunc eu auctor nisi. Praesent pellentesque, magna at sodales pellentesque, dui elit vulputate erat, eu vulputate enim turpis ac nisl. Proin vitae ipsum eu dolor lacinia lobortis. Sed nec semper sapien. Nullam lacus augue, porttitor et turpis ac, aliquet malesuada justo.Suspendisse id tristique quam. Nulla in erat eleifend, aliquet ex sit amet, molestie magna. Curabitur dolor eros, fermentum quis sagittis et, tristique eu purus. Sed pellentesque nisi sit amet metus luctus ornare. Integer iaculis mollis lorem, a tincidunt mauris iaculis eget. Nunc lobortis ante eget velit tristique, id convallis mi blandit. Nam euismod interdum enim, sed viverra nibh iaculis eu. Morbi ex nunc, blandit porta efficitur sit amet, dictum ut neque. Quisque porttitor neque ac risus iaculis, vel pharetra mi rhoncus. Sed hendrerit sodales nulla, in dictum velit pellentesque eu. Praesent massa felis, tincidunt eu ornare a, placerat eu dui. Maecenas tortor nulla, fermentum non pellentesque vel, ornare a dui. Quisque eu est libero. Pellentesque varius sodales nibh vitae fermentum.Vivamus tincidunt, erat eu faucibus congue, lacus turpis efficitur enim, at auctor lacus lacus nec ligula. Nam at turpis egestas, varius leo id, convallis elit. Etiam at sem augue. In ac arcu ornare, vehicula elit in, hendrerit lorem. In ut porta ipsum, in porta odio. Aenean risus augue, pulvinar non pharetra at, porta eu dolor. Sed varius mattis leo, at pellentesque turpis elementum ac.Mauris efficitur feugiat ex, a egestas metus. Proin orci dui, convallis sit amet malesuada at, suscipit id nulla. Curabitur ullamcorper, erat non porta vulputate, ipsum sem venenatis orci, quis auctor diam orci sit amet sem. Phasellus non iaculis mauris. In malesuada ligula nulla, vitae sollicitudin lectus egestas ut. Cras id convallis ante. Curabitur sit amet tristique turpis. Nam auctor sollicitudin velit, vel ullamcorper massa porta eget. Nam volutpat sapien quis lacinia tempus. Vestibulum est est, dictum et turpis a, venenatis mollis ante. Suspendisse quis ex dictum, finibus purus in, bibendum urna. Maecenas eu dui eu neque maximus maximus. Maecenas vehicula, nunc sed accumsan auctor, neque augue imperdiet dolor, id consequat felis erat ut mauris. Praesent iaculis vulputate elit, non semper sapien volutpat id. Donec nec blandit quam, id viverra odio.Nam congue ex sed risus interdum aliquam. Aliquam erat volutpat. Proin interdum ipsum in aliquet ullamcorper. Quisque commodo aliquam tortor, id elementum elit consectetur non. Praesent faucibus sagittis sagittis. Praesent in imperdiet augue. Nulla mattis felis at orci efficitur vulputate. Nulla tempor orci lacus, ut fringilla tortor scelerisque in. Phasellus pulvinar, dolor at rhoncus iaculis, enim magna mollis risus, at molestie libero nunc id ipsum. Morbi ultrices elementum enim aliquam vehicula. Donec vitae lacinia felis. Phasellus scelerisque eros sit amet justo vehicula, non aliquet sapien pretium. Integer finibus vel diam blandit viverra. Praesent pulvinar ipsum in lacus mattis, eget fringilla augue porttitor.Fusce sed purus eu purus malesuada mattis placerat et ipsum. Donec tortor augue, dignissim vel gravida sit amet, feugiat in lorem. Nam ultrices est ut tortor convallis, eu blandit nulla sagittis. Donec quis nunc eu ex scelerisque sagittis. Nunc egestas metus sed erat aliquet, et imperdiet nunc euismod. Aliquam ultricies tristique enim ut malesuada. Pellentesque a ante ut dui lobortis efficitur at at mi. Pellentesque scelerisque nisi a dolor elementum, at consectetur ipsum ornare. Nunc nec metus quis libero vehicula porta ut at ex. Suspendisse potenti. Sed eget venenatis risus.Maecenas fermentum, metus eget hendrerit faucibus, eros est laoreet velit, at gravida ipsum metus a eros. Pellentesque diam nibh, aliquam sit amet laoreet at, posuere at enim. Cras viverra, dolor a interdum ultrices, metus risus semper nulla, quis aliquam nisl lorem in nunc. Duis vel euismod sem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam id fermentum magna, eget semper nulla. Suspendisse magna diam, ultricies non odio a, vestibulum congue lorem. Nunc ipsum orci, volutpat a tortor nec, pulvinar pellentesque sapien. Nullam ultrices bibendum massa, viverra tincidunt ante sodales eget.Cras porttitor nibh a felis egestas finibus. Donec dapibus maximus sagittis. Nam ullamcorper ullamcorper convallis. Praesent eget faucibus libero, et maximus ligula. Curabitur eleifend, orci vitae facilisis vulputate, nisl enim iaculis libero, quis gravida lacus magna suscipit mauris. Curabitur efficitur dui interdum dictum suscipit. Maecenas facilisis efficitur est, quis interdum mauris tincidunt non. Sed eu odio ante. Morbi aliquam porta nunc, id finibus neque condimentum id. Nam pulvinar mi id elementum dictum. Maecenas pretium dolor malesuada erat bibendum, quis efficitur risus iaculis.Sed augue mi, bibendum non tempus vitae, vulputate sit amet arcu. In non nisl finibus, sodales ex quis, faucibus eros. Quisque luctus lectus placerat, feugiat leo malesuada, elementum enim. Nulla facilisi. Nunc vel venenatis augue. Quisque dui nisi, pretium quis mattis a, posuere ac felis. Vestibulum ut diam id nisi fermentum pretium. Maecenas commodo maximus aliquam. Morbi venenatis tortor ligula, quis efficitur arcu tincidunt eget. Aliquam accumsan euismod turpis, eget eleifend turpis rhoncus quis. Pellentesque auctor nulla tellus, eu finibus magna dapibus in. Curabitur orci turpis, semper et fermentum id, pretium id elit. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean semper neque id volutpat tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Sed in fermentum odio. Donec eget pretium ex. Donec egestas a mauris non pulvinar. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Integer ac sapien sed turpis gravida elementum. Aenean facilisis eros ac aliquet maximus. Proin ac auctor sapien, sit amet laoreet urna. Ut in accumsan lacus.Cras varius vitae diam non pulvinar. In dignissim, neque posuere interdum eleifend, ipsum arcu rutrum neque, id bibendum felis nulla et massa. Aenean eget placerat nisi. Sed nec faucibus magna. Vestibulum finibus volutpat venenatis. Vestibulum at facilisis leo. Ut luctus libero arcu, vel feugiat metus vulputate ac. Nam sit amet pharetra mi, ac molestie neque. Donec malesuada interdum nisi sit amet viverra. Mauris feugiat convallis est. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Curabitur a semper ipsum. Vivamus porta risus nec ex commodo bibendum. Proin vitae faucibus ex. Curabitur feugiat, est quis facilisis molestie, leo lorem placerat urna, vel euismod risus diam semper ipsum. Cras posuere id ante eget ultrices. Fusce vulputate, ante non suscipit dictum, nibh sem posuere lacus, at auctor nulla metus a odio. Vestibulum in ex nisl. Integer ut tellus tristique, feugiat orci sed, malesuada turpis. Sed sed tempor tellus, facilisis bibendum leo. Sed porta ac justo ac tincidunt.Morbi dignissim ex nibh, in suscipit urna ullamcorper ullamcorper. Nullam suscipit diam metus, sed euismod purus placerat a. Quisque eleifend semper nibh, et pulvinar nulla imperdiet ac. Proin euismod massa sit amet congue consequat. Quisque id ligula vel erat consequat consectetur. Integer eget purus sed mi accumsan bibendum. Phasellus lacinia ex eleifend tincidunt maximus. Etiam ultrices hendrerit purus, eu volutpat eros imperdiet id. Integer varius mi vel neque dignissim aliquet. Curabitur ac finibus odio. Mauris odio libero, consectetur ac congue nec, aliquet et ipsum. Aenean imperdiet lacus in odio porta aliquet. Pellentesque nec imperdiet nibh. Vestibulum placerat faucibus felis at accumsan. Sed et dignissim mi, eu dapibus nulla.Pellentesque volutpat feugiat vulputate. Vestibulum ac purus ac leo laoreet fermentum. Quisque eros augue, commodo non sem vel, convallis malesuada mauris. Vivamus ac ante eu mauris volutpat ornare eget et ex. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent nec placerat tellus, a dignissim magna. Nulla ut dui laoreet, blandit orci pretium, lacinia risus. Nullam bibendum euismod massa nec vestibulum. In sollicitudin sem massa. Morbi dapibus nisi diam, eget ullamcorper felis scelerisque in. Phasellus gravida dignissim est, eu eleifend lorem viverra at. Nam rhoncus, est placerat interdum commodo, neque diam consequat ante, vitae gravida nibh leo a nunc.Fusce at sapien diam. Pellentesque mollis, sem ac viverra fermentum, velit orci cursus odio, in rutrum odio nisi a metus. Phasellus maximus euismod mattis. Nullam magna urna, tempus tincidunt laoreet id, volutpat vel leo. Nunc vehicula placerat nulla, et tristique justo efficitur eu. Nulla interdum fermentum nisi eget egestas. Pellentesque a nulla eget tortor fringilla accumsan. Proin non tempor ligula. Sed commodo malesuada odio, ut vulputate arcu auctor in. Nunc tristique laoreet elit et lacinia. Aliquam erat volutpat. Phasellus libero purus, iaculis nec enim sed, finibus faucibus ante. Aenean dignissim lectus a ipsum blandit finibus.Vestibulum posuere dictum est non venenatis. Praesent quis libero at arcu aliquet laoreet a vel tortor. Donec dignissim arcu augue, vitae aliquam ante rutrum in. Fusce sollicitudin libero nunc, quis gravida lorem ullamcorper in. Etiam vulputate interdum enim vitae aliquet. Vestibulum eu luctus dui. Maecenas pulvinar id quam et aliquam. Aliquam in diam at nulla tristique ultrices efficitur quis massa. Nullam ultricies lobortis magna, non pharetra libero ornare et. In vitae mi ante.Duis suscipit et nisi eget mollis. Integer vel nisi eget quam luctus aliquam non vehicula est. Sed lacinia mi dolor, eu mollis est dictum quis. Integer magna felis, auctor tempus laoreet ac, tincidunt sit amet felis. Integer tristique congue viverra. Sed quis sem massa. Quisque vitae massa augue. Pellentesque semper, odio vitae ultricies semper, felis quam euismod velit, at tempus est dui a ligula. Aenean diam diam, fringilla tincidunt tempor malesuada, dictum vitae dui. Ut sed efficitur purus. Aenean ultricies sollicitudin risus ut auctor. Mauris sagittis bibendum ornare. Interdum et malesuada fames ac ante ipsum primis in faucibus.Fusce dolor massa, porta in sagittis sed, tristique pharetra lectus. Phasellus justo leo, posuere a magna id, suscipit lobortis felis. Sed et leo vitae mauris dignissim rhoncus. Suspendisse a erat ut felis viverra pretium ut id dolor. In vitae ullamcorper risus, non condimentum massa. Curabitur a tempor turpis, ac egestas turpis. Ut sodales ac diam ac fringilla. Aliquam erat volutpat. Aenean nec molestie ipsum. Curabitur vehicula eget libero sit amet eleifend. In dictum sodales urna ut maximus. Morbi at leo nec magna aliquet egestas. Ut viverra ornare nisl, et rhoncus elit rhoncus quis. Duis nulla metus, blandit vel cursus id, semper a nibh. Aenean eget facilisis magna. Nunc nisl purus, pharetra quis ex convallis, bibendum eleifend dui.Mauris cursus odio sit amet dui feugiat, imperdiet tincidunt odio finibus. Nulla ac pharetra urna. Duis gravida mauris sem, sed pulvinar dui sodales id. Ut lobortis tristique leo et ultrices. In auctor tempus metus maximus fringilla. Integer placerat dui enim, sed maximus mauris porta nec. In hac habitasse platea dictumst. Interdum et malesuada fames ac ante ipsum primis in faucibus. Quisque sagittis elit accumsan, fringilla velit at, ornare purus. Curabitur id bibendum dolor, vel vulputate dolor.Donec non scelerisque nisi. Ut ac mattis augue. Curabitur id commodo ante. Cras lobortis tempus erat non commodo. Pellentesque aliquet augue ac ex aliquam ultrices. Aenean efficitur porttitor leo, quis vestibulum quam tempus id. Nulla facilisi. Praesent velit risus, rutrum in ipsum nec, posuere auctor enim. In quis dictum risus, quis ullamcorper massa. Vivamus egestas fringilla quam, viverra ornare sem hendrerit nec.Aenean vestibulum elit ornare justo consequat, hendrerit congue nunc bibendum. Quisque at odio ut risus vehicula ullamcorper. Vestibulum fringilla ex libero, ac porta justo dictum eu. Praesent sit amet sapien lectus. Donec ut ultrices magna, nec tincidunt massa. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Mauris ullamcorper dui a imperdiet pharetra. Vivamus mauris nunc, rutrum eget turpis eu, scelerisque porttitor est. Etiam eget odio elit. Quisque urna diam, posuere eget efficitur quis, tincidunt non nulla. Nunc nec massa mauris.Proin porta, elit et fringilla pretium, massa erat finibus libero, laoreet pulvinar leo sem quis ligula. Donec purus arcu, pharetra nec convallis eu, eleifend ut urna. Donec urna leo, placerat eget vulputate sit amet, lacinia in lorem. Sed eget diam vel nisl volutpat convallis eget quis neque. Praesent gravida elementum neque, ac suscipit dolor tempor et. Maecenas id iaculis purus. Quisque non erat pulvinar, vulputate quam pharetra, vehicula nisi. Pellentesque ac mauris quis sem laoreet blandit eget et ante. Aliquam eleifend volutpat sem, in vehicula libero rhoncus eget. Sed pharetra, eros vitae tempus auctor, ipsum neque feugiat elit, non sodales augue leo ut leo. Nam tristique justo ut ornare bibendum. Donec euismod, tellus non imperdiet malesuada, purus eros ornare est, euismod laoreet eros elit vel nulla. Pellentesque convallis elementum libero, dignissim malesuada lectus ullamcorper nec. Vestibulum at massa vitae leo posuere commodo. Suspendisse tempus vitae mi vitae bibendum. Donec suscipit tristique elit, et accumsan nunc tincidunt eget.Nam nec tincidunt erat, ac hendrerit dolor. Morbi orci erat, maximus quis tempor ac, mattis id ligula. Sed eu elementum risus. Praesent commodo ipsum ut sem ornare facilisis. Etiam quis dapibus purus. Aenean quis ex porta, faucibus tortor a, scelerisque turpis. Aenean a tempus urna. Integer sit amet leo felis. Proin elementum urna at viverra luctus. Aliquam pulvinar, orci eget iaculis tempor, diam libero posuere ex, ac tincidunt leo quam a elit. Suspendisse sit amet massa aliquam ipsum venenatis iaculis. Praesent eget quam lorem.Suspendisse potenti. Sed ac ante non elit fringilla dignissim vitae posuee tortor. Duis congue mattis justo vitae tristique. Nam vitae semper justo, vel tempor tortor. Etiam laoreet quam magna, mollis vulputate lorem lacinia vel. Quisque suscipit congue porta. Phasellus id felis id dolor convallis feugiat. Fusce finibus lobortis diam, quis lacinia nisi blandit eu. Morbi nec gravida magna.Donec aliquam libero tortor, vel sollicitudin dui tristique id. Nullam lacinia leo libero, vitae vestibulum justo semper et. Proin dictum maximus urna. Curabitur fringilla sit amet tortor id bibendum. Integer dignissim sapien vitae lacus sodales bibendum. Vestibulum sem magna, elementum quis eros id, feugiat convallis justo. In hendrerit porttitor justo vitae scelerisque.Aliquam malesuada massa ac risus molestie scelerisque. Cras fermentum commodo massa, non lobortis libero gravida sit amet. Nunc porta augue quis augue rutrum ultricies. Ut pulvinar sapien fringilla auctor consectetur. Maecenas non tincidunt nibh, quis venenatis arcu. Phasellus in ultrices sapien. Sed tempor tristique placerat. Aenean maximus, ipsum eu volutpat commodo, arcu nisl cursus dui, id molestie lectus magna non ex. In hac habitasse platea dictumst. Praesent id aliquet dolor.Integer lobortis mauris eget nibh tristique, vel ultrices tortor scelerisque. Phasellus sed neque turpis. Etiam et ornare libero, a imperdiet est. Sed interdum arcu a lectus sagittis faucibus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Curabitur nec ante a nisl eleifend scelerisque. Nam lorem est, finibus nec tortor dignissim, auctor cursus est. Duis fringilla ac nulla et aliquet. Aliquam blandit orci sed lectus egestas porttitor et et lectus. Nunc volutpat luctus nisl non efficitur. Fusce purus turpis, ultricies nec lacinia a, hendrerit vitae justo.Phasellus leo odio, dapibus sed est sed, dignissim interdum ante. Donec dignissim luctus nisl, et suscipit arcu consectetur vitae. Fusce vestibulum laoreet consequat. Donec dui orci, scelerisque eu efficitur in, condimentum eu dolor. Cras pulvinar nisi id nibh accumsan, ac tincidunt arcu tincidunt. Fusce et nisl non eros semper lobortis. Aenean vitae fermentum lorem, sed imperdiet nibh. Cras vitae blandit nisi. Cras vitae fermentum ipsum. Etiam porta lorem nec turpis varius pulvinar. Mauris luctus mauris tristique, ultrices dui quis, consectetur nunc.Cras sapien felis, vestibulum non justo ut, varius egestas nisi. Suspendisse maximus augue nunc, nec congue sem sollicitudin ultrices. Etiam ut maximus augue. Aenean gravida hendrerit augue cursus malesuada. Sed vitae dictum quam. In euismod pulvinar tellus eget ullamcorper. Interdum et malesuada fames ac ante ipsum primis in faucibus. Suspendisse at orci ex. Nam egestas lectus vel hendrerit pulvinar. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; In bibendum neque vestibulum nibh tempor tristique. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Pellentesque et risus a tellus cursus pretium. Sed vitae elit interdum tortor varius commodo eu in libero.Integer in elit ut lacus viverra vestibulum. Vivamus aliquam ullamcorper felis. Phasellus porttitor interdum ligula, sed consectetur nisl euismod eu. Duis quis tempus turpis. Mauris pellentesque velit a pellentesque mattis. Nam non ornare urna. Vestibulum lacus lorem, vehicula et est in, posuere vehicula est. Morbi felis nibh, interdum ut massa vitae, feugiat aliquet turpis. Maecenas commodo nibh urna, eget placerat lacus luctus eu. Morbi varius magna vel dignissim varius. Aliquam nunc erat, finibus vitae posuere sit amet, maximus a lectus. Fusce quam orci, aliquam eu efficitur sed, auctor in lorem. In et dolor id neque placerat luctus rhoncus in nunc. Nunc scelerisque risus non tincidunt vulputate. Aliquam arcu nulla, pharetra tempus sapien a, pharetra posuere augue.Interdum et malesuada fames ac ante ipsum primis in faucibus. Ut rutrum dictum orci, eu molestie risus tristique vitae. Nullam erat mauris, molestie et nulla eu, rutrum finibus dui. Nulla tortor tortor, rhoncus quis posuere sed, mollis at lacus. Sed varius commodo justo, vel aliquam nisl accumsan ut. In id lorem est. Quisque a nisl a nulla venenatis rhoncus. Integer nec tincidunt metus. Etiam sit amet neque dui. In aliquam, risus vel pharetra varius, odio velit laoreet purus, eget molestie eros lectus a ex.Maecenas auctor pretium ipsum, a mollis nulla laoreet nec. Sed ut turpis eleifend, dapibus felis nec, sagittis risus. Quisque in mauris dui. Nam in pharetra quam. Maecenas luctus hendrerit urna sit amet consectetur. Suspendisse in mattis lectus, vitae imperdiet nunc. Cras consequat mi ligula, non congue sem lacinia et. Vestibulum tristique fringilla aliquam. Etiam hendrerit, mi ut rhoncus laoreet, libero mauris auctor magna, sit amet varius diam ligula eget odio. In lacinia porttitor magna ac auctor.Vestibulum eleifend, odio quis venenatis cursus, mauris erat dignissim lorem, nec convallis lectus justo non lorem. Sed in dolor eget enim placerat dapibus et ac lorem. Interdum et malesuada fames ac ante ipsum primis in faucibus. Duis bibendum orci nec molestie posuere. In sit amet tortor blandit, viverra est quis, tempus ante. Morbi quis malesuada massa. Quisque sit amet nisi congue, auctor massa quis, hendrerit est. Praesent tincidunt ante in condimentum egestas. Vivamus accumsan molestie tempus. Mauris placerat consequat congue. Duis purus odio, fermentum ut sem ac, sollicitudin imperdiet libero. Suspendisse fringilla suscipit ante. Donec sit amet sagittis massa, non rutrum sapien. Fusce accumsan posuere nisl non placerat. Quisque a lectus ipsum.Vestibulum tincidunt urna neque, ac posuere risus dapibus ultrices. In aliquet dolor sed tempor accumsan. Cras eu dolor placerat felis iaculis ultricies sit amet vel sem. Morbi nec nibh a est tincidunt ultricies. Praesent dignissim tristique sapien. Sed porta, lorem in posuere finibus, ipsum ligula fringilla orci, in rutrum tellus ante id orci. Etiam in suscipit dui. Nullam ultrices neque sit amet elit lacinia consectetur et eu nulla. Nulla lacinia mi purus, at facilisis libero egestas at. Donec porta finibus magna, eget efficitur magna ultrices a. Donec gravida nibh vel sollicitudin interdum.Nulla id ex convallis, luctus purus quis, aliquet ligula. Quisque nec finibus lorem. Phasellus nec faucibus metus, in laoreet magna. Cras tortor elit, interdum vel facilisis nec, blandit a ex. Fusce eu massa convallis mi iaculis blandit. Quisque viverra mi non ante commodo iaculis. Cras tincidunt ornare velit, eu lobortis erat euismod eget. Vivamus dignissim mollis odio at accumsan. Nulla ut neque vehicula, tempor dui quis, aliquam ipsum. Curabitur convallis mi non auctor dictum. Mauris viverra commodo rutrum. Mauris scelerisque nibh ullamcorper ipsum congue viverra. Morbi sagittis scelerisque turpis, quis efficitur nibh. Maecenas in erat justo. Sed cursus tellus id tortor ullamcorper, et congue purus finibus.Vivamus nec leo commodo, sodales ligula vulputate, tempus ex. Fusce lacinia diam elit, ac lobortis enim pretium rutrum. Pellentesque condimentum justo vitae ipsum convallis finibus. Donec quis sollicitudin elit, sit amet hendrerit ipsum. Mauris consectetur lorem velit, non finibus lorem dapibus ac. Cras quis est ut risus tempus ultricies ut ut purus. Suspendisse cursus velit at porttitor sagittis. Mauris non magna vel dolor vehicula vestibulum. Morbi gravida mattis lectus, vitae imperdiet neque porta eget. Nullam euismod cursus leo accumsan tempor. Integer vulputate ante et ante pellentesque viverra. Donec at massa eu massa sodales tempus.Cras nec pharetra quam, non dapibus enim. Aenean vel nunc venenatis, condimentum orci quis, tincidunt odio. Cras euismod justo ut dui lobortis ornare. Vivamus fringilla, enim eu congue porta, orci quam dictum sem, ac efficitur velit est eu nulla. Nulla mi nibh, varius malesuada ornare nec, venenatis porttitor felis. Aenean id maximus quam, non pharetra mi. Quisque ullamcorper ex risus, et cursus nisi volutpat eget. Maecenas sapien lorem, varius vitae lorem sit amet, dapibus ullamcorper elit. Pellentesque convallis velit quis dui ullamcorper, non cursus lectus accumsan. Mauris faucibus nisi sit amet tristique placerat. Cras luctus diam ut magna ultricies, et feugiat mauris pulvinar. Quisque cursus imperdiet ultricies.Praesent sodales metus et augue pharetra, at vehicula nulla tristique. Integer et dictum dui. Aenean eget libero aliquam, fermentum justo id, mattis mauris. Mauris lobortis diam metus, ut iaculis orci convallis vel. Proin ante mauris, sagittis ac orci ac, tincidunt sagittis ligula. Vestibulum lobortis nisi a ligula laoreet iaculis. Sed dignissim dolor diam, eget lacinia est sagittis et. Morbi elementum sodales ex eu facilisis. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nam nec consequat turpis, ultrices ornare quam. Integer nunc tellus, porttitor nec dolor vel, elementum tincidunt mauris. Pellentesque varius feugiat ex mollis fermentum.Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Proin quis hendrerit neque. Etiam tempor dolor in tellus tincidunt, a gravida sem ultrices. Nunc placerat turpis felis, nec dictum est pretium eu. Maecenas ullamcorper at nulla vel elementum. Maecenas sed tincidunt dolor. Interdum et malesuada fames ac ante ipsum primis in faucibus. Aenean cursus ligula arcu, ut molestie dui eleifend nec. Praesent faucibus sem leo, sit amet placerat sapien porttitor in. Mauris viverra dignissim tempor. Ut at mi ligula. Mauris faucibus ullamcorper dui in luctus. Pellentesque a porttitor ligula. Vestibulum interdum, tellus eget consectetur consectetur, nulla urna consectetur sem, non efficitur eros nisi in quam.Sed finibus dapibus massa, quis commodo elit lobortis et. Nulla mollis finibus luctus. Aliquam erat volutpat. Donec lacus lacus, consectetur sit amet pulvinar nec, porttitor sed mauris. Aliquam elementum in sapien nec aliquam. Fusce non ex ut ex iaculis rutrum. Aenean viverra arcu non elit posuere aliquam. Aenean magna massa, euismod in diam sit amet, interdum consectetur massa. Vivamus in imperdiet elit, eget vestibulum sem. Aliquam interdum diam a nulla lacinia, quis pellentesque lorem ullamcorper. Nullam rutrum, est sagittis sodales sollicitudin, tellus neque vestibulum mi, eu molestie leo elit eget justo. Integer justo tellus, efficitur at commodo sed, tempus sed mauris. Quisque iaculis ut eros in pretium. Donec in bibendum mauris, quis eleifend erat. Pellentesque aliquam dignissim varius. Etiam gravida finibus mi a aliquam.Morbi malesuada elit vel maximus hendrerit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus condimentum porta elit. Aenean mollis tellus ut urna venenatis ornare. Maecenas at scelerisque ipsum. Morbi pharetra placerat dolor, sit amet fringilla nisl posuere scelerisque. Curabitur pharetra mi at sollicitudin interdum. Nunc egestas sodales fringilla. Morbi condimentum ornare lobortis. Donec sed sodales purus, id cursus mauris. Suspendisse sit amet molestie turpis.Duis ullamcorper sem at nisl vehicula ornare. Phasellus fermentum felis et eros hendrerit cursus. Vivamus mattis lectus nec erat cursus facilisis. Suspendisse sit amet ullamcorper lorem. Aliquam sodales neque at turpis convallis volutpat. Integer consequat pellentesque tellus, et vestibulum justo lacinia a. Sed pulvinar imperdiet cursus. Nunc odio tellus, gravida ut ex sit amet, eleifend tristique massa. Pellentesque pharetra vel est sed efficitur. Curabitur viverra mauris eget metus hendrerit, ut hendrerit est vestibulum.Integer vel aliquet enim. Praesent eu congue quam. Suspendisse hendrerit eros ut ante fringilla varius. Nunc tristique facilisis urna. Nullam eget urna eget orci iaculis scelerisque egestas id ipsum. Proin non turpis ipsum. Curabitur congue lorem rutrum ligula mollis, malesuada hendrerit enim rutrum. Sed tortor justo, aliquam ac odio eget, condimentum condimentum ligula. Mauris condimentum elit magna, at vehicula arcu vehicula pellentesque.Integer placerat velit faucibus diam consectetur laoreet. Vivamus pretium lectus ac consectetur iaculis. Nulla facilisi. Sed varius nibh eget lorem commodo, a porttitor purus ornare. Mauris dapibus nulla eu dolor tristique, quis malesuada sem porta. Sed id venenatis quam, non cursus eros. Vivamus placerat pretium ligula, vel fermentum ligula suscipit at. Morbi viverra vel nulla et porttitor. Quisque mollis ultrices lorem et venenatis. Sed consectetur est nisl, non egestas orci porttitor eu. Donec id erat nec ligula rutrum dapibus eget at enim.Vestibulum leo mi, tincidunt vel ante id, posuere cursus ligula. Aenean laoreet interdum lorem, eget condimentum purus feugiat a. Praesent sed tristique purus. Maecenas rutrum posuere diam, sed posuere est auctor vel. Sed lobortis odio quis dolor ultricies, vitae tincidunt ipsum molestie. Pellentesque molestie arcu quis hendrerit condimentum. Etiam iaculis eleifend vulputate. Donec lobortis eu quam ac dapibus. Morbi vel molestie nisi. Donec quis sagittis urna, congue aliquam velit.Duis interdum mollis vehicula. Aenean iaculis eros sit amet odio cursus placerat. Proin porttitor risus orci, nec porttitor diam facilisis a. Fusce cursus, nisi vitae imperdiet sodales, justo leo luctus velit, eu luctus orci velit aliquet eros. Nulla facilisi. Integer egestas maximus ex, quis malesuada tellus laoreet sit amet. Suspendisse lacinia neque ac imperdiet aliquet.Vivamus vitae enim ex. Donec vehicula luctus sapien, id ultricies libero sodales quis. Aenean at diam nisi. Cras suscipit at lacus sed volutpat. Vivamus eu gravida justo. Praesent id rutrum eros. Aenean vel orci elit. Nam venenatis metus luctus, dictum magna sed, volutpat neque. Praesent id arcu iaculis, pretium orci a, tincidunt urna. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Curabitur posuere ut nisl vel rhoncus. Nulla facilisi. Quisque sed efficitur lacus.Quisque a diam sagittis velit sagittis aliquam. Donec cursus, ligula non mollis efficitur, libero lorem luctus eros, et tempor dolor metus eget magna. Cras facilisis mollis mi a rhoncus. Mauris ligula arcu, posuere viverra mi a, lobortis semper augue. Aliquam vitae turpis et tellus vehicula pharetra quis quis ex. Quisque diam odio, condimentum sed sem sit amet, condimentum rutrum turpis. Phasellus ligula orci, volutpat a eros in, pharetra pharetra lectus.Aliquam condimentum vestibulum risus, eget commodo est cursus nec. Cras finibus, eros vitae molestie scelerisque, justo tellus efficitur erat, et faucibus ipsum felis eu justo. Curabitur tincidunt et diam sed molestie. Nulla ornare dui varius eros aliquam, blandit porttitor lectus convallis. Ut id lectus nisi. Phasellus tellus enim, hendrerit quis purus ut, tempor interdum tortor. Quisque pretium nunc neque, ut vehicula ante consectetur eu. Cras eget velit sit amet ipsum pharetra pretium id quis quam.Curabitur eget finibus neque. Praesent suscipit eros ex, eu tempor est facilisis nec. Donec congue risus mi, efficitur semper nunc convallis sit amet. Vestibulum at accumsan magna, eu maximus massa. Quisque aliquet ipsum est, vitae porttitor eros semper ac. Nunc at dui urna. Nunc tempor pulvinar felis ac dignissim. Nulla leo felis, blandit non lectus sed, ultrices lacinia mauris. Cras ultrices commodo commodo.Morbi molestie ante a sem efficitur hendrerit. Sed pretium dignissim erat in imperdiet. Proin non arcu non ante ullamcorper accumsan. Nulla eleifend tempus nunc, in venenatis ex finibus nec. Phasellus eget elit lorem. Etiam maximus erat et velit efficitur laoreet. Donec porttitor, turpis nec consectetur malesuada, massa velit lacinia ipsum, efficitur laoreet turpis tortor eu tellus. Fusce dignissim dolor nec aliquet lobortis. Vivamus sollicitudin justo ut finibus aliquam. Duis massa nunc.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur convallis sapien eu libero dictum fermentum. Nulla maximus dignissim justo, id sagittis velit tincidunt non. Ut sed accumsan sem, ut rhoncus dolor. In sit amet suscipit sem, id dictum nisl. Pellentesque at sem lacinia, consequat massa a, viverra arcu. Curabitur eget sagittis diam. Maecenas ac ante neque. Nam fermentum lacus arcu, vitae accumsan augue porta in. Aliquam facilisis aliquet nisi, eget auctor nulla vulputate id. Aenean ultrices, nisl id fermentum laoreet, ex diam tincidunt nisl, at sagittis tellus ex ut magna. Mauris elementum sit amet urna non imperdiet. Donec id velit vitae lectus congue ornare. Pellentesque tristique suscipit interdum. Integer luctus iaculis imperdiet. Duis et sapien odio. Nulla scelerisque tempor massa, ac faucibus ligula malesuada id.Curabitur convallis felis lorem, nec tristique diam molestie sed. Phasellus mollis fermentum lacus sed elementum. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi atpurus nec nulla iaculis tempus. Donec molestie turpis neque, id fringilla mi viverra id. Cras ex est, hendrerit eu turpis pretium, vehicula egestas odio. Nullam a gravida urna. Curabitur in finibus sem, non bibendum risus. Donec tempor vitae ligula quis aliquet. Nullam eget est a dui placerat sollicitudin et a nulla. Nam facilisis volutpat lectus nec sodales. Aliquam at vestibulum eros.Proin pharetra facilisis turpis quis iaculis. Donec dignissim egestas lectus, non ullamcorper orci pretium non. Vivamus vel sodales augue. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Quisque feugiat, odio interdum faucibus laoreet, magna metus consectetur nisi, sit amet ultrices dui eros elementum neque. Nunc laoreet at sapien vitae mollis. Morbi porta massa nibh, ac dictum nulla elementum feugiat. Vivamus ultricies tortor convallis, semper lacus vel, consequat nibh. Nulla eleifend felis nec lobortis sagittis. Etiam quam nisi, ullamcorper sed ex id, gravida sagittis quam. Nunc eu auctor nisi. Praesent pellentesque, magna at sodales pellentesque, dui elit vulputate erat, eu vulputate enim turpis ac nisl. Proin vitae ipsum eu dolor lacinia lobortis. Sed nec semper sapien. Nullam lacus augue, porttitor et turpis ac, aliquet malesuada justo.Suspendisse id tristique quam. Nulla in erat eleifend, aliquet ex sit amet, molestie magna. Curabitur dolor eros, fermentum quis sagittis et, tristique eu purus. Sed pellentesque nisi sit amet metus luctus ornare. Integer iaculis mollis lorem, a tincidunt mauris iaculis eget. Nunc lobortis ante eget velit tristique, id convallis mi blandit. Nam euismod interdum enim, sed viverra nibh iaculis eu. Morbi ex nunc, blandit porta efficitur sit amet, dictum ut neque. Quisque porttitor neque ac risus iaculis, vel pharetra mi rhoncus. Sed hendrerit sodales nulla, in dictum velit pellentesque eu. Praesent massa felis, tincidunt eu ornare a, placerat eu dui. Maecenas tortor nulla, fermentum non pellentesque vel, ornare a dui. Quisque eu est libero. Pellentesque varius sodales nibh vitae fermentum.Vivamus tincidunt, erat eu faucibus congue, lacus turpis efficitur enim, at auctor lacus lacus nec ligula. Nam at turpis egestas, varius leo id, convallis elit. Etiam at sem augue. In ac arcu ornare, vehicula elit in, hendrerit lorem. In ut porta ipsum, in porta odio. Aenean risus augue, pulvinar non pharetra at, porta eu dolor. Sed varius mattis leo, at pellentesque turpis elementum ac.Mauris efficitur feugiat ex, a egestas metus. Proin orci dui, convallis sit amet malesuada at, suscipit id nulla. Curabitur ullamcorper, erat non porta vulputate, ipsum sem venenatis orci, quis auctor diam orci sit amet sem. Phasellus non iaculis mauris. In malesuada ligula nulla, vitae sollicitudin lectus egestas ut. Cras id convallis ante. Curabitur sit amet tristique turpis. Nam auctor sollicitudin velit, vel ullamcorper massa porta eget. Nam volutpat sapien quis lacinia tempus. Vestibulum est est, dictum et turpis a, venenatis mollis ante. Suspendisse quis ex dictum, finibus purus in, bibendum urna. Maecenas eu dui eu neque maximus maximus. Maecenas vehicula, nunc sed accumsan auctor, neque augue imperdiet dolor, id consequat felis erat ut mauris. Praesent iaculis vulputate elit, non semper sapien volutpat id. Donec nec blandit quam, id viverra odio.Nam congue ex sed risus interdum aliquam. Aliquam erat volutpat. Proin interdum ipsum in aliquet ullamcorper. Quisque commodo aliquam tortor, id elementum elit consectetur non. Praesent faucibus sagittis sagittis. Praesent in imperdiet augue. Nulla mattis felis at orci efficitur vulputate. Nulla tempor orci lacus, ut fringilla tortor scelerisque in. Phasellus pulvinar, dolor at rhoncus iaculis, enim magna mollis risus, at molestie libero nunc id ipsum. Morbi ultrices elementum enim aliquam vehicula. Donec vitae lacinia felis. Phasellus scelerisque eros sit amet justo vehicula, non aliquet sapien pretium. Integer finibus vel diam blandit viverra. Praesent pulvinar ipsum in lacus mattis, eget fringilla augue porttitor.Fusce sed purus eu purus malesuada mattis placerat et ipsum. Donec tortor augue, dignissim vel gravida sit amet, feugiat in lorem. Nam ultrices est ut tortor convallis, eu blandit nulla sagittis. Donec quis nunc eu ex scelerisque sagittis. Nunc egestas metus sed erat aliquet, et imperdiet nunc euismod. Aliquam ultricies tristique enim ut malesuada. Pellentesque a ante ut dui lobortis efficitur at at mi. Pellentesque scelerisque nisi a dolor elementum, at consectetur ipsum ornare. Nunc nec metus quis libero vehicula porta ut at ex. Suspendisse potenti. Sed eget venenatis risus.Maecenas fermentum, metus eget hendrerit faucibus, eros est laoreet velit, at gravida ipsum metus a eros. Pellentesque diam nibh, aliquam sit amet laoreet at, posuere at enim. Cras viverra, dolor a interdum ultrices, metus risus semper nulla, quis aliquam nisl lorem in nunc. Duis vel euismod sem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam id fermentum magna, eget semper nulla. Suspendisse magna diam, ultricies non odio a, vestibulum congue lorem. Nunc ipsum orci, volutpat a tortor nec, pulvinar pellentesque sapien. Nullam ultrices bibendum massa, viverra tincidunt ante sodales eget.Cras porttitor nibh a felis egestas finibus. Donec dapibus maximus sagittis. Nam ullamcorper ullamcorper convallis. Praesent eget faucibus libero, et maximus ligula. Curabitur eleifend, orci vitae facilisis vulputate, nisl enim iaculis libero, quis gravida lacus magna suscipit mauris. Curabitur efficitur dui interdum dictum suscipit. Maecenas facilisis efficitur est, quis interdum mauris tincidunt non. Sed eu odio ante. Morbi aliquam porta nunc, id finibus neque condimentum id. Nam pulvinar mi id elementum dictum. Maecenas pretium dolor malesuada erat bibendum, quis efficitur risus iaculis.Sed augue mi, bibendum non tempus vitae, vulputate sit amet arcu. In non nisl finibus, sodales ex quis, faucibus eros. Quisque luctus lectus placerat, feugiat leo malesuada, elementum enim. Nulla facilisi. Nunc vel venenatis augue. Quisque dui nisi, pretium quis mattis a, posuere ac felis. Vestibulum ut diam id nisi fermentum pretium. Maecenas commodo maximus aliquam. Morbi venenatis tortor ligula, quis efficitur arcu tincidunt eget. Aliquam accumsan euismod turpis, eget eleifend turpis rhoncus quis. Pellentesque auctor nulla tellus, eu finibus magna dapibus in. Curabitur orci turpis, semper et fermentum id, pretium id elit. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Aenean semper neque id volutpat tincidunt. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Sed in fermentum odio. Donec eget pretium ex. Donec egestas a mauris non pulvinar. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Integer ac sapien sed turpis gravida elementum. Aenean facilisis eros ac aliquet maximus. Proin ac auctor sapien, sit amet laoreet urna. Ut in accumsan lacus.Cras varius vitae diam non pulvinar. In dignissim, neque posuere interdum eleifend, ipsum arcu rutrum neque, id bibendum felis nulla et massa. Aenean eget placerat nisi. Sed nec faucibus magna. Vestibulum finibus volutpat venenatis. Vestibulum at facilisis leo. Ut luctus libero arcu, vel feugiat metus vulputate ac. Nam sit amet pharetra mi, ac molestie neque. Donec malesuada interdum nisi sit amet viverra. Mauris feugiat convallis est. Lorem ipsum dolor sit amet, consectetur adipiscing elit.Curabitur a semper ipsum. Vivamus porta risus nec ex commodo bibendum. Proin vitae faucibus ex. Curabitur feugiat, est quis facilisis molestie, leo lorem placerat urna, vel euismod risus diam semper ipsum. Cras posuere id ante eget ultrices. Fusce vulputate, ante non suscipit dictum, nibh sem posuere lacus, at auctor nulla metus a odio. Vestibulum in ex nisl. Integer ut tellus tristique, feugiat orci sed, malesuada turpis. Sed sed tempor tellus, facilisis bibendum leo. Sed porta ac justo ac tincidunt.Morbi dignissim ex nibh, in suscipit urna ullamcorper ullamcorper. Nullam suscipit diam metus, sed euismod purus placerat a. Quisque eleifend semper nibh, et pulvinar nulla imperdiet ac. Proin euismod massa sit amet congue consequat. Quisque id ligula vel erat consequat consectetur. Integer eget purus sed mi accumsan bibendum. Phasellus lacinia ex eleifend tincidunt maximus. Etiam ultrices hendrerit purus, eu volutpat eros imperdiet id. Integer varius mi vel neque dignissim aliquet. Curabitur ac finibus odio. Mauris odio libero, consectetur ac congue nec, aliquet et ipsum. Aenean imperdiet lacus in odio porta aliquet. Pellentesque nec imperdiet nibh. Vestibulum placerat faucibus felis at accumsan. Sed et dignissim mi, eu dapibus nulla.Pellentesque volutpat feugiat vulputate. Vestibulum ac purus ac leo laoreet fermentum. Quisque eros augue, commodo non sem vel, convallis malesuada mauris. Vivamus ac ante eu mauris volutpat ornare eget et ex. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent nec placerat tellus, a dignissim magna. Nulla ut dui laoreet, blandit orci pretium, lacinia risus. Nullam bibendum euismod massa nec vestibulum. In sollicitudin sem massa. Morbi dapibus nisi diam, eget ullamcorper felis scelerisque in. Phasellus gravida dignissim est, eu eleifend lorem viverra at. Nam rhoncus, est placerat interdum commodo, neque diam consequat ante, vitae gravida nibh leo a nunc.Fusce at sapien diam. Pellentesque mollis, sem ac viverra fermentum, velit orci cursus odio, in rutrum odio nisi a metus. Phasellus maximus euismod mattis. Nullam magna urna, tempus tincidunt laoreet id, volutpat vel leo. Nunc vehicula placerat nulla, et tristique justo efficitur eu. Nulla interdum fermentum nisi eget egestas. Pellentesque a nulla eget tortor fringilla accumsan. Proin non tempor ligula. Sed commodo malesuada odio, ut vulputate arcu auctor in. Nunc tristique laoreet elit et lacinia. Aliquam erat volutpat. Phasellus libero purus, iaculis nec enim sed, finibus faucibus ante. Aenean dignissim lectus a ipsum blandit finibus.Vestibulum posuere dictum est non venenatis. Praesent quis libero at arcu aliquet laoreet a vel tortor. Donec dignissim arcu augue, vitae aliquam ante rutrum in. Fusce sollicitudin libero nunc, quis gravida lorem ullamcorper in. Etiam vulputate interdum enim vitae aliquet. Vestibulum eu luctus dui. Maecenas pulvinar id quam et aliquam. Aliquam in diam at nulla tristique ultrices efficitur quis massa. Nullam ultricies lobortis magna, non pharetra libero ornare et. In vitae mi ante.Duis suscipit et nisi eget mollis. Integer vel nisi eget quam luctus aliquam non vehicula est. Sed lacinia mi dolor, eu mollis est dictum quis. Integer magna felis, auctor tempus laoreet ac, tincidunt sit amet felis. Integer tristique congue viverra. Sed quis sem massa. Quisque vitae massa augue. Pellentesque semper, odio vitae ultricies semper, felis quam euismod velit, at tempus est dui a ligula. Aenean diam diam, fringilla tincidunt tempor malesuada, dictum vitae dui. Ut sed efficitur purus. Aenean ultricies sollicitudin risus ut auctor. Mauris sagittis bibendum ornare. Interdum et malesuada fames ac ante ipsum primis in faucibus.Fusce dolor massa, porta in sagittis sed, tristique pharetra lectus. Phasellus justo leo, posuere a magna id, suscipit lobortis felis. Sed et leo vitae mauris dignissim rhoncus. Suspendisse a erat ut felis viverra pretium ut id dolor. In vitae ullamcorper risus, non condimentum massa. Curabitur a tempor turpis, ac egestas turpis. Ut sodales ac diam ac fringilla. Aliquam erat volutpat. Aenean nec molestie ipsum. Curabitur vehicula eget libero sit amet eleifend. In dictum sodales urna ut maximus. Morbi at leo nec magna aliquet egestas. Ut viverra ornare nisl, et rhoncus elit rhoncus quis. Duis nulla metus, blandit vel cursus id, semper a nibh. Aenean eget facilisis magna. Nunc nisl purus, pharetra quis ex convallis, bibendum eleifend dui.Mauris cursus odio sit amet dui feugiat, imperdiet tincidunt odio finibus. Nulla ac pharetra urna. Duis gravida mauris sem, sed pulvinar dui sodales id. Ut lobortis tristique leo et ultrices. In auctor tempus metus maximus fringilla. Integer placerat dui enim, sed maximus mauris porta nec. In hac habitasse platea dictumst. Interdum et malesuada fames ac ante ipsum primis in faucibus. Quisque sagittis elit accumsan, fringilla velit at, ornare purus. Curabitur id bibendum dolor, vel vulputate dolor.Donec non scelerisque nisi. Ut ac mattis augue. Curabitur id commodo ante. Cras lobortis tempus erat non commodo. Pellentesque aliquet augue ac ex aliquam ultrices. Aenean efficitur porttitor leo, quis vestibulum quam tempus id. Nulla facilisi. Praesent velit risus, rutrum in ipsum nec, posuere auctor enim. In quis dictum risus, quis ullamcorper massa. Vivamus egestas fringilla quam, viverra ornare sem hendrerit nec.Aenean vestibulum elit ornare justo consequat, hendrerit congue nunc bibendum. Quisque at odio ut risus vehicula ullamcorper. Vestibulum fringilla ex libero, ac porta justo dictum eu. Praesent sit amet sapien lectus. Donec ut ultrices magna, nec tincidunt massa. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Mauris ullamcorper dui a imperdiet pharetra. Vivamus mauris nunc, rutrum eget turpis eu, scelerisque porttitor est. Etiam eget odio elit. Quisque urna diam, posuere eget efficitur quis, tincidunt non nulla. Nunc nec massa mauris.Proin porta, elit et fringilla pretium, massa erat finibus libero, laoreet pulvinar leo sem quis ligula. Donec purus arcu, pharetra nec convallis eu, eleifend ut urna. Donec urna leo, placerat eget vulputate sit amet, lacinia in lorem. Sed eget diam vel nisl volutpat convallis eget quis neque. Praesent gravida elementum neque, ac suscipit dolor tempor et. Maecenas id iaculis purus. Quisque non erat pulvinar, vulputate quam pharetra, vehicula nisi. Pellentesque ac mauris quis sem laoreet blandit eget et ante. Aliquam eleifend volutpat sem, in vehicula libero rhoncus eget. Sed pharetra, eros vitae tempus auctor, ipsum neque feugiat elit, non sodales augue leo ut leo. Nam tristique justo ut ornare bibendum. Donec euismod, tellus non imperdiet malesuada, purus eros ornare est, euismod laoreet eros elit vel nulla. Pellentesque convallis elementum libero, dignissim malesuada lectus ullamcorper nec. Vestibulum at massa vitae leo posuere commodo. Suspendisse tempus vitae mi vitae bibendum. Donec suscipit tristique elit, et accumsan nunc tincidunt eget.Nam nec tincidunt erat, ac hendrerit dolor. Morbi orci erat, maximus quis tempor ac, mattis id ligula. Sed eu elementum risus. Praesent commodo ipsum ut sem ornare facilisis. Etiam quis dapibus purus. Aenean quis ex porta, faucibus tortor a, scelerisque turpis. Aenean a tempus urna. Integer sit amet leo felis. Proin elementum urna at viverra luctus. Aliquam pulvinar, orci eget iaculis tempor, diam libero posuere ex, ac tincidunt leo quam a elit. Suspendisse sit amet massa aliquam ipsum venenatis iaculis. Praesent eget quam lorem.Suspendisse potenti. Sed ac ante non elit fringilla dignissim vitae posuee tortor. Duis congue mattis justo vitae tristique. Nam vitae semper justo, vel tempor tortor. Etiam laoreet quam magna, mollis vulputate lorem lacinia vel. Quisque suscipit congue porta. Phasellus id felis id dolor convallis feugiat. Fusce finibus lobortis diam, quis lacinia nisi blandit eu. Morbi nec gravida magna.Donec aliquam libero tortor, vel sollicitudin dui tristique id. Nullam lacinia leo libero, vitae vestibulum justo semper et. Proin dictum maximus urna. Curabitur fringilla sit amet tortor id bibendum. Integer dignissim sapien vitae lacus sodales bibendum. Vestibulum sem magna, elementum quis eros id, feugiat convallis justo. In hendrerit porttitor justo vitae scelerisque.Aliquam malesuada massa ac risus molestie scelerisque. Cras fermentum commodo massa, non lobortis libero gravida sit amet. Nunc porta augue quis augue rutrum ultricies. Ut pulvinar sapien fringilla auctor consectetur. Maecenas non tincidunt nibh, quis venenatis arcu. Phasellus in ultrices sapien. Sed tempor tristique placerat. Aenean maximus, ipsum eu volutpat commodo, arcu nisl cursus dui, id molestie lectus magna non ex. In hac habitasse platea dictumst. Praesent id aliquet dolor.Integer lobortis mauris eget nibh tristique, vel ultrices tortor scelerisque. Phasellus sed neque turpis. Etiam et ornare libero, a imperdiet est. Sed interdum arcu a lectus sagittis faucibus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Curabitur nec ante a nisl eleifend scelerisque. Nam lorem est, finibus nec tortor dignissim, auctor cursus est. Duis fringilla ac nulla et aliquet. Aliquam blandit orci sed lectus egestas porttitor et et lectus. Nunc volutpat luctus nisl non efficitur. Fusce purus turpis, ultricies nec lacinia a, hendrerit vitae justo.Phasellus leo odio, dapibus sed est sed, dignissim interdum ante. Donec dignissim luctus nisl, et suscipit arcu consectetur vitae. Fusce vestibulum laoreet consequat. Donec dui orci, scelerisque eu efficitur in, condimentum eu dolor. Cras pulvinar nisi id nibh accumsan, ac tincidunt arcu tincidunt. Fusce et nisl non eros semper lobortis. Aenean vitae fermentum lorem, sed imperdiet nibh. Cras vitae blandit nisi. Cras vitae fermentum ipsum. Etiam porta lorem nec turpis varius pulvinar. Mauris luctus mauris tristique, ultrices dui quis, consectetur nunc.Cras sapien felis, vestibulum non justo ut, varius egestas nisi. Suspendisse maximus augue nunc, nec congue sem sollicitudin ultrices. Etiam ut maximus augue. Aenean gravida hendrerit augue cursus malesuada. Sed vitae dictum quam. In euismod pulvinar tellus eget ullamcorper. Interdum et malesuada fames ac ante ipsum primis in faucibus. Suspendisse at orci ex. Nam egestas lectus vel hendrerit pulvinar. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; In bibendum neque vestibulum nibh tempor tristique. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Pellentesque et risus a tellus cursus pretium. Sed vitae elit interdum tortor varius commodo eu in libero.Integer in elit ut lacus viverra vestibulum. Vivamus aliquam ullamcorper felis. Phasellus porttitor interdum ligula, sed consectetur nisl euismod eu. Duis quis tempus turpis. Mauris pellentesque velit a pellentesque mattis. Nam non ornare urna. Vestibulum lacus lorem, vehicula et est in, posuere vehicula est. Morbi felis nibh, interdum ut massa vitae, feugiat aliquet turpis. Maecenas commodo nibh urna, eget placerat lacus luctus eu. Morbi varius magna vel dignissim varius. Aliquam nunc erat, finibus vitae posuere sit amet, maximus a lectus. Fusce quam orci, aliquam eu efficitur sed, auctor in lorem. In et dolor id neque placerat luctus rhoncus in nunc. Nunc scelerisque risus non tincidunt vulputate. Aliquam arcu nulla, pharetra tempus sapien a, pharetra posuere augue.Interdum et malesuada fames ac ante ipsum primis in faucibus. Ut rutrum dictum orci, eu molestie risus tristique vitae. Nullam erat mauris, molestie et nulla eu, rutrum finibus dui. Nulla tortor tortor, rhoncus quis posuere sed, mollis at lacus. Sed varius commodo justo, vel aliquam nisl accumsan ut. In id lorem est. Quisque a nisl a nulla venenatis rhoncus. Integer nec tincidunt metus. Etiam sit amet neque dui. In aliquam, risus vel pharetra varius, odio velit laoreet purus, eget molestie eros lectus a ex.Maecenas auctor pretium ipsum, a mollis nulla laoreet nec. Sed ut turpis eleifend, dapibus felis nec, sagittis risus. Quisque in mauris dui. Nam in pharetra quam. Maecenas luctus hendrerit urna sit amet consectetur. Suspendisse in mattis lectus, vitae imperdiet nunc. Cras consequat mi ligula, non congue sem lacinia et. Vestibulum tristique fringilla aliquam. Etiam hendrerit, mi ut rhoncus laoreet, libero mauris auctor magna, sit amet varius diam ligula eget odio. In lacinia porttitor magna ac auctor.Vestibulum eleifend, odio quis venenatis cursus, mauris erat dignissim lorem, nec convallis lectus justo non lorem. Sed in dolor eget enim placerat dapibus et ac lorem. Interdum et malesuada fames ac ante ipsum primis in faucibus. Duis bibendum orci nec molestie posuere. In sit amet tortor blandit, viverra est quis, tempus ante. Morbi quis malesuada massa. Quisque sit amet nisi congue, auctor massa quis, hendrerit est. Praesent tincidunt ante in condimentum egestas. Vivamus accumsan molestie tempus. Mauris placerat consequat congue. Duis purus odio, fermentum ut sem ac, sollicitudin imperdiet libero. Suspendisse fringilla suscipit ante. Donec sit amet sagittis massa, non rutrum sapien. Fusce accumsan posuere nisl non placerat. Quisque a lectus ipsum.Vestibulum tincidunt urna neque, ac posuere risus dapibus ultrices. In aliquet dolor sed tempor accumsan. Cras eu dolor placerat felis iaculis ultricies sit amet vel sem. Morbi nec nibh a est tincidunt ultricies. Praesent dignissim tristique sapien. Sed porta, lorem in posuere finibus, ipsum ligula fringilla orci, in rutrum tellus ante id orci. Etiam in suscipit dui. Nullam ultrices neque sit amet elit lacinia consectetur et eu nulla. Nulla lacinia mi purus, at facilisis libero egestas at. Donec porta finibus magna, eget efficitur magna ultrices a. Donec gravida nibh vel sollicitudin interdum.Nulla id ex convallis, luctus purus quis, aliquet ligula. Quisque nec finibus lorem. Phasellus nec faucibus metus, in laoreet magna. Cras tortor elit, interdum vel facilisis nec, blandit a ex. Fusce eu massa convallis mi iaculis blandit. Quisque viverra mi non ante commodo iaculis. Cras tincidunt ornare velit, eu lobortis erat euismod eget. Vivamus dignissim mollis odio at accumsan. Nulla ut neque vehicula, tempor dui quis, aliquam ipsum. Curabitur convallis mi non auctor dictum. Mauris viverra commodo rutrum. Mauris scelerisque nibh ullamcorper ipsum congue viverra. Morbi sagittis scelerisque turpis, quis efficitur nibh. Maecenas in erat justo. Sed cursus tellus id tortor ullamcorper, et congue purus finibus.Vivamus nec leo commodo, sodales ligula vulputate, tempus ex. Fusce lacinia diam elit, ac lobortis enim pretium rutrum. Pellentesque condimentum justo vitae ipsum convallis finibus. Donec quis sollicitudin elit, sit amet hendrerit ipsum. Mauris consectetur lorem velit, non finibus lorem dapibus ac. Cras quis est ut risus tempus ultricies ut ut purus. Suspendisse cursus velit at porttitor sagittis. Mauris non magna vel dolor vehicula vestibulum. Morbi gravida mattis lectus, vitae imperdiet neque porta eget. Nullam euismod cursus leo accumsan tempor. Integer vulputate ante et ante pellentesque viverra. Donec at massa eu massa sodales tempus.Cras nec pharetra quam, non dapibus enim. Aenean vel nunc venenatis, condimentum orci quis, tincidunt odio. Cras euismod justo ut dui lobortis ornare. Vivamus fringilla, enim eu congue porta, orci quam dictum sem, ac efficitur velit est eu nulla. Nulla mi nibh, varius malesuada ornare nec, venenatis porttitor felis. Aenean id maximus quam, non pharetra mi. Quisque ullamcorper ex risus, et cursus nisi volutpat eget. Maecenas sapien lorem, varius vitae lorem sit amet, dapibus ullamcorper elit. Pellentesque convallis velit quis dui ullamcorper, non cursus lectus accumsan. Mauris faucibus nisi sit amet tristique placerat. Cras luctus diam ut magna ultricies, et feugiat mauris pulvinar. Quisque cursus imperdiet ultricies.Praesent sodales metus et augue pharetra, at vehicula nulla tristique. Integer et dictum dui. Aenean eget libero aliquam, fermentum justo id, mattis mauris. Mauris lobortis diam metus, ut iaculis orci convallis vel. Proin ante mauris, sagittis ac orci ac, tincidunt sagittis ligula. Vestibulum lobortis nisi a ligula laoreet iaculis. Sed dignissim dolor diam, eget lacinia est sagittis et. Morbi elementum sodales ex eu facilisis. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nam nec consequat turpis, ultrices ornare quam. Integer nunc tellus, porttitor nec dolor vel, elementum tincidunt mauris. Pellentesque varius feugiat ex mollis fermentum.Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Proin quis hendrerit neque. Etiam tempor dolor in tellus tincidunt, a gravida sem ultrices. Nunc placerat turpis felis, nec dictum est pretium eu. Maecenas ullamcorper at nulla vel elementum. Maecenas sed tincidunt dolor. Interdum et malesuada fames ac ante ipsum primis in faucibus. Aenean cursus ligula arcu, ut molestie dui eleifend nec. Praesent faucibus sem leo, sit amet placerat sapien porttitor in. Mauris viverra dignissim tempor. Ut at mi ligula. Mauris faucibus ullamcorper dui in luctus. Pellentesque a porttitor ligula. Vestibulum interdum, tellus eget consectetur consectetur, nulla urna consectetur sem, non efficitur eros nisi in quam.Sed finibus dapibus massa, quis commodo elit lobortis et. Nulla mollis finibus luctus. Aliquam erat volutpat. Donec lacus lacus, consectetur sit amet pulvinar nec, porttitor sed mauris. Aliquam elementum in sapien nec aliquam. Fusce non ex ut ex iaculis rutrum. Aenean viverra arcu non elit posuere aliquam. Aenean magna massa, euismod in diam sit amet, interdum consectetur massa. Vivamus in imperdiet elit, eget vestibulum sem. Aliquam interdum diam a nulla lacinia, quis pellentesque lorem ullamcorper. Nullam rutrum, est sagittis sodales sollicitudin, tellus neque vestibulum mi, eu molestie leo elit eget justo. Integer justo tellus, efficitur at commodo sed, tempus sed mauris. Quisque iaculis ut eros in pretium. Donec in bibendum mauris, quis eleifend erat. Pellentesque aliquam dignissim varius. Etiam gravida finibus mi a aliquam.Morbi malesuada elit vel maximus hendrerit. Interdum et malesuada fames ac ante ipsum primis in faucibus. Vivamus condimentum porta elit. Aenean mollis tellus ut urna venenatis ornare. Maecenas at scelerisque ipsum. Morbi pharetra placerat dolor, sit amet fringilla nisl posuere scelerisque. Curabitur pharetra mi at sollicitudin interdum. Nunc egestas sodales fringilla. Morbi condimentum ornare lobortis. Donec sed sodales purus, id cursus mauris. Suspendisse sit amet molestie turpis.Duis ullamcorper sem at nisl vehicula ornare. Phasellus fermentum felis et eros hendrerit cursus. Vivamus mattis lectus nec erat cursus facilisis. Suspendisse sit amet ullamcorper lorem. Aliquam sodales neque at turpis convallis volutpat. Integer consequat pellentesque tellus, et vestibulum justo lacinia a. Sed pulvinar imperdiet cursus. Nunc odio tellus, gravida ut ex sit amet, eleifend tristique massa. Pellentesque pharetra vel est sed efficitur. Curabitur viverra mauris eget metus hendrerit, ut hendrerit est vestibulum.Integer vel aliquet enim. Praesent eu congue quam. Suspendisse hendrerit eros ut ante fringilla varius. Nunc tristique facilisis urna. Nullam eget urna eget orci iaculis scelerisque egestas id ipsum. Proin non turpis ipsum. Curabitur congue lorem rutrum ligula mollis, malesuada hendrerit enim rutrum. Sed tortor justo, aliquam ac odio eget, condimentum condimentum ligula. Mauris condimentum elit magna, at vehicula arcu vehicula pellentesque.Integer placerat velit faucibus diam consectetur laoreet. Vivamus pretium lectus ac consectetur iaculis. Nulla facilisi. Sed varius nibh eget lorem commodo, a porttitor purus ornare. Mauris dapibus nulla eu dolor tristique, quis malesuada sem porta. Sed id venenatis quam, non cursus eros. Vivamus placerat pretium ligula, vel fermentum ligula suscipit at. Morbi viverra vel nulla et porttitor. Quisque mollis ultrices lorem et venenatis. Sed consectetur est nisl, non egestas orci porttitor eu. Donec id erat nec ligula rutrum dapibus eget at enim.Vestibulum leo mi, tincidunt vel ante id, posuere cursus ligula. Aenean laoreet interdum lorem, eget condimentum purus feugiat a. Praesent sed tristique purus. Maecenas rutrum posuere diam, sed posuere est auctor vel. Sed lobortis odio quis dolor ultricies, vitae tincidunt ipsum molestie. Pellentesque molestie arcu quis hendrerit condimentum. Etiam iaculis eleifend vulputate. Donec lobortis eu quam ac dapibus. Morbi vel molestie nisi. Donec quis sagittis urna, congue aliquam velit.Duis interdum mollis vehicula. Aenean iaculis eros sit amet odio cursus placerat. Proin porttitor risus orci, nec porttitor diam facilisis a. Fusce cursus, nisi vitae imperdiet sodales, justo leo luctus velit, eu luctus orci velit aliquet eros. Nulla facilisi. Integer egestas maximus ex, quis malesuada tellus laoreet sit amet. Suspendisse lacinia neque ac imperdiet aliquet.Vivamus vitae enim ex. Donec vehicula luctus sapien, id ultricies libero sodales quis. Aenean at diam nisi. Cras suscipit at lacus sed volutpat. Vivamus eu gravida justo. Praesent id rutrum eros. Aenean vel orci elit. Nam venenatis metus luctus, dictum magna sed, volutpat neque. Praesent id arcu iaculis, pretium orci a, tincidunt urna. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Curabitur posuere ut nisl vel rhoncus. Nulla facilisi. Quisque sed efficitur lacus.Quisque a diam sagittis velit sagittis aliquam. Donec cursus, ligula non mollis efficitur, libero lorem luctus eros, et tempor dolor metus eget magna. Cras facilisis mollis mi a rhoncus. Mauris ligula arcu, posuere viverra mi a, lobortis semper augue. Aliquam vitae turpis et tellus vehicula pharetra quis quis ex. Quisque diam odio, condimentum sed sem sit amet, condimentum rutrum turpis. Phasellus ligula orci, volutpat a eros in, pharetra pharetra lectus.Aliquam condimentum vestibulum risus, eget commodo est cursus nec. Cras finibus, eros vitae molestie scelerisque, justo tellus efficitur erat, et faucibus ipsum felis eu justo. Curabitur tincidunt et diam sed molestie. Nulla ornare dui varius eros aliquam, blandit porttitor lectus convallis. Ut id lectus nisi. Phasellus tellus enim, hendrerit quis purus ut, tempor interdum tortor. Quisque pretium nunc neque, ut vehicula ante consectetur eu. Cras eget velit sit amet ipsum pharetra pretium id quis quam.Curabitur eget finibus neque. Praesent suscipit eros ex, eu tempor est facilisis nec. Donec congue risus mi, efficitur semper nunc convallis sit amet. Vestibulum at accumsan magna, eu maximus massa. Quisque aliquet ipsum est, vitae porttitor eros semper ac. Nunc at dui urna. Nunc tempor pulvinar felis ac dignissim. Nulla leo felis, blandit non lectus sed, ultrices lacinia mauris. Cras ultrices commodo commodo.Morbi molestie ante a sem efficitur hendrerit. Sed pretium dignissim erat in imperdiet. Proin non arcu non ante ullamcorper accumsan. Nulla eleifend tempus nunc, in venenatis ex finibus nec. Phasellus eget elit lorem. Etiam maximus erat et velit efficitur laoreet. Donec porttitor, turpis nec consectetur malesuada, massa velit lacinia ipsum, efficitur laoreet turpis tortor eu tellus. Fusce dignissim dolor nec aliquet lobortis. Vivamus sollicitudin justo ut finibus aliquam. Duis massa nunc.";

    if(!removeArquivo(clusterArquivo, metaDados)){        //Remove o arquivo anterior para substituir pelo arquivo editado
        printf("Erro na remocao do arquivo anterior\n");
        return 0;
    }

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    pegaCluster(clusterArquivo, &dir, metaDados); //pega os metadados do cluster a ser editado
    atualCluster = clusterArquivo;
    //Loop para iterar sobre cada cluster necessario para armazenar o arquivo
    do{
        //Posiciona o ponteiro do arquivo no inicio do cluster principal do arquivo
        fseek(arq, metaDados.initCluster + ((metaDados.tamCluster * 1000) * atualCluster + indexCluster), SEEK_SET);
        //Escreve o texto no cluster ate chegar no final do texto ou ocupar o cluster todo
        while((texto[indexTexto] != '\0') && (indexCluster < (metaDados.tamCluster * 1000))){
            fwrite(&texto[indexTexto], sizeof(char), 1, arq);
            indexTexto++;
            indexCluster++;
        }

        alteraTabelaFat(255, atualCluster, metaDados);          //Marca por enquanto o cluster atual como final na tabela FAT

        if(texto[indexTexto] != '\0'){                                      //Se nao chegou no fim do texto
            if(primeiraPosicaoDisponivel(&proximoCluster, metaDados)){      //Se conseguiu encontrar um proximo cluster diponivel
                indexCluster = 0;                                           //Reinicia o iterador
                alteraTabelaFat(proximoCluster, atualCluster, metaDados);   //Atualiza a tabela para apontar para o proximo cluster
                atualCluster = proximoCluster;                              //Muda o cluster atual para o proximo a ser preenchido
            }else{                                                          //Se nao ha cluster disponivel
                printf("Nao foi possivel encontrar cluster disponivel\n");
                return 0;                                                   //Avisa ao ususario retornando erro
            }
        }else{                                                              //Se chegou ao final do texto
            proximoCluster = 0;                                             //Sinaliza para sair do loop
        }

    }while(proximoCluster != 0);

    while(indexCluster < (metaDados.tamCluster * 1000)){    //Preenche o restante do cluster com zeros
        fwrite(&zero, sizeof(char), 1, arq);
        indexCluster++;
    }

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na atualizacao dos clusters\n");
        return 0;
    }
    return 1;

}

void detectaComando(char comando[], char** caminho, char *dirAtual, short int* sair, MetaDados metaDados){
/* Detecta os possíveis comandos exigidas pelo usuário,
 * separando a operação do possível nome de diretórios e arquivos
 * Entrada:
 *      Input de comando dado pelo ususario
 *      Ponteiro (linha) do diretorio atual
 *      Tabela FAT que armazena os ponteiros dos discos
 *      Flag que controla o comando de saida do usuario
 *      Metadados para auxiliar na busca no arquivo
 */
    char clusterDisponivel, cluster = 0;
    char *operacao = NULL, *nome = NULL, *extensao = NULL, *resto = NULL;
    ListaStrings *listaComandos, *listaComandosAux;
    NodoCluster auxCluster;


    if(strcmp(comando, "")){   //se foi dado algum input
        pegaOperacaoNome(comando, &operacao, &nome, &resto);

        //MKDIR
        if((strcmp(operacao, "MKDIR") == 0)||(strcmp(operacao, "mkdir") == 0)){
            nome = strtok(nome, ".");
            extensao = strtok(NULL, ".");

            if(primeiraPosicaoDisponivel(&clusterDisponivel, metaDados) && extensao == NULL){   //se encontrou cluster disponivel
                if((nome != NULL) && (mkDir(nome, *dirAtual, clusterDisponivel, metaDados))){
                    printf("Diretorio Criado!\n");
                }else{
                    printf("Erro ao criar o diretorio\n");
                }
            }else{
                printf("Erro ao criar o diretorio\n");
            }

        //DIR
        }else if((strcmp(operacao, "DIR")== 0)||(strcmp(operacao, "dir")== 0)){
            if(!dir(*dirAtual, metaDados)){
                printf("Nao foi possivel listar todos os diretorios e arquvios\n");
            }

        //CD
        }else if((strcmp(operacao, "CD") == 0)||(strcmp(operacao, "cd") == 0)){
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);

            if(!cd(listaComandos, dirAtual, metaDados)){
                printf("Caminho nao encontrado\n");
            }else if(!strcmp(nome, "..")){    //se foi feito CD com sucesso e o comando foi ".."
                voltaCaminho(caminho);
            }else if(!strcmp(listaComandos->comando, "root")){ //se o comando comecou com '/root'
                reconstroiCaminho(caminho, listaComandos);
            }else{                            //senão, concatena o nome do novo caminho
                avancaCaminho(caminho, listaComandos);
            }
            apagaLSE(listaComandos);

        //MKFILE
        }else if((strcmp(operacao, "MKFILE") == 0)||(strcmp(operacao, "mkfile") == 0)){
            if(primeiraPosicaoDisponivel(&clusterDisponivel, metaDados)){
                nome = strtok(nome, ".");
                extensao = strtok(NULL, ".");
                if((nome != NULL) && (extensao != NULL)){
                    if(strcmp(extensao, "TXT") == 0 || strcmp(extensao, "txt") == 0){
                        if(mkFile(nome, extensao, *dirAtual, clusterDisponivel, metaDados)){
                            printf("Arquivo Criado!\n");
                        }else{
                            printf("Erro ao criar o arquivo\n");
                        }
                    }else{
                        printf("Extensao nao reconhecida pelo sistema\n");
                    }
                }else{
                    printf("Erro ao criar o arquivo\n");
                }
            }

        //RM
        }else if((strcmp(operacao, "RM") == 0)||(strcmp(operacao, "rm") == 0)){
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            if(!rm(listaComandos, dirAtual, &cluster, metaDados)){
                printf("Caminho nao encontrado\n");
            }else{
                if(removeCluster(cluster, metaDados)){ //se removeu os clusters com sucesso
                    pegaCluster(cluster, &auxCluster, metaDados);     //apenas retira o cluster removido da lista de filho do pai
                    removeFilho(auxCluster.pai, cluster, metaDados);
                    printf("Cluster removido\n");
                }
            }
            if(*dirAtual == 0){                          //se o usuario voltou pro root agora
                realloc(*caminho, sizeof(char)*5);       //realoca espaco correto pro caminho
                strcpy(*caminho, "root");                //coloca "root" na string caminho
            }
            apagaLSE(listaComandos);

        //EDIT
        }else if((strcmp(operacao, "EDIT") == 0 )||(strcmp(operacao, "edit") == 0)){
            int clusterIncialArquivo;
            listaComandos = NULL;
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            if(listaComandos != NULL && resto != NULL){
                clusterIncialArquivo = encontraCaminho(listaComandos, *dirAtual, *dirAtual, metaDados);
                if(clusterIncialArquivo != -1){
                    if(edit(resto, clusterIncialArquivo, metaDados)){
                        printf("Arquivo editado com sucesso!\n");
                    }else{
                        printf("Ocorreu um erro ao editar o arquivo\n");
                    }
                }else{
                    printf("O arquivo nao foi encontrado. Veja se o caminho esta correto.\n");
                }
                apagaLSE(listaComandos);
            }else{
                printf("A chamada da funcao EDIT deve ser do tipo: EDIT NOMEARQUIVO.txt TEXTO\n");
            }

        //MOVE
        }else if((strcmp(operacao, "MOVE") == 0)||(strcmp(operacao, "move") == 0)){
            listaComandos = NULL;
            listaComandosAux = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            listaComandosAux = pegaSequenciaComandos(resto, listaComandosAux);
            if(listaComandos != NULL && listaComandosAux != NULL && move(listaComandos, listaComandosAux, dirAtual, metaDados)){
                printf("Cluster movido com sucesso!\n");
            }else{
                printf("Nao foi possivel mover o cluster\n");
            }
            if(*dirAtual == 0){                          //se o usuario voltou para o root
                realloc(*caminho, sizeof(char)*5);       //realoca espaco correto pro caminho
                strcpy(*caminho, "root");                //coloca "root" na string caminho
            }
            apagaLSE(listaComandos);
            apagaLSE(listaComandosAux);

        //RENAME
        }else if((strcmp(operacao, "RENAME") == 0)||(strcmp(operacao, "rename") == 0)){
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            if((listaComandos != NULL) && (resto != NULL) && reName(listaComandos, resto, dirAtual, metaDados)){
                printf("Renomeado com sucesso\n");
            }else{
                printf("Nao foi possivel renomear\n");
            }
            apagaLSE(listaComandos);

        //EXIT
        }else if((strcmp(operacao, "EXIT") == 0)||(strcmp(operacao, "exit") == 0)){
            *sair = 1;

        //CLS
        }else if((strcmp(operacao, "cls") == 0)||(strcmp(operacao, "CLS") == 0)){
            system("cls");

        //UNDEFINED
        }else{
            printf("Comando nao reconhecido.\n");
        }
    }
}

