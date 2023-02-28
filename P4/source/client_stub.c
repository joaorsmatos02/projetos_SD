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
#include <zookeeper/zookeeper.h>

zhandle_t* zh;
struct rtree_t* head;
struct rtree_t* tail;
int is_connected;

static char* watcher_ctx = "Zookeeper Data Watcher";

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
int rtree_connect() {

    int retval, retval1 = -1;
    const char* zoo_root = "/chain";
                                                                                                            
	struct String_vector* children_list = (struct String_vector *) malloc(sizeof(struct String_vector));
    
    // ##############################################################################################################
	/* Get the list of children synchronously */
	retval = zoo_wget_children(zh, zoo_root, child_watcher, watcher_ctx, children_list);                  // 1 para fazer watch aos filhos
	if (retval != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", zoo_root);
	    exit(EXIT_FAILURE);
	}
	fprintf(stderr, "\n=== znode listing === [ %s ]", zoo_root); 
	for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]); 
	}
	fprintf(stderr, "\n=== done ===\n");
    // ##############################################################################################################

    char* max_server_path = calloc(40, sizeof(char));
    char* min_server_path = calloc(40, sizeof(char));
    strcat(max_server_path , "/chain/");
    strcpy(max_server_path + 7, children_list->data[0]);
    strcat(min_server_path , "/chain/");
    strcpy(min_server_path + 7, children_list->data[0]);

    // Gets the max and min server id superior to the current server id
    for (int i = 0; i < children_list->count; i++)  {
        char* path = strdup("/chain/");
        strcat(path, children_list->data[i]);

		if(strcmp(path, max_server_path) > 0) {
            strcpy(max_server_path + 7, children_list->data[i]);
        }
        if(strcmp(path, min_server_path) < 0) {
            strcpy(min_server_path + 7, children_list->data[i]);
        }
        free(children_list->data[i]);
        free(path);
	}

    // Ir buscar a informacao dos servidores head e tail

    char* head_buffer = malloc(25);
    char* tail_buffer = malloc(25);
    int buffer_len = 25;

    retval1 = zoo_get(zh, max_server_path, 1, tail_buffer, &buffer_len, NULL);

    if (retval1 != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", max_server_path);
	    exit(EXIT_FAILURE);
	}

    retval1 = zoo_get(zh, min_server_path, 1, head_buffer, &buffer_len, NULL);

    if (retval1 != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", max_server_path);
	    exit(EXIT_FAILURE);
	}

    char* ipPortBuffer1 = strtok((char*) head_buffer, ":");
    char** ipPortTokens1 = (char**) malloc(sizeof(char*) * 2);
    int tokenQuantity1 = 0;
    
    // extrair os tokens o array inputTokens
    for(;ipPortBuffer1 != NULL; tokenQuantity1++) {
        ipPortTokens1[tokenQuantity1] = ipPortBuffer1;
        ipPortBuffer1 = strtok(NULL, " ");
    }

    head = malloc(sizeof(struct rtree_t));

    head->ip_addr = ipPortTokens1[0];
    head->port = ipPortTokens1[1];

    free(ipPortTokens1);
    free(ipPortBuffer1);

    if(network_connect(head) != 0)
        printf("Connection head ERROR");

    char* ipPortBuffer2 = strtok((char*) tail_buffer, ":");
    char** ipPortTokens2 = (char**) malloc(sizeof(char*) * 2);
    int tokenQuantity2 = 0;
    
    // extrair os tokens o array inputTokens
    for(;ipPortBuffer2 != NULL; tokenQuantity2++) {
        ipPortTokens2[tokenQuantity2] = ipPortBuffer2;
        ipPortBuffer2 = strtok(NULL, " ");
    }

    tail = malloc(sizeof(struct rtree_t));

    tail->ip_addr = ipPortTokens2[0];
    tail->port = ipPortTokens2[1];

    free(ipPortTokens2);
    free(ipPortBuffer2);

    if(network_connect(tail) != 0)
        printf("Connection tail ERROR");

    return 0;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(){
    int result1 = network_close(head);
    int result2 = network_close(tail);
    free(head);
    free(tail);
    return result1 + result2;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct entry_t *entry){
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
    
    struct _MessageT* messageRecieve = network_send_receive(head, &messageSend);
    sleep(1);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_PUT+1) {
        int verify = messageRecieve->verify;

        while(rtree_verify(verify) == -1) {
            rtree_put(entry);
            sleep(1);
        }

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
struct data_t *rtree_get(char *key){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GET;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    messageSend.key = key;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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
int rtree_del(char *key){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_DEL;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    messageSend.key = key;

    struct _MessageT* messageRecieve = network_send_receive(head, &messageSend);
    sleep(1);

    if(messageRecieve->opcode == MESSAGE_T__OPCODE__OP_DEL+1) {
        int verify = messageRecieve->verify;

        while(rtree_verify(verify) == -1) {
            rtree_del(key);
            sleep(1);
        }

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
int rtree_size(){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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
int rtree_height(){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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
char **rtree_get_keys(){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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
void **rtree_get_values(){
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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
int rtree_verify(int op_n) {
    struct _MessageT messageSend;
    message_t__init(&messageSend);

    messageSend.opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    messageSend.c_type = MESSAGE_T__C_TYPE__CT_RESULT;

    messageSend.verify = op_n;

    struct _MessageT* messageRecieve = network_send_receive(tail, &messageSend);

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

/**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	}
}

void zk_connect(char* address_port) {
    zh = zookeeper_init(address_port, connection_watcher, 2000, 0, NULL, 0); 
	if (zh == NULL)	{
	    exit(EXIT_FAILURE); 
	}
}

void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    struct String_vector* children_list =	(struct String_vector*) malloc(sizeof(struct String_vector));
    if (state == ZOO_CONNECTED_STATE)	 {
        if (type == ZOO_CHILD_EVENT) {
            if(rtree_connect() != 0)
                return;
        }
        free(children_list);
    }
}