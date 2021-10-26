/*****************************************************************************************************/
/*                                   FUNCOES AUXILIARES                                              */
/*****************************************************************************************************/
/* Funcoes que manipulam strings e alocam precisamente o espaco necessario para armazena-las.        */
/* Servem tanto para receber o input do usuario quanto para construir strings visiveis no terminal.  */
/*****************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estruturas.h"

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
 *      Ponteiro para a string que armazena qualquer informacao restante
 */
    *operacao = strtok(comando, " ");
    *nome = strtok(NULL, " ");
    *resto = strtok(NULL, "\0");
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
