#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listaStrings.h"

/*****************************************************************************************************/
/*                                   FUNCOES AUXILIARES                                              */
/*****************************************************************************************************/

int inicializaArquivo(){
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

    if((arq = fopen("ArqDisco.bin", "r")) == NULL){        //se o arquivo não existe ainda
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
        for(i = 0; i < (TAMCLUSTER - sizeof(NodoCluster)); i++)
            fwrite(&zero, sizeof(char), 1, arq);

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
    }
    return 1;
}

char* stringEntrada(FILE* fp, size_t tamanho){
/* Função que pega o input do usuário e estende o tamanho
 * da variável que guarda a string caso o input seja muito grande
 * Entrada:
 *      Ponteiro para FILE para saber de qual stream pegar o input
 *      Tamanho inicial para armezar o input
 * Retorno:
 *      Ponteiro para a string com o input recebido
 */
    char *str;
    int ch;
    size_t index = 0;
    str = realloc(NULL, sizeof(*str) * tamanho); //tamanho é o tamanho inicial

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

int pegaTabela(char tabela[], MetaDados metaDados){
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

    fseek(arq, metaDados.initIndice + 1, SEEK_SET);
    while(i < metaDados.tamIndice && fread(&tabela[i], sizeof(char), 1, arq) == 1){ //enquanto houver sucesso na leitura da tabela
        i++;
    }
    fclose(arq);

    if(i != metaDados.tamIndice){  //se nao chegou no final da tabela
        printf("Erro na leitura do arquivo\n");
        return 0;
    }
    return 1;
}

int pegaCluster(int ponteiroCluster, NodoCluster* cluster, MetaDados metaDados){
/* Dado um inteiro, que representa o ponteiro de um cluster, retorna o cluster para o qual o ponteiro aponta.
 * Entrada:
 *      Inteiro que representa o ponteiro para a linha em que o cluster está
 *      Ponteiro para NodoCluster para preenche-lo com o cluster apontado
 * Retorno:
 *      1, caso a leitura seja feito com sucesso
 *      0, caso haja falha na leitura
 */
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * ponteiroCluster), SEEK_SET);
    fread(cluster, sizeof(NodoCluster), 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro a leitura do cluster no arquivo\n");
        return 0;
    }

    return 1;
}

int primeiraPosicaoDisponivel(char tabela[], MetaDados metaDados){
/* Retorna o ponteiro(linha) do pirmeiro cluster disponível
 * Entrada:
 *      Tabela FAT que armazena os ponteiros dos discos
 * Retorno:
 *      Inteiro positivo representando a linha disponível
 *      -1 caso não haja cluster disponível
 */
    int i = 0;

    while(i < metaDados.tamIndice && tabela[i] != 0)
        i++;

    if(i == metaDados.tamIndice)
        i = -1;

    return i;
}

int alteraTabelaFat(char tabela[], int ponteiroCluster, MetaDados metaDados){
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
    fseek(arq, metaDados.initIndice + 1, SEEK_SET);
    fwrite(tabela, sizeof(char) * metaDados.tamIndice, 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na escrita do arquivo!\n");
        return 0;
    }
    return 1;
}

int adicionaFilho(char pai, char filho, MetaDados metaDados){
/* Insere um ponteiro, da tabela fat, na LSE de filhos do cluster "pai"
 * Se a LSE de filhos do cluster pai está vazia, insere o filho na primeira posição
 * Caso contrário, insere o filho na ultima posição da LSE.
 * Entrada:
 *   char, que representa o ponteiro(linha) do cluster pai
 *   char, que representa o ponteiro(linha) do cluster filho.
 * Retorno:
 *   1, caso haja sucesso na escrita da nova lista de filhos
 *   0, caso falhe na escrita
 */
    char a = 0;
    FILE *arq;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro do arquivo no cluster onde será inserido um filho
    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * pai), SEEK_SET);

    //Percorre todos os caracteres do cluster até encontrar o marcador '*'
    while(a != '*'){
        a = fgetc(arq);
    }

    //Percorre, a partir do marcador '*', até encontrar o primeira posição livre para inserir o ponteiro do filho
    while(a != 0){
        a = fgetc(arq);
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

int pegaCaminho(int diretorioAtual, char* caminho, MetaDados metaDados){
/* Cria uma string formatada do caminho no disco ate o diretorio atual
 * Entrada:
 *      inteiro (ponteiro), representando o cluster atual
 *      ponteiro para uma string a ser preenchida com o caminho formatado
 * Retorno:
 *      1 caso o caminho seja montado com sucesso
 *      0 caso haja falha na montagem
 */
    NodoCluster cluster;

    if(pegaCluster(diretorioAtual, &cluster, metaDados)){
        if(diretorioAtual != 0){
            if(pegaCaminho(cluster.pai, caminho, metaDados)){
                strcat(caminho, "/");
                strcat(caminho, cluster.nome);
                return 1;
            }else{
                return 0;
            }
        }else{
            strcpy(caminho, "root");
            return 1;
        }
    }else{
        return 0;
    }
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

/*****************************************************************************************************/
/*                                   FUNCOES PRINCIPAIS                                              */
/*****************************************************************************************************/

int mkDir(char* nome, char clusterPai, char cluster, char tabela[], MetaDados metaDados){
/* Cria um diretório no primeiro cluster disponível dado o diretório atual
 * Entrada:
 *      String com o nome do diretorio dado pelo usuario
 *      Ponteiro (char) para o pai do diretório a ser criado
 *      Ponteiro (char) para o cluster diponível
 *      Tabela FAT que armazena os ponteiros dos discos
 * Retorno:
 *      1 caso seja realizado com sucesso
 *      0 caso a criação falhe
 */
    FILE *arq;
    NodoCluster novo = {"", "", 'a', '*'};

    //Marca o primeiro cluster disponivel como ocupado e escreve no arquivo
    if(!alteraTabelaFat(tabela, cluster, metaDados)){
        printf("Erro ao atualizar a tabela\n");
        return 0;
    }

    //Cria o novo Cluster
    strcpy(novo.nome, nome);
    novo.pai = clusterPai;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o cursor na linha do primeiro cluster disponível que será preenchido pelo novo diretório
    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * cluster), SEEK_SET);
    //Escreve o cluster no arquivo
    fwrite(&novo, sizeof(NodoCluster), 1, arq);

    fclose(arq);
    if(ferror(arq)){
        printf("Erro ao gravar o novo cluster\n");
        return 0;
    }

    if(!adicionaFilho(clusterPai, cluster, metaDados)){  //se houve erro ao adicionar um filho na lista de filhos
        printf("Erro ao atualizar o diretorio pai\n");
        return 0;
    }

    return 1;
}

int dir(char pai, MetaDados metaDados){
/* Lista todos os subdiretorios e arquivos dado um diretorio principal
 * Entrada:
 *      Char, que representa o ponteiro da pasta que queremos exibir os arquivos e diretórios
 * Retorno:
 *      1, caso consigamos ler com sucesso do arquivo
 *      0, caso haja falha na leitura
 */
    long i = 0;
    char aux = 0;
    FILE *arq;
    NodoCluster cluster;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o cursor do arquivo no cluster que desejamos listar as informações
    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * pai), SEEK_SET);

    //percorre o cluster até encontrar o marcador '*'
    while(aux != '*'){
        aux = fgetc(arq);
    }
    aux = fgetc(arq);
    //Verifica se a lista de filhos do cluster está vazia.
    if(aux == 0){
        printf("<vazio>");
    }else{     //Se não, percorre a lista de filhos até o final e printa os nomes na tela.
        while(aux != 0){
            //guarda a posição do filho atual
            i = ftell(arq);
            //Pega o cluster que está na lista de filhos
            if(!pegaCluster(aux, &cluster, metaDados)){
                fclose(arq);
                return 0;
            }
            printf("%s\n", cluster.nome);
            //retorna para a lista de filhos
            fseek(arq, i, SEEK_SET);
            aux = fgetc(arq);
        }
    }
    fclose(arq);
    printf("\n");

    if(ferror(arq)){
        printf("Nao foi possivel ler todos os subdiretorios\n");
        return 0;
    }
    return 1;
}

int cdRecursiva(ListaStrings *listaComandos, int *diretorioAtual, char subdir, MetaDados metaDados){
/* Caso o comando de CD nao for '..', precisamos recursivamente navegar pelas pastas
 * Entrada:
 *      Uma lista de strings, que representa o caminho de um diretório/arquivo
 *      Inteiro que representa o diretório em que foi realizado a operação
 *      Char que representa um subdiretorio para a funcao ser chamada recursivamente
 * Retorno:
 *      1 caso o caminho tenha sido encontrado
 *      0 caso o caminho nao tenha sido encontrado
 */
    char aux = 0;
    long i = 0;
    FILE *arq;
    NodoCluster dir;

    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Posiciona o ponteiro no cluster atual
    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * subdir), SEEK_SET);

    //Se nenhuma das opções acima for satisfeita, inicia a busca, com recursão,
    //até encontar, ou não, o caminho solicitado
    //Encontra o marcador de inicio da lista de filhos
    while(aux != '*'){
        aux = fgetc(arq);
    }
    aux = fgetc(arq);

    if(aux == 0){                           //Se o primeiro filho for igual a zero, a pasta pai não tem filho
        fclose(arq);                        //Fecha o arquivo e retorna 0, indicando erro.
        return 0;
    }else{                                  //Se não, busca se há uma pasta com o nome solicitado no caminho indicado
        while(aux != 0){
            i = ftell(arq);                 //guarda a posição atual na lista de filhos
            if(!pegaCluster(aux, &dir, metaDados)){    //pega o cluster indicado pelo filho
                printf("Erro na leitura do cluster\n");
                return 0;
            }
            //Verifica se o nome da pasta encontrada na lista de filhos possui o mesmo nome do caminho solicitado
            if(strcmp(listaComandos->comando, dir.nome) == 0){
                //Se sim, verifica se o próximo elemento da lista de comandos é 0
                if(listaComandos->prox == NULL){
                    //Caso seja, seta o diretório buscado como atual e retorna 1
                    *diretorioAtual = aux;
                    fclose(arq);
                    return 1;
                }else{//Se o próximo elemento da lista de comandos não é 0, chama a função recursivamente.
                    return cdRecursiva(listaComandos->prox, diretorioAtual, aux, metaDados);
                }
            }
            fseek(arq, i, SEEK_SET);
            aux = fgetc(arq);
        }
    }

    fclose(arq);
    if(ferror(arq)){
        printf("Erro na leitura do arquivo\n");
        return 0;
    }
    return 1;
}

int cd(ListaStrings *listaComandos, int *diretorioAtual, MetaDados metaDados){
/* Muda o diretorio atual para algum subdiretorio
 * Entrada:
 *      Uma lista de strings, que representa o caminho de um diretório/arquivo
 *      Inteiro que representa o diretório em que foi realizado a operação
 * Retorno:
 *      1 caso o caminho tenha sido encontrado
 *      0 caso o caminho nao tenha sido encontrado
 */
    NodoCluster dir;
    FILE *arq;

    //Verifica se a lista de comando é vazia
    if(listaComandos == NULL){
        return 1;
    }

    //Se nao for, abre o arquivo
    if((arq = fopen("ArqDisco.bin", "r+b")) == NULL){
        printf("Erro na abertura do arquivo\n");
        return 0;
    }

    //Pega as informacoes do diretorio atual
    fseek(arq, metaDados.initCluster + 2 + ((metaDados.tamCluster + 1) * (*diretorioAtual)), SEEK_SET);
    fread(&dir, sizeof(NodoCluster), 1, arq);
    fclose(arq);
    if(strcmp(listaComandos->comando, "..") == 0 && !ferror(arq)){  //Verifica se o comando digitado foi o de "voltar"
        if(*diretorioAtual != 0){                                   //Se o diretório atual é diferente do root, volta para a pasta "pai"
            *diretorioAtual = dir.pai;
            return 1;
        }                                                           //Senao, fecha o arquivo e retorna 1, indicando erro.
        return 0;
    }

    //Senao for o comando '..', percorre recursivamente pelas pastas
    return(cdRecursiva(listaComandos, diretorioAtual, *diretorioAtual, metaDados));
}

void detectaComando(char comando[], int *dirAtual, char tabela[], short int* sair, MetaDados metaDados){
/* Detecta os possíveis comandos exigidas pelo usuário,
 * separando a operação do possível nome de diretórios e arquivos
 * Entrada:
 *      Input de comando dado pelo ususario
 *      Ponteiro (linha) do diretorio atual
 *      Tabela FAT que armazena os ponteiros dos discos
 *      Flag que controla o comando de saida do usuario
 */
    int clusterDisponivel;
    char *operacao = NULL, *nome = NULL;
    ListaStrings *listaComandos;

    if(strcmp(comando, "")){   //se foi dado algum input
        pegaOperacaoNome(comando, &operacao, &nome);

        //MKFILE
        if(strcmp(operacao, "MKFILE") == 0){
            printf("Criar Arquivo");

        //MKDIR
        }else if(strstr(operacao, "MKDIR") != NULL){
            clusterDisponivel = primeiraPosicaoDisponivel(tabela, metaDados);
            if((nome != NULL) && (clusterDisponivel != -1) && (mkDir(nome, *dirAtual, clusterDisponivel, tabela, metaDados))){
                printf("Diretorio Criado!\n");
            }else{
                printf("Erro ao criar o diretorio\n");
            }

        //DIR
        }else if(strstr(operacao, "DIR") != NULL){
            dir(*dirAtual, metaDados);

        //CD
        }else if(strcmp(operacao, "CD") == 0){
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            if(!cd(listaComandos, dirAtual, metaDados)){
                printf("Caminho nao encontrado\n");
            }
            apagaLSE(listaComandos);
        }

        //RM
        else if(strcmp(operacao, "RM") == 0){
            printf("Deletar arquivo/direorio\n");

        //EDIT
        }else if(strstr(operacao, "EDIT") != NULL){
            printf("Editar arquivo\n");

        //MOVE
        }else if(strcmp(operacao, "MOVE") == 0){
            printf("Mover diretorio/arquivo\n");

        //RENAME
        }else if(strcmp(operacao, "RENAME") == 0){
            printf("Renomear arquivo/diretorio\n");

        //EXIT
        }else if(strcmp(operacao, "EXIT") == 0){
            *sair = 1;

        //UNDEFINED
        }else{
            printf("Comando nao reconhecido.\n");
        }
    }
}
