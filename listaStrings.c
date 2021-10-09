#include <stdlib.h>
#include <string.h>
#include "estruturas.h"

ListaStrings* inserirLSEStrings(ListaStrings* lseString, char * str){
/* ListaStrings*, char* -> ListaStrings*
 * Dado um ponteiro para uma Lista de Strings e uma String, insere a String na lista.
 * Se a lista está vazia, insere na primeira posição
 * Caso contrário, percorre toda a lista e insere no final.
 */
    ListaStrings *novo, *aux;
    novo = (ListaStrings*) malloc(sizeof(ListaStrings));
    novo->comando = str;
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

ListaStrings* apagaLSE(ListaStrings* ptNum){
/* Libera toda a memória alocada e ocupada por uma LSE
 * Entrada:
 *      Ponteiro para o início da lista
 * Retorno:
 *      Ponteiro NULL
 */
    ListaStrings* aux;
    while(ptNum != NULL){
        aux = ptNum;
        ptNum = ptNum->prox;
        free(aux);
    }
    return NULL;
}
