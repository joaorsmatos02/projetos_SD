// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include "../include/network_server.h"
#include "../include/read_write-private.h"
#include "../include/network_server-private.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_MSG 2048

int socketfd = 0;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port){

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar a socket");
        return -1;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

     // Preenche estrutura server para bind
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Faz bind
    if(bind(socketfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        network_server_close();
        return -1;
    }

    // Faz listen
    if(listen(socketfd, 0) < 0){
        perror("Erro ao executar listen");
        network_server_close();
        return -1;
    }

    return socketfd;
}

/* Handler do SIGINT
*/
void ctrlC() {
    tree_skel_destroy();
    network_server_close();
    exit(1);
} 

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket){

    struct sockaddr client = { 0 };
    socklen_t size_client = 0;

    struct sigaction sa;
    sa.sa_handler = ctrlC;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask); 

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("main:");
        exit(0);
    }

    while(1) {

        if((listening_socket = accept(socketfd, (struct sockaddr*) &client, &size_client)) != -1) {
            printf("Client Connected\n");
            struct _MessageT* msg;

            while((msg = network_receive(listening_socket)) != NULL) {

                if(msg == NULL || invoke(msg) == -1)
                    return -1;

                if(network_send(listening_socket, msg) == -1)
                    return -1;
            }
 
        }
    }

    return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura _MessageT.
 */
struct _MessageT *  network_receive(int client_socket){
    int nbytes = 0;
    uint8_t buf[sizeof(uint8_t)*MAX_MSG];
    memset(buf, 0, sizeof(uint8_t)*MAX_MSG);
    
    // Lê string enviada pelo cliente do socket referente a conexão
    if((nbytes = read_all(client_socket,buf,MAX_MSG)) <= 0){
        perror("Client Disconnected");
        close(client_socket);
        return NULL;
    }

    struct _MessageT* message;
    message = message_t__unpack(NULL, nbytes, buf);

    return message;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct _MessageT *msg){
    size_t length = message_t__get_packed_size(msg);
    uint8_t* buffer = malloc(length);

    if(buffer == NULL)
        return -1;

    size_t packedLength = message_t__pack(msg, buffer);

    int nbytes;
    if((nbytes = write_all(client_socket, buffer, packedLength)) != packedLength){
        perror("Erro ao enviar dados ao servidor");
        close(client_socket);
        return -1;
    }

    message_t__free_unpacked(msg, NULL);
    free(buffer);

    return 0;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    return close(socketfd);
}
