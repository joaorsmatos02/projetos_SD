// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"
#include "entry.h"

struct tree_t {
	struct entry_t* entry;
	struct tree_t *right, *left;
};

/* Itera recursivamente sobre tree, copiando as keys para arr
*/
void tree_iterator_keys(char** arr, struct tree_t *tree, int* pos);

/* Itera recursivamente sobre tree, copiando os values para arr
*/
void tree_iterator_values(void** arr, struct tree_t *tree, int* pos);

/* Encontra e retorna o no com a chave mais pequena na tree (no mais a esquerda)
*/
struct tree_t* smallest_key(struct tree_t* tree);

/* Encontra e retorna o no com a chave mais alta na tree (no mais a direita)
*/
struct tree_t* biggest_key(struct tree_t* tree);

#endif
