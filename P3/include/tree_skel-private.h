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