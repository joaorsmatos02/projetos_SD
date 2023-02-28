// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <zookeeper/zookeeper.h>

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

struct rtree_t {
    char* ip_addr;
    char* port;
    int sockfd;
};

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

void zk_connect(char* address_port);

void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

#endif