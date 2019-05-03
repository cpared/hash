#include "hash.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>




typedef enum {
	VACIO,
	OCUPADO,
	BORRADO,
}tipo_estado;

typedef enum {
	ACHICAR=1,
	AGRANDAR,
}criterio_t;

#define CAPACIDAD_INICIAL 11
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
    size_t pos;
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

size_t buscar_vacio(const hash_t *hash, const char *clave){
	
	size_t inicial=(size_t)fhash(clave);
	
	size_t pos;
	for (size_t i=0; i< hash->capacidad; i++){
		pos= (inicial + i)% hash->capacidad;
		if (!hash->tabla[pos]) return pos;
	}
	return 0;
}


size_t buscar_ocupado(const hash_t *hash, const char *clave){
	
	size_t inicial=(size_t)fhash(clave);
	size_t pos;
	for (size_t i=0; i< hash->capacidad; i++){
		pos= (inicial + i)% hash->capacidad;
		if (!hash->tabla[pos]) continue;
		
		if (hash->tabla[pos]->estado==OCUPADO && strcmp(hash->tabla[pos]->clave,clave)==0){
			return pos;
		} else if (hash->tabla[pos]->estado==VACIO){
			return 0;
		}	
	}
	return 0;
}


//------------------------HASH CERRADO---------------------------------


hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	
  hash_t* hash=calloc(1,sizeof(hash_t));
  if(!hash) return NULL;
  
  
  
  hash->tabla=calloc(CAPACIDAD_INICIAL,sizeof(campo_t*));
  if(!hash->tabla){
    free(hash);
    return NULL;
  }

  hash->destruir_dato=destruir_dato;
  hash->capacidad=CAPACIDAD_INICIAL; 
  return hash;
}


void *hash_obtener(const hash_t *hash, const char *clave){
    size_t i = buscar_ocupado(hash,clave);
    if (i==0 && !hash->tabla[i]) return NULL;
	//if (i==0 && hash->tabla[i]->estado!=OCUPADO && strcmp(hash->tabla[i]->clave,clave)!=0) return NULL;
	return hash->tabla[i]->dato;
}


void hash_destruir(hash_t *hash){
    size_t i = 0;
    while (i < hash->capacidad){
        if (!hash->tabla[i]){
			 i++;
			 continue;
		}	 
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


bool hash_pertenece(const hash_t *hash, const char *clave){	
	size_t inicial= (size_t) fhash(clave);
	
	for (size_t i=0; i<hash->capacidad; i++){
		size_t pos= (inicial + i)%hash->capacidad;
		if (!hash->tabla[pos]) continue;
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

void primos(hash_t *hash){
	size_t primos_pequenios[12]={2,3,5,7,11,13,17,19,23,29,31,37};
	size_t proximo;
	size_t j=  hash->capacidad;
	for (size_t i=0; i<12; i++){
		if (primos_pequenios[i]==j){
			proximo=primos_pequenios[i];
			break;
		}	
    }
	if (proximo) hash->capacidad= (size_t)proximo;

	hash->capacidad= hash->contador*hash->contador +hash->contador +41;
	hash->contador++;

}

size_t factorial(size_t n){
	size_t c,fact;
	fact=1;
	for (c = 1; c <= n; c++){
		fact = fact * c;
	}
	return fact;		
}

size_t calcular_primo(hash_t* hash, int criterio){
	size_t prox,ant;
	size_t actual=hash->capacidad;
	
	if (criterio==ACHICAR){
		size_t i=actual;
		while(!ant && i>=2){
			if ((factorial(i-1) + 1)%i==0) ant=i;
			i--;
		}
		return ant;
	}
	size_t i=actual;
	while(!prox){
		if ((factorial(i-1) + 1)%i==0) prox=i;
		i++;
	}
	return prox;
}



void redimensionar(hash_t *hash,int criterio){
	
	hash_t *viejo= hash;
	hash_iter_t *iterador_viejo= hash_iter_crear(viejo);
	
	hash_t *nuevo=hash_crear(viejo->destruir_dato);
	
	while(!hash_iter_al_final(iterador_viejo)){
		
		const char *clave_vieja=hash_iter_ver_actual(iterador_viejo);
		char *clave_nueva=NULL;
		if (clave_vieja==NULL) continue;
		strcpy(clave_nueva,clave_vieja);
		
		size_t i= buscar_vacio(nuevo,clave_nueva);
		if(i==0 && hash->tabla[i]->estado!=VACIO) continue;
		nuevo->tabla[i]->clave=clave_nueva;
		nuevo->tabla[i]->dato=iterador_viejo->hash->tabla[(iterador_viejo->pos)]->dato;
			
		hash_iter_avanzar(iterador_viejo);
	}
	primos(viejo);
	//size_t n=calcular_primo(viejo,criterio);
	//nuevo->tabla=realloc(nuevo->tabla,n); 
	nuevo->tabla=realloc(nuevo->tabla,viejo->capacidad); //
	nuevo->cantidad=viejo->cantidad;
	nuevo->contador=viejo->contador;
	
	
	hash=nuevo;
	
	hash_iter_destruir(iterador_viejo);
	hash_destruir(viejo);

}



//-----------PRIMITIVAS DE HASH QUE DEPENDEN DE REDIMENSION-----------

void *hash_borrar(hash_t *hash, const char *clave){
	
	if(!hash_pertenece(hash,clave)) return NULL;
		
	size_t i=buscar_ocupado(hash,clave);
	if (i==0 && hash->tabla[i]->estado!=OCUPADO && strcmp(hash->tabla[i]->clave,clave)!=0) return NULL;

	void* dato;
	if (hash->destruir_dato!=NULL){
		dato=hash_obtener(hash,clave);
		hash->destruir_dato(hash->tabla[i]->dato);
	}
	dato=hash->tabla[i]->dato;
	hash->tabla[i]->estado= BORRADO;
	hash->tabla[i]->clave=NULL;
	
	hash->cantidad--;
	hash->borrados++;
	
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	if (carga >=7)	redimensionar(hash,AGRANDAR);

	return dato;
}


bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	
	size_t i;
	if(hash_pertenece(hash,clave)){
		i=buscar_ocupado(hash,clave);
		if (!hash->tabla[i]) return false;
		if (i==0 && hash->tabla[i]->estado!=OCUPADO && strcmp(hash->tabla[i]->clave,clave)!=0) return false;
		strcpy(hash->tabla[i]->clave,clave);
		
		hash->tabla[i]->dato= dato;
		hash->tabla[i]->estado= OCUPADO;	
		
		return true;
	}
	i=buscar_vacio(hash,clave);

	if (hash->tabla[i]) return false;
	
    hash->tabla[i]=calloc(1,sizeof(campo_t));
    
    if(!hash->tabla[i]) return false;
    
    hash->tabla[i]->clave=calloc(1,sizeof(char*));
    if(!hash->tabla[i]->clave) return NULL;
	
	strcpy(hash->tabla[i]->clave,clave);
	
	hash->tabla[i]->dato= dato;
	hash->tabla[i]->estado= OCUPADO;	
	hash->cantidad++;
	
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	if (carga >=7)	redimensionar(hash,AGRANDAR);
	return true;
}


