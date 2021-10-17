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

void inicializaCaminho(char** caminho){
/* Aloca espaco na memoria para guardar a string 'root' na variavel caminho inicializando-a
 * Entrada:
 *      Ponteiro de ponteiro para string caminho
 */
    *caminho = (char *) malloc(5*sizeof(char));
    strcpy(*caminho, "root");
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

void pegaOperacaoNome(char comando[], char** operacao, char** nome, char** resto){
/* Dado um comando do usuario, pega a operacao e o possível nome do diretorio ou arquivo fornecido
 * Entrada:
 *      String do comando todo fornecido pelo usuário
 *      Ponteiro para a string que armazena a operação
 *      Ponteiro para a string que armazena o nome do diretorio ou arquivo
 */
    *operacao = strtok(comando, " ");
    *nome = strtok(NULL, " ");
    *resto = strtok(NULL, " ");
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
 *      Char que presenta o cluster a ser atualizado na tabela
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

    fseek(arq, metaDados.initIndice + ponteiroCluster, SEEK_SET); //vai ate o indice correto da tabela FAT
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
    int aux = 0, *comandoAux = NULL;
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

void voltaCaminho(char** caminho){
/* Funcao que dado uma string representando um caminho de diretorios,
 * transforma numa string com um caminho a menos, de acordo com o comando 'CD .."
 * Entrada:
 *      Ponteiro para ponteiro da string que armazena o caminho
 */
    int index = strlen(*caminho) - 1;

    while(*(*caminho + index) != '/'){    //procura a ultima barra no caminho
        index--;
    }

    *(*caminho + index) = '\0';           //substitui por '\0'
    *caminho = (char *) realloc(*caminho, sizeof(char) * (index + 1)); //realloca espaco de acordo com o novo tamanho
}

void avancaCaminho(char **caminho, ListaStrings *listaComandos){
/* Funcao que avanca um caminho dado o caminho atual e o novo nome a ser adicionado
 * Entrada:
 *      Ponteiro para ponteiro da string que armazena o caminho
 *      Lista de strings com o caminho todo
 */
    int tamanhoRealloc = 0;
    ListaStrings *aux;

    aux = listaComandos;
    while(aux != NULL){
        tamanhoRealloc += strlen(aux->comando) + 1; //soma o tamanho da nova string a ser concatenada junto com as barras
        aux = aux->prox;
    }

    //realloca espaco para a nova string
    *caminho = (char *) realloc(*caminho, sizeof(char) * (strlen(*caminho) + tamanhoRealloc + 1)); //realoca contando com o '\0'
    aux = listaComandos;
    while(aux != NULL){
        strcat(*caminho, "/");
        strcat(*caminho, aux->comando);  //concatena todos os novos caminhos
        aux = aux->prox;
    }
}

void reconstroiCaminho(char **caminho, ListaStrings *listaComandos){
/* Reconstroi todo o caminho da string, limpando-a e colocando o novo caminho dado pelo usuario
 * Entrada:
 *      Ponteiro para ponteiro da string que armazena o caminho
 *      Lista de strings com o caminho todo
 */
    int tamanhoRealloc = 0;
    ListaStrings *aux;

    aux = listaComandos;
    while(aux != NULL){
        tamanhoRealloc += strlen(aux->comando) + 1; //soma o tamanho da nova string a ser concatenada junto com as barras
        aux = aux->prox;
    }

    //realloca espaco para a nova string
    *caminho = (char *) realloc(*caminho, sizeof(char) * (tamanhoRealloc + 1)); //realoca contando com o '\0'
    strcpy(*caminho, "root");  //copia o 'root'
    aux = listaComandos->prox;
    while(aux != NULL){
        strcat(*caminho, "/");
        strcat(*caminho, aux->comando);  //concatena todos os novos caminhos se houverem
        aux = aux->prox;
    }
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

    lsNome = inserirLSEStrings(lsNome, nome);

    if(encontraCaminho(lsNome, clusterPai, clusterPai, metaDados) != -1){
        apagaLSE(lsNome);
        printf("Ja existe um diretorio com esse nome.\n");
        return 0;
    }
     apagaLSE(lsNome);
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

    aux = fgetc(arq);                               //Verifica se o cluster tem filhos
    if(aux == 0){                                   //Se não tem filhos
        fclose(arq);                                //Fecha arquivo
        return 1;
    }else{                                          //Se o cluster tem filhos
        while(aux != 0){
            if(aux != 254)                          //Se nao for um filho removido
                removeCluster(aux, metaDados);      //Chama a função recursivamente para a sub-árvore do filho
            aux = fgetc(arq);                       //pega o proximo filho
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
        if(strstr(operacao, "MKDIR") != NULL){
            if(primeiraPosicaoDisponivel(&clusterDisponivel, metaDados)){   //se encontrou cluster disponivel
                if((nome != NULL) && (mkDir(nome, *dirAtual, clusterDisponivel, metaDados))){
                    printf("Diretorio Criado!\n");
                }else{
                    printf("Erro ao criar o diretorio\n");
                }
            }

        //DIR
        }else if(strstr(operacao, "DIR") != NULL){
            if(!dir(*dirAtual, metaDados)){
                printf("Nao foi possivel listar todos os diretorios e arquvios\n");
            }

        //CD
        }else if(strcmp(operacao, "CD") == 0){
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
        }else if(strcmp(operacao, "MKFILE") == 0){
            if(primeiraPosicaoDisponivel(&clusterDisponivel, metaDados)){
                nome = strtok(nome, ".");
                extensao = strtok(NULL, ".");
                if((nome != NULL) && (extensao != NULL) && (mkFile(nome, extensao, *dirAtual, clusterDisponivel, metaDados))){
                    printf("Arquivo Criado!\n");
                }else{
                    printf("Erro ao criar o arquivo\n");
                }
            }

        //RM
        }else if(strcmp(operacao, "RM") == 0){
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
        }else if(strstr(operacao, "EDIT") != NULL){
            printf("Editar arquivo\n");

        //MOVE
        }else if(strcmp(operacao, "MOVE") == 0){
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
        }else if(strcmp(operacao, "RENAME") == 0){
            listaComandos = NULL;
            listaComandos = pegaSequenciaComandos(nome, listaComandos);
            if((listaComandos != NULL) && (resto != NULL) && reName(listaComandos, resto, dirAtual, metaDados)){
                printf("Renomeado com sucesso\n");
            }else{
                printf("Nao foi possivel renomear\n");
            }
            apagaLSE(listaComandos);

        //EXIT
        }else if(strcmp(operacao, "EXIT") == 0){
            *sair = 1;

        //UNDEFINED
        }else{
            printf("Comando nao reconhecido.\n");
        }
    }
}

