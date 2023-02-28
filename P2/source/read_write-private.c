// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <errno.h>
#include "../include/read_write-private.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

int read_all(int sock, const uint8_t* buf, int len) {
    int bufsize = 0;
    void* buffer = (void*) buf;

    while(len > 0) {
        int res = read(sock, buffer, len);
        bufsize += res;
        ioctl(sock, FIONREAD, &len);
        if(res <= 0) {
            if(errno==EINTR)
                continue;
            return res;
        }
        buffer += res;
    }

    return bufsize;
}

int write_all(int sock, const uint8_t* buf, int len) {
    int bufsize = len;
    void* buffer = (void*) buf;

    while(len > 0) {
        int res = write(sock, buffer, len);
        if(res <= 0) {
            if(errno==EINTR)
                continue;
            perror("write failed");
            return res;
        }
        buffer += res;
        len -= res;
    }

    return bufsize;
}
