// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <stdlib.h>
#include <string.h>
#include "../include/data.h"

/* Função que cria um novo elemento de dados data_t, reservando a memória
 * necessária para armazenar os dados, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size) {
    if(size <= 0)
        return NULL;
    struct data_t* this = (struct data_t*) malloc(sizeof(struct data_t));
    if (this == NULL) // verificar malloc
        return NULL;
    this -> datasize = size;
    this -> data = malloc(size);
    if (this -> data == NULL)
        return NULL;
    return this;
}

/* Função que cria um novo elemento de dados data_t, inicializando o campo
 * data com o valor passado no parâmetro data, sem necessidade de reservar
 * memória para os dados.
 */
struct data_t *data_create2(int size, void *data) {
    if(size <= 0 || data == NULL)
        return NULL;
    struct data_t* this = (struct data_t*) malloc(sizeof(struct data_t));
    if(this == NULL)
        return NULL;
    this->datasize = size;
    this->data = data;
    return this;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data) {
    if(data == NULL)
        return;
    if(data -> data != NULL) {
        free(data->data);
        data->data = NULL;
    }
    free(data);
    data = NULL;
}

/* Função que duplica uma estrutura data_t, reservando toda a memória
 * necessária para a nova estrutura, inclusivamente dados.
 */
struct data_t *data_dup(struct data_t *data) {
    if (data == NULL || data->data == NULL || data->datasize <= 0)
        return NULL;
    struct data_t * new = data_create(data->datasize);
    if(new == NULL)
        return NULL;
    memcpy(new->data, data->data, data->datasize);
    return new;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data) {
    if (data != NULL && data->data != NULL && new_size >= 0 && new_data != NULL) {
        free(data->data);
        data->data = new_data;
        data->datasize = new_size;
    }
}