// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <stdlib.h>
#include <stdio.h>
#include "../include/tree_skel.h"
#include "../include/network_server.h"
#include <signal.h>

int main(int argc, char* argv[]) {

    signal(SIGPIPE, SIG_IGN);

    int socketfd = 0;

    if((socketfd = network_server_init(atoi(argv[1]))) == -1)
        return -1;

    if(tree_skel_init() == -1)
        return -1;

    if(network_main_loop(socketfd) == -1)
        return -1;

    return 0;
}