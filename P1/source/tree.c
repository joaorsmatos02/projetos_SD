// Grupo 20
// Tomás Barreto nº 56282
// João Matos nº 56292
// Diogo Pereira nº 56302

#include <stdlib.h>
#include "../include/tree.h"
#include "../include/tree-private.h"
#include "../include/entry.h"
#include <string.h>

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create() {
    struct tree_t* result = malloc(sizeof(struct tree_t));
    if(result == NULL)
        return NULL;
    result->entry = NULL;
    result->left = NULL;
    result->right = NULL;
    return result;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) {
    if(tree == NULL)
        return;
    if(tree->right != NULL && tree->right->entry != NULL && tree->right->entry->key != NULL) {
        tree_destroy(tree->right);
        tree->right = NULL;
    }
    if(tree->left != NULL && tree->left->entry != NULL && tree->left->entry->key != NULL) {
        tree_destroy(tree->left);
        tree->left = NULL;
    }
    if(tree->entry != NULL && tree->entry->key != NULL) {
        entry_destroy(tree->entry);
        tree->entry = NULL;
    }
    free(tree);
    tree = NULL;
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value) {
    if(tree == NULL || key == NULL || value == NULL)
        return -1;
    
    char* new_key = strdup(key);
    struct data_t* new_value = data_dup(value);
    struct entry_t* entry = entry_create(new_key, new_value);
    
    int done = 0;

    while(!done) {
        if(tree->entry == NULL || tree->entry->key == NULL) {
            tree->entry = entry;
            done = 1;
        } else { 
            switch (entry_compare(entry, tree->entry)) {
                case 0:
                    entry_destroy(tree->entry);
                    tree->entry = entry;
                    done = 1;
                    break;
                case -1:
                    if(tree->left == NULL || tree->left->entry == NULL || tree->left->entry->key == NULL)
                        tree->left = tree_create();
                    tree = tree->left;
                    break;
                default:
                    if(tree->right == NULL || tree->right->entry == NULL || tree->right->entry->key == NULL)
                        tree->right = tree_create();
                    tree = tree->right;
            }
        }
    }
    return 0;
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key) {
    if(tree == NULL || tree->entry == NULL || tree-> entry->key == NULL || key == NULL)
        return NULL;
    int cmp = strcmp(key, tree->entry->key);
    if(cmp == 0) {
        struct data_t* result = data_dup(tree->entry->value);
        return result;
    }
    else if (cmp < 0)
        return tree_get(tree->left, key);
    else
        return tree_get(tree->right, key);
    return NULL;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key) {
    if (tree == NULL || key == NULL)
        return -1;

    int cmp = strcmp(key, tree->entry->key);

    if (cmp < 0)
        return tree_del(tree->left, key);
    else if (cmp > 0)
        return tree_del(tree->right, key);
    else { // tree e o no a apagar
        if ((tree->left == NULL || tree->left->entry == NULL || tree->left->entry->key == NULL) && 
        (tree->right == NULL || tree->right->entry == NULL || tree->right->entry->key == NULL)) { // se for terminal
            tree_destroy(tree);
            tree->entry = NULL;
            return 0;
        } else {
            entry_destroy(tree->entry);
            if (tree->left == NULL) { // se nao tiver a esquerda
                // ir buscar no mais pequeno a direita para substituir
                struct tree_t* temp = smallest_key(tree->right);
                tree->entry = entry_dup(temp->entry); // copiar conteudo do no
                return tree_del(tree->right, temp->entry->key); // eliminar no
            } else {
                struct tree_t* temp = biggest_key(tree->left);
                tree->entry = entry_dup(temp->entry); // copiar conteudo do no
                return tree_del(tree->left, temp->entry->key); // eliminar no
            }
        }
    }
}

/* Encontra e retorna o no com a chave mais pequena na tree (no mais a esquerda)
*/
struct tree_t* smallest_key(struct tree_t* tree) {
    if(tree == NULL)
        return NULL;
    while (tree->left != NULL)
        tree = tree->left;
    return tree;
}

/* Encontra e retorna o no com a chave mais alta na tree (no mais a direita)
*/
struct tree_t* biggest_key(struct tree_t* tree) {
    if(tree == NULL)
        return NULL;
    while (tree->right != NULL)
        tree = tree->right;
    return tree;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree) {
    if(tree == NULL || tree->entry == NULL || tree->entry->key == NULL)
        return 0;
    return 1 + tree_size(tree->right) + tree_size(tree->left);
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree) {
    if(tree == NULL || tree->entry == NULL || tree->entry->key == NULL)
        return 0;
    int left_side = tree_height(tree->left);
    int right_side = tree_height(tree->right);
    int max = 0;
    if(left_side >= right_side)
        max = left_side;
    else
        max = right_side;
    return max + 1;        
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree) { //NOT SURE PROBABLY WRONG
    if(tree == NULL || tree->entry == NULL)
        return NULL;

    int size = tree_size(tree);
    char** result = (char**) malloc(size * sizeof(char*) + sizeof(NULL));
    result[size] = NULL;
    int pos = 0;

    tree_iterator_keys(result, tree, &pos);
    
    return result;
}

/* Itera recursivamente sobre tree, copiando as keys para arr
*/
void tree_iterator_keys(char** arr, struct tree_t *tree, int* pos) {
    if(tree->left != NULL)
        tree_iterator_keys(arr, tree->left, pos);
    arr[*pos] = malloc(strlen(tree->entry->key) + 1);
    strcpy(arr[*pos], tree->entry->key);
    (*pos)++;
    if(tree->right != NULL)
        tree_iterator_keys(arr, tree->right, pos);
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void **tree_get_values(struct tree_t *tree) {
    if(tree == NULL || tree->entry == NULL)
        return NULL;

    int size = tree_size(tree);
    void** result = (void**) malloc(size * sizeof(void*) + sizeof(NULL));
    result[size] = NULL;
    int pos = 0;

    tree_iterator_values(result, tree, &pos);
    
    return result;
}

/* Itera recursivamente sobre tree, copiando os values para arr
*/
void tree_iterator_values(void** arr, struct tree_t *tree, int* pos) {
    if(tree->left != NULL)
        tree_iterator_values(arr, tree->left, pos);
    arr[*pos] = malloc(tree->entry->value->datasize);
    memcpy(arr[*pos], tree->entry->value->data, tree->entry->value->datasize);
    (*pos)++;
    if(tree->right != NULL)
        tree_iterator_values(arr, tree->right, pos);
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys) {
    int pos = 0;
    while(keys[pos] != NULL) {
        free(keys[pos]);
        pos++;
    }
    free(keys[pos]);
    free(keys);
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */
void tree_free_values(void **values) {
    int pos = 0;
    while(values[pos] != NULL) {
        free(values[pos]);
        pos++;
    }
    free(values[pos]);
    free(values);
}