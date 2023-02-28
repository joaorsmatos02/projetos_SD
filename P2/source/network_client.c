// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include "../include/network_client.h"
#include "../include/client_stub-private.h"
#include "../include/read_write-private.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../include/sdmessage.pb-c.h"

#define MAX_MSG 2048

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree){
    int sockfd;
    struct sockaddr_in server;

    // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    // Preenche estrutura server para estabelecer conexao
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(rtree->port));
    if (inet_pton(AF_INET, rtree->ip_addr, &server.sin_addr) < 1) {
        printf("Erro ao converter IP\n");
        close(sockfd);
        return -1;
    }

    // Estabelece conexao com o servidor definido em server
    if (connect(sockfd,(struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(sockfd);
        return -1;
    }

    rtree->sockfd = sockfd;
    
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct _MessageT *network_send_receive(struct rtree_t * rtree, struct _MessageT *msg){
    size_t length = message_t__get_packed_size((const struct _MessageT*) msg);
    uint8_t* buffer = malloc(length*sizeof(uint8_t));

    if(buffer == NULL)
        return NULL;
    
    size_t packedLength = message_t__pack(msg, buffer);

    int nbytes;
    if((nbytes = write_all(rtree->sockfd, buffer, packedLength)) != packedLength){
        perror("Erro ao enviar dados ao servidor");
        close(rtree->sockfd);
        return NULL;
    }
    free(buffer);

    const uint8_t buf[MAX_MSG];

    if((nbytes = read_all(rtree->sockfd, buf, MAX_MSG)) <= 0){
        perror("Erro ao receber dados do servidor");
        close(rtree->sockfd);
        return NULL;
    }

    msg = message_t__unpack(NULL, nbytes, buf);

    return msg;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
    return close(rtree->sockfd);
}