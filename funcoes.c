#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcoes.h"

int inicializaArquivo(){
/* Inicializa o arquivo que simula o disco.
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
    NodoCluster root = {"root", "", 'a', NULL};

    if((arq = fopen("ArqDisco.bin", "a+b")) == NULL){  //se houve problema na abertura do arquivo
        printf("Erro na criacao do arquivo!\n");
        return 0;
    }

    //criacao dos metadados
    fwrite(&metaDados, sizeof(MetaDados), 1, arq);      //escreve os metadados
    fwrite("\n", sizeof(char), 1, arq);

    //criacao da tabela FAT
    fwrite(&FF, sizeof(char), 1, arq);                  //preenche a primeira posição da tabela FAT para o diretório root
    for(i = 0; i < TAMTABELA - 1; i++)
        fwrite(&zero, sizeof(char), 1, arq);            //preenche o restante da tabela
    fwrite("\n", sizeof(char), 1, arq);

    //Laco que faz a criacao dos 256 clusters
    fwrite(&root, sizeof(NodoCluster), 1, arq);         //armazena o root no primeiro cluster
    for(i = 0; i < TAMTABELA - 1; i++){
        fwrite("\n", sizeof(char), 1, arq);
        for(j = 0; j < TAMCLUSTER; j++)
            fwrite(&zero, sizeof(char), 1, arq);   //armazena os outros 255 clusters
    }

    fclose(arq);
    if(ferror(arq)){  //se houver erro em qualquer parte da escrita
        printf("Erro no preenchimento do arquivo!");
        return 0;
    }
    return 1;
}

int pegaMetadados(MetaDados* metaDados){
/* Carrega os metadados do arquivo de disco se possível
 * Entrada:
 *      Ponteiro para estrutura de metadados para preencher
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

void pegaOperacaoNome(char comando[], char** operacao, char** nome){
/* Dado um comando do usuario, pega a operacao e o possível nome do diretorio ou arquivo fornecido
 * Entrada:
 *      String do comando todo fornecido pelo usuário
 *      Ponteiro para a string que armazena a operação
 *      Ponteiro para a string que armazena o nome do diretorio ou arquivo
 */
    *operacao = strtok(comando, " ");
    *nome = strtok(NULL, " ");
}

int pegaTabela(char tabela[]){
/* Carrega para a memória a tabela FAT do arquivo.
 * Entrada:
 *      Tabela FAT que armazena os ponteiros dos discos
 * Retorno:
 *      1 caso leia a tabela no arquivo com sucesso
 *      0 caso falhe na leitura
 */
    int i = 0;
    FILE *arq;
    arq = fopen("ArqDisco.bin", "r");

    if (arq == NULL){ // Se houve erro na abertura{
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    fseek(arq, sizeof(MetaDados) + 1, SEEK_SET);
    while(i < TAMTABELA && fread(&tabela[i], sizeof(char), 1, arq) == 1){ //enquanto houver sucesso na leitura da tabela
        i++;
    }
    fclose(arq);

    if(i != TAMTABELA){  //se nao chegou no final da tabela
        printf("Erro na leitura do arquivo\n");
        return 0;
    }
    return 1;
}

int primeiraPosicaoDisponivel(char tabela[]){
/* Retorna o ponteiro(linha) do pirmeiro cluster disponível
 * Entrada:
 *      Tabela FAT que armazena os ponteiros dos discos
 * Retorno:
 *      Inteiro positivo representando a linha disponível
 *      -1 caso não haja cluster disponível
 */
    int i = 0;

    while(i < 256 && tabela[i] != 0)
        i++;

    if(i == 256)
        i = -1;

    return i;
}

int alteraTabelaFat(char tabela[], int ponteiroCluster){
/* Atualiza a tabela e escreve a tabela modificada no arquivo
 * Entrada:
 *      Tabela FAT que armazena os ponteiros dos discos
 *      Inteiro que presenta o cluster a ser atualizado na tabela
 * Retorno:
 *      1 caso escreva com sucesso.
 *      0 caso haja falha na escrita.
 */

    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo!\n");
        return 0;
    }

    tabela[ponteiroCluster] = 255;
    fseek(arq, sizeof(MetaDados) + 1, SEEK_SET);
    fwrite(tabela, sizeof(char) * TAMTABELA, 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita do arquivo!\n");
        return 0;
    }
    return 1;
}

int adicionaFilho(char pai, char filho){
/* Insere um ponteiro, da tabela fat, na LSE de filhos do cluster "pai"
 * Se a LSE de filhos do cluster pai está vazia, insere o filho na primeira posição
 * Caso contrário, insere o filho na ultima posição da LSE.
 * Recebe:
 *   char, que representa o ponteiro(linha) do cluster pai
 *   char, que representa o ponteiro(linha) do cluster filho.
 * Retorno:
 *   1, caso haja sucesso na escrita da nova lista de filhos
 *   0, caso falhe na escrita
 */
    ListaFilhos *lf, *aux;
    NodoCluster dir;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

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
    if(ferror(arq)){
        printf("Erro na escrita do arquivo\n");
        return 0;
    }
    return 1;
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
    NodoCluster novo = {"", "", 'a', NULL};
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
    fclose(arq);
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


