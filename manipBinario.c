/*****************************************************************************************************/
/*                                   FUNCOES AUXILIARES                                              */
/*****************************************************************************************************/
/* Funcoes que criam, leem, adicionam, editam e removem informacoes do arquivo binario diretamente.  */
/* Com elas conseguimos pegar informacoes dos metadados, tabela de indices e clusters.               */
/*****************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estruturas.h"

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
 *      Metadados para auxiliar na busca dentro do arquivo binario
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
