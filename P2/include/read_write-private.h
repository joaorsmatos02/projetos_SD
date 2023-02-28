// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#ifndef _READ_WRITE_PRIVATE_H
#define _READ_WRITE_PRIVATE_H

#include <arpa/inet.h>

// Lê todos os bytes existentes no socket(sock)
int read_all(int sock, const uint8_t* buf, int len);

// Escreve len bytes de buf no socket(sock)
int write_all(int sock, const uint8_t* buf, int len);

#endif