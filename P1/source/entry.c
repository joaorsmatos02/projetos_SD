// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <stdlib.h>
#include <string.h>
#include "../include/entry.h"

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.
 */
struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t* this = (struct entry_t*) malloc(sizeof (struct entry_t));
    if (this == NULL) // verificar malloc
        return NULL;
    this->key = key;
    this->value = data;
    return this;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry) {
    if(entry == NULL)
        return;
    if(entry->key != NULL ) {
        free(entry->key);
        entry->key = NULL;
    }
    if(entry->value != NULL) {
        data_destroy(entry->value);
        entry->value = NULL;
    }
    free(entry);
    entry = NULL;
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
    if (entry == NULL || entry->key == NULL || entry->value == NULL)
        return NULL;
    char* new_key = (char*) malloc(strlen(entry->key) + 1);
    strcpy(new_key, entry->key);
    struct data_t* new_data = data_dup(entry->value);
    struct entry_t* new = entry_create(new_key, new_data);
    return new;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    if(entry != NULL && new_key != NULL && new_value != NULL) {
        if(entry->key != NULL)
            free(entry->key);
        entry->key = new_key;

        if(entry->value != NULL)
            data_destroy(entry->value);
        entry->value = new_value;
    }
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    if(strcmp(entry1->key,entry2->key) == 0)
        return 0; // keys iguais
    if(strcmp(entry1->key,entry2->key) < 0)
        return -1; // entry1 menor
    return 1; // entry1 maior
}