#include <zookeeper/zookeeper.h>

struct op_proc {
    int max_proc;       //ultima operacao processada
    int* in_progress;   //operacoes em processamento
};

struct request_t {
    int op_n;//o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    struct request_t* next_op;
};

struct thread_nr {
    int number;
};

/*Auxilia no fecho do servidor*/
void kill_threads();

struct rtreez_t {
    char* ip_addr;
    char* port;
    int sockfd;
};

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

struct rtreez_t* connect_next_zk_server(char* address_port);

int network_connect_zk(struct rtreez_t *rtreez);

int zk_send(struct rtreez_t * rtreez, struct _MessageT *msg);

int zk_connect(char* server_port, char* zoo_ip_port);

int connect_next_server(const char* zoo_root, char* zoo_ip_port);