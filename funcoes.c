#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"

void inicializaArquivo(){
/* Inicializa o arquivo que simula o disco.
 * Cria uma área de metadados de 8 bytes
 * Cria uma áre de 256 bytes, que armazena a tabela fat
 * Inicializa o cluster "root", tornando-o o principal.
 * Inicializa os 255 clusters restantes de 32KB cada
 */
    int i;
    MetaDados metaDados = {TAMTABELA, TAMCLUSTER, 0, 1}; //Estrutura do tipo MetaDados, que inicia os meta dados referente ao disco.
    FILE *arq;                                           //ponteiro para o arquivo
    //int bytesCluster = 0;
    char zero = 0;
    char valor255 = 255;
    NodoCluster root = {"root", "", 'a', 'a', NULL};

    arq = fopen("ArqDisco.bin", "r+b");

    if (arq == NULL){
        printf("Problemas na criacao do arquivo\n");
        return;
    }else{
        fwrite(&metaDados, sizeof(MetaDados), 1, arq);
        fwrite("\n", sizeof(char), 1, arq);

        for(i = 0; i < TAMTABELA; i++){
            fwrite(&zero, sizeof(char), 1, arq);
        }
        fwrite("\n", sizeof(char), 1, arq);

        //Laco que a criacao dos 256 clusters
        for(i = 0; i < TAMTABELA; i++){
            //Laco que controla a criacao de um cluster com 32KB, todos com 0
            for(int j = 0; j < TAMCLUSTER; j++){
            fwrite(&zero, sizeof(char), 1, arq);
            }
            fwrite("\n", sizeof(char), 1, arq);
        }

        fseek(arq, sizeof(MetaDados), SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&valor255, sizeof(char), 1, arq);

        //Criacao do cluster root
        fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 , SEEK_SET);
        fwrite("\n", sizeof(char), 1, arq);
        fwrite(&root, sizeof(NodoCluster), 1, arq);
    }
    fclose(arq);
}

int pegaMetadados(MetaDados* metaDados){
/* Carrega os metadados do arquivo de disco se possível
 * Devolve 1 se a operação for feita com sucesso
 * Devolve 0 se a operação falhar */
    FILE *arq;
    arq = fopen("ArqDisco.bin", "rb+");

    //Se houve erro na abertura
    if (arq == NULL){
        printf("Problemas na abertura do arquivo\n");
        return 0;
    }

    //se houve erro na leitura
    if(fread(metaDados, sizeof(MetaDados), 1, arq) != 1){
        printf("Problemas na leitura do arquivo\n");
        fclose(arq);
        return 0;
    }
    fclose(arq);
    return 1;
}

void pegaOperacaoNome(char comando[], char** operacao, char** nome){
/* Dado um comando do usuário, pega a operacao e o possível nome do diretório ou arquivo fornecido */
    *operacao = strtok(comando, " ");
    *nome = strtok(NULL, " ");
}

int pegaTabela(char tabela[]){
/* Recebe um ponteiro para uma tabela de 256 bytes.
 * Retorna por parâmetro a tabela fat que armazena os ponteiros dos discos.
 * Caso a leia a tabela no arquivo com sucesso, devolve 1.
 * Caso falhe na leitura, devolve 0.
 */
    int i;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r");

    if (arq == NULL){ // Se houve erro na abertura{
        printf("Problemas na abertura do arquivo\n");
        return 0;
    }

    fseek(arq, sizeof(MetaDados) + 1, SEEK_SET);
    while(i < TAMTABELA && fread(&tabela[i], sizeof(char), 1, arq) == 1){ //enquanto houver sucesso na leitura da tabela
        i++;
    }
    fclose(arq);

    if(i != TAMTABELA){  //se nao chegou no final da tabela
        printf("Problemas na leitura do arquivo\n");
        return 0;
    }
    return 1;
}

int primeiraPosicaoDisponivel(char tabela[]){
/* Retorna o ponteiro(linha) do pirmeiro cluster disponível */
    int i = 0;

    while(i < 256 && tabela[i] != 0)
        i++;

    if(i == 256)
        i = 0;

    return i;
}

void adicionaFilho(char pai, char filho){
/* Insere um ponteiro, da tabela fat, na LSE de filhos do cluster "pai"
 * Se a LSE de filhos do cluster pai está vazia, insere o filho na primeira posição
 * Caso contrário, insere o filho na ultima posição da LSE.
 * Recebe:
 *   char, que representa o ponteiro(linha) do cluster pai
 *   char, que representa o ponteiro(linha) do cluster filho.
 */
    ListaFilhos *lf, *aux;
    NodoCluster dir;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");

    //Posiciona o ponteiro do arquivo na linha do cluster pai
    fseek(arq, sizeof(MetaDados) + 1 + TAMTABELA + 1  + ((TAMCLUSTER + 1) * pai), SEEK_SET);
    fread(&dir, sizeof(NodoCluster), 1, arq);

    //Aloca um espaço em memória para inserir um novo filho na LSE
    lf = malloc(sizeof(ListaFilhos));
    lf->filho = filho;

    //Se a LSE dos filhos é NULL, então insere o filho na primeira posição da LSE
    if(dir.filhos == NULL){
        lf->prox = NULL;
        dir.filhos = lf;
    }else{//Caso contrário, percorre a LSE até o final e insere o filho no fim.
        aux = dir.filhos;
        while(aux->prox != NULL){
           aux = aux->prox;
        }
        aux->prox = lf;
        lf->prox = NULL;
    }
    //Grava a atualização feita na LSE dos filhos.
    fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 + ((TAMCLUSTER + 1) * pai), SEEK_SET);
    fwrite("\n", sizeof(char), 1, arq);
    fwrite(&dir, sizeof(NodoCluster), 1, arq);
    fclose(arq);
}

int mkDir(char* nome, int clusterPai, int cluster, char tabela[]){
/* Cria um diretório no primeiro cluster disponível dado o diretório atual
 * Retorna 1 caso seja realizado com sucesso
 * Retorna 0 caso a criação falhe
 */
    //Ponteiro para o arquivo
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");
    int i = 0;

    //Cria o novo Cluster
    NodoCluster novo = {"", "", 'a','a', NULL};
    strcpy(novo.nome, nome);
    novo.pai = clusterPai;


    //Marca o primeiro cluster dispnível como ocupado e escreve no arquivo(talvez vire uma função)
    alteraTabelaFat(tabela, cluster);

    //Posiciona o cursor na linha do primeiro cluster disponível que será preenchido pelo novo diretório
    fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 + ((TAMCLUSTER + 1) * cluster), SEEK_SET);
    fwrite("\n", sizeof(char), 1, arq);

    //Escreve o cluster no arquivo
    i = fwrite(&novo, sizeof(NodoCluster), 1, arq);
    //Se deu erro durante a gravação, sai da função, retornando 0.
    if(i == 0){
        fclose(arq);
        return 0;
    }

    fclose(arq);
    adicionaFilho(clusterPai, cluster);
    return 1;
}

void dir(char pai){
/* Lista todos os subdiretorios e arquivos de um diretorio principal
 * Recebe:
 *   char, que representa o diretorio principal
 */
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");
    ListaFilhos *aux;
    NodoCluster dir, subdir;
    //Posiciona o cursor do arquivo no ponteiro(linha) que representa o diretório principal.
    fseek(arq, sizeof(MetaDados)+1 + TAMTABELA+1  + ((TAMCLUSTER + 1) * pai), SEEK_SET);
    fread(&dir, sizeof(NodoCluster), 1, arq);

    aux = dir.filhos;
    if(aux == NULL){
        printf("<vazio>");
    }
    //Laço que percorre a LSE de filhos, printando-os, até chegar ao final da mesma.
     while(aux != NULL){
        fseek(arq, sizeof(MetaDados)+1 + TAMTABELA+1  + ((TAMCLUSTER + 1) * aux->filho), SEEK_SET);
        fread(&subdir, sizeof(NodoCluster), 1, arq);
        printf("%s  ", subdir.nome);
        aux = aux->prox;
    }
    printf("\n");


}

char* stringEntrada(FILE* fp, size_t tamanho){
/* Função que pega o input do usuário e estende o tamanho
 * da variável que guarda a string caso o input seja muito grande
 */
    char *str;
    int ch;
    size_t index = 0;
    str = realloc(NULL, sizeof(*str)*tamanho); //tamanho é o tamanho inicial

    if(!str)
        return str;

    while(EOF != (ch = fgetc(fp)) && ch != '\n'){
        str[index++] = ch;
        if(index == tamanho){
            str = realloc(str, sizeof(*str)*(tamanho += REALLOCSIZE));
            if(!str)
                return str;
        }
    }
    str[index++] = '\0';

    return realloc(str, sizeof(*str)*index);
}

ListaStrings* inserirLSEStrings(ListaStrings* lseString, char * string){
/* ListaStrings*, char* -> ListaStrings*
 * Dado um ponteiro para uma Lista de Strings e uma String, insere a String na lista.
 * Se a lista está vazia, insere na primeira posição
 * Caso contrário, percorre toda a lista e insere no final.
*/
    ListaStrings *novo, *aux;
    novo = (ListaStrings*) malloc(sizeof(ListaStrings));
    novo->comando = string;
    if(lseString == NULL){
        novo->prox = NULL;;
        return novo;
    }else{
        aux = lseString;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = novo;
        novo->prox = NULL;
    }
    return lseString;
}

ListaStrings* pegaSequenciaComandos(char *comando, ListaStrings *lc){
/* char*, ListaStrings* -> ListaStrings
 * Dado uma String, contendo um caminho separado por '/', retorna uma
 * Lista de Strings contendo somente as strings que estão entre as barras.
 * Ex pegaSequenciaComandos("TESTE/NOVO/FOTOS/FESTA", ListaStrings): TESTE->NOVO->FOTOS->FESTA->NULL
 */
    char* pt;
    pt = strtok(comando, "/");
    while(pt){
        lc = inserirLSEStrings(lc,pt);
        pt = strtok(NULL, "/");
    }
    return lc;
}

int percorreFilhos(ListaFilhos *lf, ListaStrings *listaComandos, int *diretorioAtual){
/* ListaFilhos*, ListaStrings*, int* -> int.
 * Dado uma lista de filhos de um diretório, uma lista de strings, que representa o caminho
 * de um diretório/arquivo e o um int, que representa o diretório em que foi realizado uma operação.
 * Retorna 1 caso o caminho não tenha sido encontrado
 * Retorna 0 caso o caminho tenha sido encontrado
 */
    NodoCluster dir;
    ListaFilhos *aux;
    aux = lf;

    //Pega o diretório atual do disco
    dir = pegaCluster(*diretorioAtual);
    if(listaComandos == NULL){
        return 1;
    }else if(strcmp(listaComandos->comando, "..") == 0 && *diretorioAtual != 0){
        *diretorioAtual = dir.pai;
        return 0;
    }
    while(aux != NULL){
    //Pega o diretório do respectivo filho
    dir = pegaCluster(aux->filho);
        //Verifica se o nome do diretório do respectivo filho é igual ao que está na primeira posição da lista de comandos
        if(strcmp(listaComandos->comando, dir.nome) == 0){
            //Se sim, verifica se o próximo elemento da lista de comandos é NULL
            if(listaComandos->prox == NULL){
                //Caso seja, seta o diretório buscado como atual e retorna 0
                *diretorioAtual = aux->filho;
                return 0;
            }else{//Se o próximo elemento da lista de comandos não é NULL, chama a função recursivamente.
            return percorreFilhos(dir.filhos, listaComandos->prox, diretorioAtual);
            }
        }
        aux = aux->prox;
    }
    return 1;
}

ListaStrings* apagaLSE(ListaStrings* ptNum){
/*
    ListaStrings -> ListaStrings
    Libera toda a memória alocada e ocupada por uma LSE
*/
    ListaStrings* aux;
    while(ptNum != NULL){
        aux = ptNum;
        ptNum = ptNum->prox;
        free(aux);
    }
    return NULL;
}
void detectaComando(char comando[], int *dirAtual, char tabela[], short int* sair){
/* Detecta os possíveis comandos exigidas pelo usuário,
 * separando a operação do possível nome de diretórios e arquivos */
    char *operacao = NULL, *nome = NULL;

    ListaStrings *listaComandos;

    pegaOperacaoNome(comando, &operacao, &nome);

    if(strcmp(operacao, "MKFILE") == 0){
        printf("Criar Arquivo");
    }else if(strstr(operacao, "MKDIR") != NULL){
        if(nome != NULL && mkDir(nome, *dirAtual, primeiraPosicaoDisponivel(tabela), tabela)){
            printf("Diretorio Criado!\n");
        }else{
            printf("Erro ao criar o diretorio\n");
        }
    }else if(strstr(operacao, "DIR") != NULL){
        dir(*dirAtual);
    }else if(strcmp(operacao, "CD") == 0){
        NodoCluster dir;
        dir = pegaCluster(*dirAtual);
        listaComandos = NULL;
        listaComandos = pegaSequenciaComandos(nome, listaComandos);
        if(percorreFilhos(dir.filhos, listaComandos, dirAtual)%2 != 0){
            printf("Caminho nao encontrado\n");
        }
        apagaLSE(listaComandos);
    }



    else if(strcmp(operacao, "RM") == 0){
        printf("Deletar arquivo/direorio\n");
    }else if(strstr(operacao, "EDIT") != NULL){
        printf("Editar arquivo\n");
    }else if(strcmp(operacao, "MOVE") == 0){
        printf("Mover diretorio/arquivo\n");
    }else if(strcmp(operacao, "RENAME") == 0){
        printf("Renomear arquivo/diretorio\n");
    }else if(strcmp(operacao, "SAIR") == 0){
        *sair = 1;
    }else{
        printf("Comando nao reconhecido.\n");
    }
}

NodoCluster pegaCluster(int ponteiroCluster){
/*
    Int -> NodoCluster;
    Dado um inteiro, que representa o ponteiro de um cluster, retorna o cluster para o qual o ponteiro aponta.
*/
    NodoCluster dir;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");
    fseek(arq, sizeof(MetaDados)+1 + TAMTABELA+1  + ((TAMCLUSTER + 1) * ponteiroCluster), SEEK_SET);
    fread(&dir, sizeof(NodoCluster), 1, arq);
    fclose(arq);

    return dir;
}

int insereNodoCluster(NodoCluster nodoCluster, int ponteiroCluster){
    NodoCluster dir;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");

    fseek(arq, sizeof(MetaDados) + TAMTABELA + 1 + ((TAMCLUSTER + 1) * ponteiroCluster), SEEK_SET);
    fwrite("\n", sizeof(char), 1, arq);


    if(fwrite(&dir, sizeof(NodoCluster), 1, arq)){
        fclose(arq);
        return 1;
    }else{
        fclose(arq);
        return 0;
    }
}
int alteraTabelaFat(char tabela[], int ponteiroCluster){
  NodoCluster dir;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r+b");

    tabela[ponteiroCluster] = 255;
    fseek(arq, sizeof(MetaDados)+1, SEEK_SET);
    fwrite(tabela, sizeof(char) * TAMTABELA, 1, arq);


    fclose(arq);
}


