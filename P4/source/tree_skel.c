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
#include "../include/sdmessage.pb-c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zookeeper/zookeeper.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include "../include/read_write-private.h"
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>

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

    
static int is_connected;
static zhandle_t* zh;
static char* node_id;
static char* next_server;
struct rtreez_t* rtreez;
static char* watcher_ctx = "Zookeeper Data Watcher";
struct String_vector* children_list;
char* zoo_ip_port_1;
const char* zoo_root;

struct rtreez_t* next_in_chain;

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

    threads = malloc(nrThreads * sizeof(pthread_t));

    last_assigned = 1;

    opProc = malloc(sizeof(struct op_proc));
    opProc->max_proc = 0;
    opProc->in_progress = calloc(nrThreads, sizeof(int));

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

    for(int i = 0; i < nrThreads; i++) {
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

        struct _MessageT msg;
        //Processamento DELETE
        if(queue_head->op == 0) {
            pthread_mutex_lock(&treeLock);
            tree_del(tree, queue_head->key);

            message_t__init(&msg);
            msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
            msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;
            msg.key = queue_head->key;


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

            message_t__init(&msg);

            struct _EntryT entry_copy;
            entry_t__init(&entry_copy);

            struct _DataT data_copy;
            data_t__init(&data_copy);

            entry_copy.key = queue_head->key;
            data_copy.data = queue_head->data;
            data_copy.datasize = strlen(queue_head->data) + 1;
            entry_copy.value = &data_copy;
            msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
            msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
            msg.entry = &entry_copy;

            free(new_data3);
            pthread_mutex_unlock(&treeLock);
        }

        pthread_mutex_lock(&opProcLock);
        if(queue_head->op_n > opProc->max_proc)
            opProc->max_proc = queue_head->op_n;
        
        opProc->in_progress[*((int*) params)] = -1;
        pthread_mutex_unlock(&opProcLock);

        // Enviar para o prox server
        
        if(next_in_chain != NULL)
            if(zk_send(next_in_chain, &msg) != 0)
                printf("Erro a repassar a mensagem");

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


struct rtreez_t* connect_next_zk_server(char* address_port) {
    struct rtreez_t* connection = malloc(sizeof(struct rtreez_t));
    
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

    if(network_connect_zk(connection) == 0)
        return connection;
        
    free(connection);

    return NULL;
}

int network_connect_zk(struct rtreez_t *rtreez) {
    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server para estabelecer conexao
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(rtreez->port));
    if (inet_pton(AF_INET, rtreez->ip_addr, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(sockfd);
        return -1;
    }

    // Estabelece conexao com o servidor definido em server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sockfd);
        return -1;
    }

    rtreez->sockfd = sockfd;
    
    return 0;
}

int zk_send(struct rtreez_t * rtreez, struct _MessageT *msg) {
    size_t length = message_t__get_packed_size((const struct _MessageT*) msg);
    uint8_t* buffer = malloc(length*sizeof(uint8_t));

    if(buffer == NULL)
        return -1;
    
    size_t packedLength = message_t__pack(msg, buffer);

    int nbytes;
    if((nbytes = write_all(rtreez->sockfd, buffer, packedLength)) != packedLength){
        perror("Erro ao enviar dados ao servidor");
        close(rtreez->sockfd);
        return -1;
    }
    free(buffer);

    return 0;
}

int zk_connect(char* server_port, char* zoo_ip_port) {
    // ##############################################################################################################
    /* Connect to ZooKeeper server */
    zoo_ip_port_1 = zoo_ip_port;                                                         //zookeeper ip and port
	zoo_root = "/chain";                                                                             //zookeeper root ZNode (/chain)
	children_list = (struct String_vector *) malloc(sizeof(struct String_vector));    //list of root childs
    

	zh = zookeeper_init(zoo_ip_port, connection_watcher, 2000, 0, NULL, 0); 
	if (zh == NULL)	{
	    return -1;
	}

    struct ifaddrs *addrs,*tmp;

    getifaddrs(&addrs);
    tmp = addrs;
    char ip_address[15];
    int fd;
    struct ifreq ifr;

    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET)
            
            if(strcmp(tmp->ifa_name, "lo") != 0) {
                fd = socket(AF_INET, SOCK_DGRAM, 0);
                ifr.ifr_addr.sa_family = AF_INET;
                memcpy(ifr.ifr_name, tmp->ifa_name, IFNAMSIZ - 1);
                ioctl(fd, SIOCGIFADDR, &ifr);
                close(fd);
                strcpy(ip_address, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
                printf("System IP Address is: %s\n", ip_address);
                break;
            }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);

    char* ip_port = calloc(strlen(ip_address) + 5, sizeof(char));
    strcat(ip_port, ip_address);
    strcat(ip_port , ":");
    strcat(ip_port , server_port);

    int new_path_len1 = 1024;
    char* new_path1 = malloc(new_path_len1);

    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL)) {
        if (ZOK != zoo_create(zh, "/chain", ip_port, strlen(ip_port) + 1, &(ZOO_OPEN_ACL_UNSAFE), 0, new_path1, new_path_len1)) {
            fprintf(stderr, "Error creating znode from path %s!\n", new_path1);
            return -1;
        }
    }
    else {
        new_path1 = "/chain";
    }
    // ##############################################################################################################
    
    // ##############################################################################################################
    // Create an ephemeral and sequencial ZNode, child of /chain
    char* node_path = calloc(120, sizeof(char));
    strcat(node_path, new_path1);
    strcat(node_path, "/node");
    
    int new_path_len = 1024;
    char* new_path = malloc(new_path_len);

    if (ZOK != zoo_create(zh, node_path, ip_port, strlen(ip_port) + 1, &(ZOO_OPEN_ACL_UNSAFE), ZOO_EPHEMERAL | ZOO_SEQUENCE, new_path, new_path_len)) {
        fprintf(stderr, "Error creating znode from path %s!\n", node_path);
        return -1;
    }
    node_id = strdup(new_path);
    fprintf(stderr, "Ephemeral Sequencial ZNode created! ZNode path: %s\n", new_path);
    // ##############################################################################################################

	// ##############################################################################################################
	
    if(connect_next_server(zoo_root, zoo_ip_port) != 0)
        return -1;

    return 0;
}


static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    struct String_vector* children_list =	(struct String_vector*) malloc(sizeof(struct String_vector));
    if (state == ZOO_CONNECTED_STATE)	 {
        if (type == ZOO_CHILD_EVENT) {
            if(connect_next_server(zoo_root, zoo_ip_port_1) != 0)
                return;
        }
        free(children_list);
    }
}

int connect_next_server(const char* zoo_root, char* zoo_ip_port) {
    /* Get the list of children synchronously */
    int retval, retval1 = -1;
    
	retval = zoo_wget_children(zh, zoo_root, child_watcher, watcher_ctx, children_list);                  // 1 para fazer watch aos filhos
    
	if (retval != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", zoo_root);
	    return -1;
	}
	fprintf(stderr, "\n=== znode listing === [ %s ]", zoo_root); 
	for (int i = 0; i < children_list->count; i++)  {
		fprintf(stderr, "\n(%d): %s", i+1, children_list->data[i]); 
	}
	fprintf(stderr, "\n=== done ===\n");
    // ##############################################################################################################
    
    // ##############################################################################################################
    char* max_server_path = strdup(node_id);

    // Gets the max server id superior to the current server id
    for (int i = 0; i < children_list->count; i++)  {
        
        char* path = strdup("/chain/");
        strcat(path, children_list->data[i]);

		if(strcmp(path, max_server_path) > 0) {
            max_server_path = calloc(strlen(children_list->data[i]) + 1, sizeof(char));
            strcat(max_server_path , "/chain/");
            strcat(max_server_path, children_list->data[i]);
            break;
        }
        free(path);
	}


    for (int i = 0; i < children_list->count; i++)
        free(children_list->data[i]);

    char* buffer = malloc(25);
    int buffer_len = 25;

    retval1 = zoo_get(zh, max_server_path, 1, buffer, &buffer_len, NULL);

    if (retval1 != ZOK)	{
		fprintf(stderr, "Error retrieving znode from path %s!\n", max_server_path);
	    return -1;
	}
    // ##############################################################################################################

    // ##############################################################################################################
    // ver se o servidor atual é o ultimo

    if(strcmp(node_id, max_server_path) == 0) {
        next_server = NULL;
    }
    else {
        next_server = malloc(buffer_len);
        memcpy(next_server, buffer, buffer_len);

        next_in_chain = malloc(sizeof(struct rtreez_t));
        next_in_chain = connect_next_zk_server(buffer);
    }

    return 0;
}