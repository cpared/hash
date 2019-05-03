#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#define CAPACIDAD_INICIAL 11
#define POS_INICIAL 0
#define TAM_MIN 11


typedef enum {
	VACIO,
	OCUPADO,
	BORRADO,
}tipo_estado;

typedef enum {
	ACHICAR=1,
	AGRANDAR,
}criterio_t;


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
    size_t borrados;
};


struct hash_iter{
    size_t pos;
    const hash_t* hash;
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
    if (!hash->tabla[i] || hash->tabla[i]->estado!=OCUPADO || strcmp(hash->tabla[i]->clave,clave)!=0) return NULL;
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


//--------------------------BUSCADORES---------------------------------

size_t buscar_primero(const hash_t* hash){
	int pos=-1;
	for (size_t i=0; i< hash->capacidad; i++){
		if (hash->tabla[i] ){
			pos=(int)i;
			break;
		}
	}
	if (pos!=-1){
		return (size_t)pos;
	}
    
    return hash->capacidad;
}

size_t buscar_ultimo(const hash_t* hash){
	int pos=-1;
	size_t max=buscar_primero(hash);
	for (size_t i=hash->capacidad-1; i>max; i--){
		if (hash->tabla[i]){
			pos=(int)i;
			break;
		}
	}
	if (pos!=-1){
		return (size_t)pos;
	}
    
    return (size_t)POS_INICIAL;
}

//--------------------------ITERADOR---------------------------------


hash_iter_t *hash_iter_crear(const hash_t *hash){
    
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if(!iter) return NULL;

	iter->hash=hash;
    if(!iter->hash) return NULL;
    
    //busca la primera posicion ocupada
    iter->pos = buscar_primero(hash);
    return iter;
}


bool hash_iter_al_final(const hash_iter_t *iter){
    if( iter->pos == iter->hash->capacidad  ) return true;
	//|| iter->pos==buscar_ultimo(iter->hash)
	return false;
}


bool hash_iter_avanzar(hash_iter_t *iter){
	
	if (hash_iter_al_final(iter) || !iter) return false;
	//while(!iter->hash->tabla[iter->pos] && iter->hash->cantidad<=iter->pos){
	
	for(size_t i=iter->pos; i< iter->hash->capacidad;i++){
		if (iter->hash->tabla[i] && iter->hash->tabla[i]->estado==OCUPADO){
			 iter->pos=i;
			return true;
		}
	}
	return false;
	
			
}


const char *hash_iter_ver_actual(const hash_iter_t *iter){
	
	if(iter->hash->tabla[(iter->pos)] && iter->hash->tabla[(iter->pos)]->estado==OCUPADO) return iter->hash->tabla[(iter->pos)]->clave; 
	return NULL;
}


void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}


//-------------------------REDIMENSION---------------------------------


size_t factorial(size_t n){
	size_t c,fact;
	fact=1;
	for (c = 1; c <= n; c++){
		fact = fact * c;
	}
	return fact;		
}

size_t calcular_primo(hash_t* hash, int criterio){
	size_t ant;
	size_t actual=hash->capacidad;

	if (criterio==ACHICAR){
		if (actual==TAM_MIN) return actual;
		size_t i=actual;
		while(!ant){
			if ((factorial(i-1) + 1)%i==0) ant=i;
			i--;
		}
		return ant;
	}
	
	size_t i=actual;
	size_t prox=actual;
	while(prox==actual){
		if ((factorial(i-1) + 1)%i==0) prox=i;
		i++;
	}
	return prox;
}


void destruir_tabla(hash_t* hash){
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
}



void redimensionar(hash_t *hash,int criterio){
	
	
	hash_iter_t* iterador= hash_iter_crear(hash);
	
	hash_t* nuevo=hash_crear(hash->destruir_dato);
	
	size_t n=calcular_primo(hash,criterio);
	nuevo->tabla=realloc(nuevo->tabla,n); 
	
	
	while(!hash_iter_al_final(iterador)){
		
		const char *clave_vieja=hash_iter_ver_actual(iterador);
		if (!clave_vieja) continue;
		
		size_t i=buscar_vacio(nuevo,clave_vieja);

		if (nuevo->tabla[i])continue;
		
		nuevo->tabla[i]=calloc(1,sizeof(campo_t));
		
		if(!nuevo->tabla[i]) continue;
		
		nuevo->tabla[i]->clave=calloc(1,sizeof(char*));
		if(!nuevo->tabla[i]->clave) continue;

		strcpy(nuevo->tabla[i]->clave,clave_vieja);

		nuevo->tabla[i]->dato=iterador->hash->tabla[(iterador->pos)]->dato;
		
		nuevo->cantidad++;	
		hash_iter_avanzar(iterador);
	}
	destruir_tabla(hash);
	hash->tabla=nuevo->tabla;
	free(nuevo);
	hash_iter_destruir(iterador);
	

}



//-----------PRIMITIVAS DE HASH QUE DEPENDEN DE REDIMENSION-----------

void *hash_borrar(hash_t *hash, const char *clave){
	
	if(!hash_pertenece(hash,clave)) return NULL;
		
	size_t i=buscar_ocupado(hash,clave);
	if (!hash->tabla[i]) return NULL;
	if (i==0 && hash->tabla[i]->estado!=OCUPADO && strcmp(hash->tabla[i]->clave,clave)!=0) return NULL;

	void* dato;
	if (hash->destruir_dato!=NULL){
		dato=hash_obtener(hash,clave);
		hash->destruir_dato(hash->tabla[i]->dato);
	}
	dato=hash->tabla[i]->dato;
	hash->tabla[i]->estado= BORRADO;
	//hash->tabla[i]->clave='\0';
	hash->tabla[i]->clave=NULL;
	hash->cantidad--;
	hash->borrados++;
	
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	if (carga >=0.7)	redimensionar(hash,AGRANDAR);

	return dato;
}


bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	
	size_t i;
	if(hash_pertenece(hash,clave)){
		i=buscar_ocupado(hash,clave);
		if (!hash->tabla[i] || hash->tabla[i]->estado!=OCUPADO || strcmp(hash->tabla[i]->clave,clave)!=0) return false;
		strcpy(hash->tabla[i]->clave,clave);

		hash->tabla[i]->dato= dato;
		hash->tabla[i]->estado= OCUPADO;	
		
		return true;
	}
	float carga= (float)(hash->cantidad+ hash->borrados)/ (float) hash->capacidad;
	if (carga >=0.7)	redimensionar(hash,AGRANDAR);
	
	i=buscar_vacio(hash,clave);

	if (hash->tabla[i]) return false;
	
    hash->tabla[i]=calloc(1,sizeof(campo_t));
    
    if(!hash->tabla[i]) return false;
    
    hash->tabla[i]->clave=calloc(1,sizeof(char*));
    if(!hash->tabla[i]->clave) return false;
	
	strcpy(hash->tabla[i]->clave,clave);
	
	hash->tabla[i]->dato= dato;
	hash->tabla[i]->estado= OCUPADO;	
	hash->cantidad++;
	
	return true;
}


