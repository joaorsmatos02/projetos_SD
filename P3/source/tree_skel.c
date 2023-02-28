// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include "../include/tree_skel.h"
#include "../include/sdmessage.pb-c.h"
#include "../include/tree.h"
#include "../include/entry.h"
#include "../include/data.h"
#include "../include/tree_skel-private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct tree_t* tree;
int last_assigned;
struct op_proc* opProc;
struct request_t* queue_head;
int nrRequests;
pthread_mutex_t queueLock, treeLock, opProcLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_t* threads;
int nrThreads;
int terminate;
int* params;

/* Inicia o skeleton da árvore.
* O main() do servidor deve chamar esta função antes de poder usar a
* função invoke(). 
* A função deve lançar N threads secundárias responsáveis por atender 
* pedidos de escrita na árvore.
* Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
*/
int tree_skel_init(int N) {
    tree = tree_create();

    terminate = 0;

    nrThreads = N;

    threads = malloc(N * sizeof(pthread_t));

    last_assigned = 1;

    opProc = malloc(sizeof(struct op_proc));
    opProc->max_proc = 0;
    opProc->in_progress = calloc(N, sizeof(int));

    //inicialização do queue_head
    queue_head = malloc(sizeof(struct request_t));
    queue_head->op_n = -1;
    queue_head->op = -1;
    queue_head->key = NULL;
    queue_head->data = NULL;
    queue_head->next_op = NULL;
    nrRequests = 0;

    if(tree == NULL)
        return -1;

    params = malloc(nrThreads * sizeof(int));

    for(int i = 0; i < N; i++) {
        params[i] = i;
        pthread_create(&(threads[i]), NULL, &process_request, (void*) &params[i]);
    }

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
    
    if(msg->opcode == MESSAGE_T__OPCODE__OP_SIZE) {
        msg->size_height = tree_size(tree);
        result = 0;
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_HEIGHT){
        msg->size_height = tree_height(tree);
        result = 0;
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        struct request_t* new_request = malloc(sizeof(struct request_t));

        new_request->op_n = last_assigned;
        new_request->op = 0;
        new_request->key = strdup(msg->key);
        new_request->data = NULL;
        new_request->next_op = NULL;

        pthread_mutex_lock(&queueLock);
        if(nrRequests == 0) {
            queue_head->op_n = new_request->op_n;
            queue_head->op = new_request->op;
            queue_head->key = new_request->key;
            queue_head->data = new_request->data;
            queue_head->next_op = NULL;
            nrRequests += 1;
        }
        else {
            struct request_t* temp = queue_head;

            for(int i = 0; i < nrRequests - 1; i++)
                temp = temp->next_op;

            temp->next_op = malloc(sizeof(struct request_t));
            temp->next_op->op_n = new_request->op_n;
            temp->next_op->op = new_request->op;
            temp->next_op->key = new_request->key;
            temp->next_op->data = new_request->data;
            temp->next_op->next_op = NULL;
            nrRequests += 1;
        }

        last_assigned += 1;

        msg->verify = last_assigned - 1;
        result = 0;
        pthread_mutex_unlock(&queueLock);
        pthread_cond_broadcast(&cond);

        free(new_request);
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GET) {
        struct data_t* new_data1 = tree_get(tree, msg->key);
        if(new_data1 != NULL) {
            struct _DataT* new_data2 = (struct _DataT*) malloc(sizeof(struct _DataT));
            if(new_data2 == NULL)
                return -1;
            data_t__init(new_data2);

            new_data2->data = new_data1->data;
            new_data2->datasize = new_data1->datasize;
            msg->data = new_data2;
            free(new_data1);
            result = 0;
        }
    } 
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_PUT) {
        struct request_t* new_request = malloc(sizeof(struct request_t));

        new_request->op_n = last_assigned;
        new_request->op = 1;
        new_request->key = strdup(msg->entry->key);
        new_request->data = strdup(msg->entry->value->data);
        new_request->next_op = NULL;

        pthread_mutex_lock(&queueLock);
        if(nrRequests == 0) {
            queue_head->op_n = new_request->op_n;
            queue_head->op = new_request->op;
            queue_head->key = new_request->key;
            queue_head->data = new_request->data;
            queue_head->next_op = NULL;
            nrRequests += 1;
        }
        else {
            struct request_t* temp = queue_head;

            for(int i = 0; i < nrRequests - 1; i++)
                temp = temp->next_op;

            temp->next_op = malloc(sizeof(struct request_t));
            temp->next_op->op_n = new_request->op_n;
            temp->next_op->op = new_request->op;
            temp->next_op->key = new_request->key;
            temp->next_op->data = new_request->data;
            temp->next_op->next_op = NULL;
            nrRequests += 1;
        }

        last_assigned += 1;

        msg->verify = last_assigned - 1;
        result = 0;
        pthread_mutex_unlock(&queueLock);
        pthread_cond_broadcast(&cond);

        free(new_request);
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS) {
        size_t size = tree_size(tree);
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
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_GETVALUES) {
        void** values = tree_get_values(tree);

        msg->values = (struct ProtobufCBinaryData*) malloc(tree_size(tree) * sizeof(struct ProtobufCBinaryData));

        for(int i = 0; i < tree_size(tree); i++) {
            msg->values[i].data = malloc(((struct data_t*) values[i])->datasize + 1);
            msg->values[i].len = ((struct data_t*) values[i])->datasize;
            memcpy(msg->values[i].data, ((struct data_t*) values[i])->data, ((struct data_t*) values[i])->datasize);
            free(((struct data_t*) values[i])->data);
            free(((struct data_t*) values[i]));
        }
        free(values);

        msg->n_values = tree_size(tree);
            
        result = 0;
    }
    else if(msg->opcode == MESSAGE_T__OPCODE__OP_VERIFY) {
        msg->verify = verify(msg->verify);

        result = 0;
    }

    if(result == 0)
        msg->opcode = msg->opcode + 1;
    else 
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;

    return 0;
}

/* Verifica se a operação identificada por op_n foi executada.
*/
int verify(int op_n) {
    if(op_n > opProc->max_proc)
        return 0;
    else if (op_n == opProc->max_proc && op_n != 0) {
        return 1;
    }
    else {
        for(int i = 0; i < nrThreads; i++) {
            if(opProc->in_progress[i] == op_n || op_n <= 0)
                return 0;
        }
    }

    return 1;
}

/* Função da thread secundária que vai processar pedidos de escrita.
*/
void* process_request (void *params) {

    while(1) {
        pthread_mutex_lock(&queueLock);

        while(queue_head->op_n == -1) {
            if(terminate == 1) {
                pthread_mutex_unlock(&queueLock);
                pthread_exit(NULL);
            }
            pthread_cond_wait(&cond, &queueLock);
        }

        pthread_mutex_lock(&opProcLock);
        opProc->in_progress[*((int*) params)] = queue_head->op_n;
        pthread_mutex_unlock(&opProcLock);

        //Processamento DELETE
        if(queue_head->op == 0) {
            pthread_mutex_lock(&treeLock);
            tree_del(tree, queue_head->key);
            pthread_mutex_unlock(&treeLock);
        }
        //Processamento PUT
        else if(queue_head->op == 1) {
            char* key = queue_head->key;
            void* data = queue_head->data;
            int datasize = strlen(queue_head->data) + 1;

            struct data_t* new_data3 = data_create2(datasize, data);

            pthread_mutex_lock(&treeLock);
            tree_put(tree, key, new_data3);
            free(new_data3);
            pthread_mutex_unlock(&treeLock);
        }

        pthread_mutex_lock(&opProcLock);
        if(queue_head->op_n > opProc->max_proc)
            opProc->max_proc = queue_head->op_n;
        
        opProc->in_progress[*((int*) params)] = -1;
        pthread_mutex_unlock(&opProcLock);

        //apagar o pedido acabado de processar
        struct request_t* next = queue_head->next_op;
        if(queue_head->op == 0) {
            free(queue_head->key);
            free(queue_head);
        }
        else{
            free(queue_head->key);
            free(queue_head->data);
            free(queue_head);
        } 
        
        queue_head = next;
        nrRequests -= 1;

        if(nrRequests == 0) {
            queue_head = malloc(sizeof(struct request_t));
            queue_head->op_n = -1;
            queue_head->op = -1;
            queue_head->key = NULL;
            queue_head->data = NULL;
            queue_head->next_op = NULL;
        }
        
        pthread_mutex_unlock(&queueLock);
    }

    return 0;
}

/*Auxilia no fecho do servidor*/
void kill_threads() {
    terminate = 1;
    pthread_cond_broadcast(&cond);
    int* ret = 0;
    for(int i = 0; i < nrThreads; i++) {
        pthread_join(threads[i], (void**) ret);
    }

    free(opProc->in_progress);
    free(opProc);
    free(threads);

    free(queue_head);
    free(params);
}