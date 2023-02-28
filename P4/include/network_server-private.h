// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#ifndef _NETWORK_SERVER_PRIVATE
#define _NETWORK_SERVER_PRIVATE

#include <poll.h>
#include <zookeeper/zookeeper.h>

/* Handler do SIGINT
*/
void ctrlC();

/* Remove um pollfd da lista de pollfds
*/
int removeFD(struct pollfd* fds, int position, int nrFds);

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context);

#endif