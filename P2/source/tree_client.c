// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client_stub.h"
#include "../include/tree_client.h"
#include "../include/entry.h"
#include "../include/data.h"
#include <signal.h>

int main(int argc, char* argv[]) {

    signal(SIGPIPE, SIG_IGN);

    //estabelecer conexao com servidor
    struct rtree_t* remoteTree = rtree_connect(argv[1]);
    //verificar se a conexao foi bem sucedida
    if(remoteTree == NULL)
        exit(-1);

    char* bufferStdio = malloc(256);

    printMenu();

    printf("Insira o seu pedido: ");

    char** inputTokens = malloc(sizeof(char*) * 3); 
    int tokenQuantity = 0;

    while(1) {
        // ler do stdin
        fgets(bufferStdio, 256, stdin);
        bufferStdio[strcspn(bufferStdio, "\n")] = 0;
        char* tokenBuffer = strtok(bufferStdio, " ");
        

        // extrair os tokens o array inputTokens
        for(;tokenBuffer != NULL; tokenQuantity++) {
            inputTokens[tokenQuantity] = tokenBuffer;
            tokenBuffer = strtok(NULL, " ");
        }

        if(strcmp(inputTokens[0], "quit") == 0 && tokenQuantity == 1)
            break;

        //process the input
        inputHandler(inputTokens, tokenQuantity, remoteTree);

        tokenQuantity = 0;
    }

    free(inputTokens);
    free(bufferStdio);

    return rtree_disconnect(remoteTree);
}

void inputHandler(char** inputTokens, int tokenQuantity, struct rtree_t* rtree) {

    int result = -1;

    if(strcmp(inputTokens[0], "put") == 0) {
        
        if(tokenQuantity == 3) {
            struct data_t* value = data_create(strlen(inputTokens[2]) + 1);
            memcpy(value->data, inputTokens[2], value->datasize);

            struct entry_t* entry = entry_create(strdup(inputTokens[1]), value);
            
            result = rtree_put(rtree, entry);
            entry_destroy(entry);
        }
    }
    else if(strcmp(inputTokens[0], "get") == 0) {
        if(tokenQuantity == 2) {
            struct data_t* data = rtree_get(rtree, inputTokens[1]);

            if(data != NULL) {
                result = 0;
                printf("Answer: %s\n", (char *) data->data); //assumindo que e string
            }

            data_destroy(data);
        }
    }
    else if(strcmp(inputTokens[0], "del") == 0) {
        if(tokenQuantity == 2)
            result = rtree_del(rtree, inputTokens[1]);
    }
    else if(strcmp(inputTokens[0], "size") == 0) {
        if(tokenQuantity == 1) {
            int sizeTree = rtree_size(rtree);
            printf("Answer: %d\n", sizeTree);
            result = 0;
        }
    }
    else if(strcmp(inputTokens[0], "height") == 0) {
        if(tokenQuantity == 1) {
            int heightTree = rtree_height(rtree);
            printf("Answer: %d\n", heightTree);
            result = 0;
        }
    }
    else if(strcmp(inputTokens[0], "getkeys") == 0) {
        if(tokenQuantity == 1) {
            char** keys = rtree_get_keys(rtree);
            result = 0;

            printf("Answer: \n");
            for(int i = 0; keys[i] != NULL; i++) {
                printf("%s\n", keys[i]);
                free(keys[i]);
            }

            free(keys);
        }
    }
    else if(strcmp(inputTokens[0], "getvalues") == 0) {
        if(tokenQuantity == 1) {
            char** values = (char**) rtree_get_values(rtree);
            result = 0;

            printf("Answer: \n");
            for(int i = 0; values[i] != NULL; i++) {
                printf("%s\n", values[i]);
                free(values[i]);
            }

            free(values);
        }
    }

    if (result == 0)
        printf("Operação bem sucedida!\n");
    else
        printf("Erro na operação...\n\nInsira um pedido válido: ");

    return;
}

void printMenu() {
    printf("MENU OPTIONS: \n\n");
    printf("put <key> <data>    - Esta função permite ao utilizador colocar uma entrada na árvore uma chave(key) e um valor(data)\n\n");
    printf("get <key>           - Devolve a entrada da árvore identificada pela chave(key)\n\n");
    printf("del <key>           - Elimina uma entrada da árvore identificada pela chave(key)\n\n");
    printf("size                - Devolve o tamanho da árvore\n\n");
    printf("height              - Devolve a altura da árvore\n\n");
    printf("getkeys             - Devolve todas as chaves(keys) das entradas da árvore\n\n");
    printf("getvalues           - Devolve todos os valores(values) de todas as entradas da árvore\n\n");
    printf("quit                - Termina o programa\n");

    return;
}