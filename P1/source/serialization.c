// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <string.h>
#include <stdlib.h>
#include "../include/serialization.h"

/* Serializa todas as keys presentes na variável char **keys
 * para o buffer char *key_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através 
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf) {
    if(keys == NULL)
        return -1;
    
    int size = 0;

    for(int k = 0; keys[k] != NULL; k++) 
        size += strlen(keys[k]);

    *keys_buf = (char*) malloc(size + sizeof(NULL));

    if (keys_buf == NULL)
        return -1;
    
    int total = 0;
    for(int i = 0; keys[i] != NULL; i++) {
        strcpy(*keys_buf + total, *(keys + i)); // copiar cada key de keys para key_buff
        total += strlen(keys[i]);
    }
    
    return size + sizeof(NULL);
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char** buffer_to_keyArray(char *keys_buf, int keys_buf_size) {
    char** resultado = (char**) malloc((strlen(keys_buf) / keys_buf_size) * sizeof(char*));
    if (resultado == NULL) // verificar malloc
        return NULL;
    for(int i = 0; i < strlen(keys_buf) / keys_buf_size; i++) {
        resultado[i] = malloc(keys_buf_size + 1);
        memcpy(resultado[i], keys_buf + i * keys_buf_size, keys_buf_size);
    }
    return resultado;
}
