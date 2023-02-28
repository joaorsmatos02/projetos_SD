// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include "../include/client_stub.h"
#include "../include/client_stub-private.h"
#include "../include/data.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../include/sdmessage.pb-c.h"
#include "../include/network_client.h"


/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port){
    struct rtree_t* connection = malloc(sizeof(struct rtree_t));
    
    if(connection == NULL)
        return NULL;

    char* ipPortBuffer = strtok((char*) address_port, ":");
    char** ipPortTokens = (char**) malloc(sizeof(char*) * 2);
    int tokenQuantity = 0;
    
    // extrair os tokens o array inputTokens
    for(;ipPortBuffer != NULL; tokenQuantity++) {
        ipPortTokens[tokenQuantity] = ipPortBuffer;
        ipPortBuffer = strtok(NULL, " ");
    }

    connection->ip_addr = ipPortTokens[0];
    connection->port = ipPortTokens[1];

    free(ipPortTokens);
    free(ipPortBuffer);

    if(network_connect(connection) == 0)
        return connection;
        
    free(connection);

    return NULL;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree){
    int result = network_close(rtree);
    free(rtree);
    return result;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    struct _EntryT entry_copy;
    entry_t__init(&entry_copy);

    struct _DataT data_copy;
    data_t__init(&data_copy);

    entry_copy.key = entry->key;
    data_copy.data = entry->value->data;
    data_copy.datasize = entry->value->datasize;
    entry_copy.value = &data_copy;
    messageSend.opcode = MESSAGE_T__OPCODE__OP_PUT;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    messageSend.entry = &entry_copy;
    
    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_PUT+1) {
        int verify = messageRecieve->verify;
        message_t__free_unpacked(messageRecieve, NULL);
        return verify;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return -1;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return -1;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GET;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    messageSend.key = key;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_GET+1) {
        struct data_t* data = data_create(messageRecieve->data->datasize);
        memcpy(data->data, messageRecieve->data->data, data->datasize);
        message_t__free_unpacked(messageRecieve, NULL);
        return data;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return NULL;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_DEL;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    messageSend.key = key;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_DEL+1) {
        int verify = messageRecieve->verify;
        message_t__free_unpacked(messageRecieve, NULL);
        return verify;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return -1;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return -1;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_SIZE+1) {
        int size = messageRecieve->size_height;
        message_t__free_unpacked(messageRecieve, NULL);
        return size;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return -1;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return -1;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_HEIGHT+1) {
        int height = messageRecieve->size_height;
        message_t__free_unpacked(messageRecieve, NULL);
        return height;
    }  
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return -1;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return -1;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_GETKEYS+1) {

        char** keys = malloc(sizeof(char*) * (messageRecieve->n_keys + 1));

        for(int i = 0; i < messageRecieve->n_keys; i++) {
            keys[i] = calloc(messageRecieve->keys[i].len + 1, sizeof(char));
            memcpy(keys[i], messageRecieve->keys[i].data, messageRecieve->keys[i].len);
        }      

        keys[messageRecieve->n_keys] = NULL;

        message_t__free_unpacked(messageRecieve, NULL);

        return keys;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return NULL;
    }

    message_t__free_unpacked(messageRecieve, NULL);

    return NULL;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void **rtree_get_values(struct rtree_t *rtree){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_GETVALUES+1) {
        void** result = malloc((messageRecieve->n_values + 1) * sizeof(void*));

        for(int i = 0; i < messageRecieve->n_values; i++) {
            result[i] = malloc(messageRecieve->values[i].len);
            memcpy(result[i], messageRecieve->values[i].data, messageRecieve->values[i].len);
        }
        result[messageRecieve->n_values] = NULL;

        message_t__free_unpacked(messageRecieve, NULL);
        return result;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return NULL;
    }
    message_t__free_unpacked(messageRecieve, NULL);

    return NULL;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int rtree_verify(struct rtree_t *rtree, int op_n) {
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_RESULT;

    messageSend.verify = op_n;

    struct _MessageT* messageRecieve = network_send_receive(rtree, &messageSend);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_VERIFY + 1) {
        int verify = messageRecieve->verify;
        message_t__free_unpacked(messageRecieve, NULL);
        return verify;
    }
    else if (messageRecieve->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
        message_t__free_unpacked(messageRecieve, NULL);
        return -1;
    }

    return 0;
}

//TRATAR OS -1