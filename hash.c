#include "hash.h"
#include <stddef.h>

#define CAPACIDAD_INICIAL 11
#define POS_INICIAL 0

typedef struct campo{
    void* dato;
    int estado;
    void* clave;
}campo_t;

struct hash{
    void (*destruir_dato) (void*);
    void** tabla;
    size_t capacidad;
    size_t cantidad;
};

struct hash_iter{
    int pos;
    hash_t* hash;
};

// Función Hash djb2
int fhash(unsigned char *str){ 
    int hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    int pos = fhash(clave);
    while(hash->tabla[pos]->estado != 0){
        if (strcmp(hash->tabla[pos]->clave, clave) == 0) return hash->tabla[pos]->dato;
        pos++;
    }
    return NULL;
}

void hash_destruir(hash_t *hash){
    size_t i = 0;
    while (i < hash->capacidad){
        if(hash->destruir_dato){
            hash->destruir_dato(hash->tabla[i]->clave);
        }
        free(hash->tabla[i]);
        i++;
    }
    free(hash->tabla);
    free(hash);
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    return hash_obtener(hash,clave) == NULL;
}


hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if(!iter) return NULL;
    iter->hash = hash;
    iter->pos = POS_INICIAL;
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    iter->pos++;
    return true;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->pos == iter->hash->capacidad;
}


//-------------------------EUGE---------------------------------

hash_t *hash_crear(hash_destruir_dato_t destruir_dato);
void *hash_borrar(hash_t *hash, const char *clave);
size_t hash_cantidad(const hash_t *hash);
bool hash_guardar(hash_t *hash, const char *clave, void *dato);




void hash_iter_destruir(hash_iter_t* iter);
const char *hash_iter_ver_actual(const hash_iter_t *iter);