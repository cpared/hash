#include "hash.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

typedef enum {
DOS=2,
TRES=3,
CINCO=5,
SIETE=7,
ONCE=11,
TRECE=13,
DIECISIETE=17,
DIECINUEVE=19,
VEINTITRES=23,
VEINTINUEVE=29,
TRENTAIUNO=31,
TREINTISIETE37,
CUERENTAYUNO=41,
CUARENTAYTRES=43,
CUARENTAYNUEVE=47,
CINCUENTAYTRES=53,
} primos;


typedef enum {
	VACIO,
	OCUPADO,
	BORRADO,
}tipo_estado;

#define CAPACIDAD_INICIAL ONCE
#define POS_INICIAL 0

typedef struct campo{
    void* dato;
    int estado;
    void* clave;
}campo_t;


struct hash{
    void (*destruir_dato) (void*);
    campo_t** tabla;
    size_t capacidad;
    size_t cantidad;
    size_t contador;
    size_t borrados;
};


struct hash_iter{
    int pos;
    hash_t* hash;
};

// Función Hash djb2
int fhash(const char *str){ 
    int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// Función destruir dato
void destruir_dato(void* dato){
	free(dato);
}	




//--------------------------BUSQUEDA---------------------------------

int buscar_vacio(const hash_t *hash, const char *clave){
	
	int inicial=(int)fhash(clave);
	
	int pos;
	for (int i=0; i< (int)hash->capacidad; i++){
		pos= (inicial + i)% (int)hash->capacidad;
		if (hash->tabla[pos]->estado== VACIO) return pos;
	}
	return -1;	
}


int buscar_ocupado(const hash_t *hash, const char *clave){
	
	int inicial=fhash(clave);
	int pos;
	for (int i=0; i< (int)hash->capacidad; i++){
		pos= (inicial + i)% (int)hash->capacidad;
		if (hash->tabla[pos]->estado==OCUPADO && strcmp(hash->tabla[pos]->clave,clave)==0){
			return pos;
		} else if (hash->tabla[pos]->estado==VACIO){
			return -1;
		}	
	}
	return -1;
}


//------------------------HASH CERRADO---------------------------------


hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	
  hash_t* hash=malloc(sizeof(hash_t));
  if(!hash) return NULL;
  
  hash->tabla=malloc(sizeof(campo_t*)*CAPACIDAD_INICIAL);
  if(!hash->tabla){
    free(hash);
    return NULL;
  }
  
  for(int i=0;i<CAPACIDAD_INICIAL;i++){
    hash->tabla[i]=malloc(sizeof(campo_t));
	
	if(!hash->tabla[i]){
		free(hash->tabla);
		free(hash);
		return NULL;
    }
    hash->tabla[i]->clave=malloc(sizeof(char*));
    if(!hash->tabla[i]->clave){
		free(hash->tabla);
		free(hash);
		return NULL;
    }
    
    hash->tabla[i]->estado=VACIO;
  }
  
  hash->destruir_dato=destruir_dato;
  hash->capacidad=CAPACIDAD_INICIAL;
  hash->cantidad=0;
  hash->borrados=0;
  hash->contador=CAPACIDAD_INICIAL; //mantiene el enum pero no me esta saliendo. Te la regalo
  return hash;
}


void *hash_obtener(const hash_t *hash, const char *clave){
    int i = buscar_ocupado(hash,clave);
	if (i==-1) return NULL;
	return hash->tabla[i]->dato;
}


void hash_destruir(hash_t *hash){
    size_t i = 0;
    while (i < hash->capacidad){
        if(hash->destruir_dato){
            hash->destruir_dato(hash->tabla[i]->clave);
        }
        free(hash->tabla[i]->clave);
        free(hash->tabla[i]);
        i++;
    }
    free(hash->tabla);
    free(hash);
}


bool hash_pertenece(const hash_t *hash, const char *clave){	
	int inicial=fhash(clave);
	
	for (int i=0; i< (int)hash->capacidad; i++){
		int pos= (inicial + i)% (int)hash->capacidad;
		if (hash->tabla[pos]->estado==OCUPADO && strcmp(hash->tabla[pos]->clave,clave)==0){
			return true;
		}	
	}
	return false;
}


size_t hash_cantidad(const hash_t *hash){	
	return hash->cantidad;
}


//--------------------------ITERADOR---------------------------------


hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if(!iter) return NULL;
    
    size_t t=sizeof(hash);
    iter->hash=malloc(t);

    if(!iter->hash) return NULL;
	memcpy(iter->hash,hash,t);
	
    iter->pos = POS_INICIAL;
    return iter;
}


bool hash_iter_al_final(const hash_iter_t *iter){
    if(!iter->pos) return true;
    return iter->pos == iter->hash->capacidad;
}


bool hash_iter_avanzar(hash_iter_t *iter){
    if (hash_iter_al_final(iter)) return false;
    iter->pos++;
    return true;
}


const char *hash_iter_ver_actual(const hash_iter_t *iter){
	
	if(!iter->pos) return NULL;
	return iter->hash->tabla[(iter->pos)]->clave; 
}


void hash_iter_destruir(hash_iter_t* iter){
	free(iter->hash);
	free(iter);
}


//-------------------------REDIMENSION---------------------------------


void redimensionar(hash_t *hash){
	
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	
	if ( carga < 0.33 ){
		return;
	}
	hash_t *viejo= hash;
	hash_iter_t *iterador_viejo= hash_iter_crear(viejo);
	
	hash_t *nuevo=hash_crear(viejo->destruir_dato);
	
	while(!hash_iter_al_final(iterador_viejo)){
		
		const char *clave_vieja=hash_iter_ver_actual(iterador_viejo);
		char *clave_nueva=NULL;
		strcpy(clave_nueva,clave_vieja);
		hash_guardar(nuevo, clave_nueva, iterador_viejo->hash->tabla[(iterador_viejo->pos)]->dato);
		
		hash_iter_avanzar(iterador_viejo);
	}
	hash=nuevo;
	size_t nuevo_capacidad=viejo->contador + 1; // probelma del enum
	hash->capacidad=nuevo_capacidad;
	hash->contador++; //actualizar el contador de el enum
	
	hash_iter_destruir(iterador_viejo);
	free(viejo);

}



//-----------PRIMITIVAS DE HASH QUE DEPENDEN DE REDIMENSION-----------

void *hash_borrar(hash_t *hash, const char *clave){
	
	if(!hash_pertenece(hash,clave)) return NULL;
		
	int i=buscar_ocupado(hash,clave);
	if(i==-1) return NULL;

	void* dato;
	if (hash->destruir_dato!=NULL){
		dato=hash_obtener(hash,clave);
		hash->destruir_dato(hash->tabla[i]->dato);
	}
	dato=hash->tabla[i]->dato;
	hash->tabla[i]->estado= BORRADO;
	hash->tabla[i]->clave=NULL;
	
	//redimensionar(hash);
	hash->cantidad--;
	hash->borrados++;
	return dato;
}


bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	
	int i;
	if(!hash_pertenece(hash,clave)){
		i=buscar_vacio(hash,clave);
	
	} else {
		i=buscar_ocupado(hash,clave);
	}			
	if(i==-1) return false;
	
	strcpy(hash->tabla[i]->clave,clave);
	
	hash->tabla[i]->dato= dato;
	hash->tabla[i]->estado= OCUPADO;	
	hash->cantidad++;
	
	//redimensionar(hash);
	return true;
}




