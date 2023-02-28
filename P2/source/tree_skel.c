// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include "../include/tree_skel.h"
#include "../include/sdmessage.pb-c.h"
#include "../include/tree.h"
#include "../include/entry.h"
#include "../include/data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tree_t* tree;

/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(){
    tree = tree_create();

    if(tree == NULL)
        return -1;

    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy(){
    tree_destroy(tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct _MessageT *msg){

    if(tree == NULL || msg == NULL)
        return -1;

    int result = -1;
    
    switch(msg->opcode) {
        case MESSAGE_T__OPCODE__OP_SIZE:msg->size_height = tree_size(tree);
                                        result = 0;
                                        break;
        case MESSAGE_T__OPCODE__OP_HEIGHT:  msg->size_height = tree_height(tree);
                                            result = 0;
                                            break;
        case MESSAGE_T__OPCODE__OP_DEL: result = tree_del(tree, msg->key);
                                        break;
        case MESSAGE_T__OPCODE__OP_GET: struct data_t* new_data1 = tree_get(tree, msg->key);
                                        if(new_data1 == NULL)
                                            break;
                                        
                                        struct _DataT* new_data2 = (struct _DataT*) malloc(sizeof(struct _DataT));
                                        if(new_data2 == NULL)
                                            return -1;
                                        data_t__init(new_data2);

                                        new_data2->data = new_data1->data;
                                        new_data2->datasize = new_data1->datasize;
                                        msg->data = new_data2;
                                        free(new_data1);
                                        result = 0;
                                        break;
        case MESSAGE_T__OPCODE__OP_PUT: char* key = msg->entry->key;
                                        void* data = msg->entry->value->data;
                                        int datasize = msg->entry->value->datasize;

                                        struct data_t* new_data3 = data_create2(datasize, data);
                                        result = tree_put(tree, key, new_data3);
                                        free(new_data3);

                                        break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: size_t size = tree_size(tree);
                                            char** keys = tree_get_keys(tree);

                                            msg->keys = (struct ProtobufCBinaryData*) malloc(size * sizeof(struct ProtobufCBinaryData));

                                            for(int i = 0; i < size; i++) {
                                                msg->keys[i].data = malloc(strlen(keys[i]) + 1);
                                                memcpy(msg->keys[i].data, keys[i], strlen(keys[i]) + 1);
                                                msg->keys[i].len = strlen(keys[i]);
                                                free(keys[i]);       
                                            }
                                            free(keys);

                                            msg->n_keys = size;
                                            
                                            result = 0;
                                            break;
        case MESSAGE_T__OPCODE__OP_GETVALUES:   void** values = tree_get_values(tree);

                                                msg->values = (struct ProtobufCBinaryData*) malloc(tree_size(tree) * sizeof(struct ProtobufCBinaryData));

                                                for(int i = 0; i < tree_size(tree); i++) {
                                                    msg->values[i].data = malloc(((struct data_t*) values[i])->datasize + 1);
                                                    msg->values[i].len = ((struct data_t*) values[i])->datasize;
                                                    memcpy(msg->values[i].data, ((struct data_t*) values[i])->data, ((struct data_t*) values[i])->datasize);
                                                    free(values[i]);
                                                }
                                                free(values);

                                                msg->n_values = tree_size(tree);
                                                    
                                                result = 0;
                                                break;
        default: break;
    }

    if(result == 0)
        msg->opcode = msg->opcode + 1;
    else 
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;

    return 0;
}
