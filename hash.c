#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#define CAPACIDAD_INICIAL 11
#define BITE_ESTADO 1
#define POS_INICIAL 0
#define BORRADOS_INICIAL 0
#define VALOR_AGRANDAR 0.7
#define VALOR_REDUCIR 0.3
#define CANT_CAMPOS 3
/* ******************************************************************
 *                        STRUCT HASH
 * *****************************************************************/
typedef enum {
	REDUCIR = 1,
	AGRANDAR = 0,
}criterio_t;

typedef enum {
	VACIO,
	OCUPADO,
	BORRADO,
}tipo_estado;

typedef struct campo{
    void* dato;
    int estado;
    char* clave;
}campo_t;

struct hash{
    void (*destruir_dato)(void*);
    campo_t** tabla;
    size_t capacidad;
    size_t cantidad;
    size_t borrados;
};

struct hash_iter{
    size_t cant;
    size_t pos;
    const hash_t* hash;
};

/* ******************************************************************
 *                        FUNCION HASH
 * *****************************************************************/

// Función Hash djb2
int fhash(const char *str){ 
    int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/* ******************************************************************
 *                        PRIMITIVAS HASH
 * *****************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	
    hash_t* hash=calloc(1,sizeof(hash_t));
    if(!hash) return NULL;
    
    hash->tabla=calloc(CAPACIDAD_INICIAL,sizeof(campo_t));
    if(!hash->tabla){
        free(hash);
        return NULL;
    }

    //Recorro la tabla y creo los campos inicialmente en estado VACIO
    for (size_t i = 0; i < CAPACIDAD_INICIAL; i++){
        campo_t* campo = malloc(sizeof(campo_t));
        if(!campo){
            free(hash->tabla);
            free(hash);
            return NULL;
        } 
        campo->estado = VACIO;
        campo->clave = NULL;
        campo->dato = NULL;
        hash->tabla[i] = campo;
    }

    hash->destruir_dato = destruir_dato;
    hash->capacidad = CAPACIDAD_INICIAL; 
    hash->borrados = BORRADOS_INICIAL;
    return hash;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    if(hash->cantidad == 0) return NULL;
    
    size_t pos = (size_t) fhash(clave) % hash->capacidad;
    while(hash->tabla[pos]->estado != VACIO){
        if (hash->tabla[pos]->estado == OCUPADO && strcmp(hash->tabla[pos]->clave, clave) == 0){
            return hash->tabla[pos]->dato;
        }
        pos = (pos + 1) % hash->capacidad;
    }
    return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){

    size_t pos = (size_t) fhash(clave) % hash->capacidad;
    while(hash->tabla[pos]->estado != VACIO){
        if (hash->tabla[pos]->estado == OCUPADO && strcmp(hash->tabla[pos]->clave, clave) == 0){
            return true;
        } 
        pos = (pos + 1) % hash->capacidad;
    }
    return false;
}

size_t hash_cantidad(const hash_t *hash){	
	return hash->cantidad;
}

void hash_destruir(hash_t *hash){
    size_t i = 0;
    while (i < hash->capacidad){
        if(hash->destruir_dato){
            hash->destruir_dato(hash->tabla[i]->dato);
        }
        free(hash->tabla[i]->clave);
        free(hash->tabla[i]);
        i++;
    }
    free(hash->tabla);
    free(hash);
}


size_t buscar_vacio(const hash_t *hash, const char *clave){
	
	size_t inicial = (size_t)fhash(clave) % hash->capacidad;
	size_t pos = inicial;
    if (hash->cantidad != 0){
        for (size_t i = 1; i < hash->capacidad && hash->tabla[pos]->estado != VACIO; i++){
            pos = (inicial + i) % hash->capacidad;
        }
    }
	return pos;
}

bool redimensionar(hash_t *hash,int criterio){

    size_t capacidad_anterior = hash->capacidad;
    if(criterio == AGRANDAR) hash->capacidad = (hash->capacidad * 2) + 1;
    if(criterio == REDUCIR) hash->capacidad = hash->capacidad / 2;
	
    campo_t** tabla_nueva = calloc(hash->capacidad,sizeof(campo_t));
    if(!tabla_nueva) return NULL;
    campo_t** tabla_vieja = hash->tabla;
    hash->tabla = tabla_nueva;

    //Recorro la tabla y creo los campos inicialmente en estado VACIO
    for (size_t i = 0; i < hash->capacidad; i++){
        campo_t* campo = malloc(sizeof(campo_t));
        if(!campo){
            free(hash->tabla);
            return NULL;
        } 
        campo->estado = VACIO;
        campo->clave = NULL;
        campo->dato = NULL;
        hash->tabla[i] = campo;
    }

    size_t pos;
    for(size_t i = 0; i <capacidad_anterior; i++){ //recorro tabla vieja
        if(tabla_vieja[i]->estado == OCUPADO){ //agrego en tabla nueva en espacio vacio
            pos = (size_t) fhash(tabla_vieja[i]->clave) % hash->capacidad;
            while(hash->tabla[pos]->estado != VACIO){
                pos++;
            }
            free(hash->tabla[pos]);
            hash->tabla[pos] = tabla_vieja[i];
            continue;
        }
        free(tabla_vieja[i]);
    }
    free(tabla_vieja);
    hash->borrados = BORRADOS_INICIAL;
    return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
	
	if(!hash_pertenece(hash,clave)) return NULL;

    size_t pos = (size_t)fhash(clave) % hash->capacidad;
	void* dato = hash_obtener(hash,clave);

	if (hash->destruir_dato){
		hash->destruir_dato(hash->tabla[pos]->dato);
	}
    free(hash->tabla[pos]->clave);
	hash->tabla[pos]->estado = BORRADO;
	hash->tabla[pos]->clave = NULL;
	hash->cantidad--;
	hash->borrados++;
	
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	if (carga <= VALOR_REDUCIR && hash->capacidad != CAPACIDAD_INICIAL)	redimensionar(hash,REDUCIR);

	return dato;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    size_t pos = (size_t) fhash(clave) % hash->capacidad;
    //veo si la clave ya esta guardada, si es así, la reemplazo
    if (hash_pertenece(hash,clave)){
        if(hash->destruir_dato) free(hash->tabla[pos]->dato);
        hash->tabla[pos]->dato = dato;
		hash->tabla[pos]->estado= OCUPADO;
        return true;
    }

    // Veo si tengo que redimensionar la tabla
	float carga= (float)(hash->cantidad + hash->borrados)/ (float) hash->capacidad;
	if (carga >= VALOR_AGRANDAR) redimensionar(hash,AGRANDAR);

    pos = buscar_vacio(hash,clave); //si hay colición, busco pos vacía

    //Reservo memoria para la clave, guardo la clave
    char* copia_clave = malloc(sizeof(char) + strlen(clave));
    if(!copia_clave) return NULL;
    strcpy(copia_clave,clave);
    hash->tabla[pos]->clave = copia_clave;
    

    hash->tabla[pos]->dato = dato;
    hash->tabla[pos]->estado= OCUPADO;
    hash->cantidad ++;
    return true;
}

/* ******************************************************************
 *                        ITERADOR HASH
 * *****************************************************************/

size_t buscar_primero(const hash_t* hash){
	int pos=-1;
	for (size_t i=0; i< hash->capacidad; i++){
		if (hash->tabla[i]->estado == OCUPADO){
			pos=(int)i;
			break;
		}
	}
	if (pos!=-1){
		return (size_t)pos;
	}
    
    return hash->capacidad;
}

hash_iter_t *hash_iter_crear(const hash_t *hash){
    
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if(!iter) return NULL;

	iter->hash = hash;
    iter->cant = 0;
    iter->pos = buscar_primero(hash);
    return iter;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if(hash_iter_al_final(iter)) return NULL;
	if(iter->hash->tabla[(iter->pos)]->estado == OCUPADO) return iter->hash->tabla[(iter->pos)]->clave; 
	return NULL;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}

bool hash_iter_avanzar(hash_iter_t *iter){
	
	if (hash_iter_al_final(iter) || !iter) return false;
    
	iter->cant ++;
	for(size_t i=iter->pos +1; i < iter->hash->capacidad;i++){
		if (iter->hash->tabla[i]->estado==OCUPADO){
			iter->pos=i;
			return true;
		}
	}
	return false;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->cant == iter->hash->cantidad;
}
