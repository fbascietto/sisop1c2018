/*
 * funcionesConsola.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include "planificador.h"


void block(char* key_value, int ESI_ID){

	bool coincideValor(void* key){

		t_clave* clave = (t_clave*) key;

		return strcmp(clave->claveValor, key_value) == 0;


	}

	bool coincideID(void* proceso){

		t_proceso_esi* esi = (t_proceso_esi*) proceso;

		return esi->id == ESI_ID;

	}

	t_clave* key = list_find(listaKeys, coincideValor);

	if(esi_ejecutando->id == ESI_ID){
		queue_push(key->colaBloqueados, esi_ejecutando);
	}else{
		t_proceso_esi* esi_a_bloquear = list_find(colaListos, coincideID);
		queue_push(key->colaBloqueados, esi_a_bloquear);
	}

}

void unblock(char* key_value){


	bool coincideValor(void* key){

		t_clave* clave = (t_clave*) key;

		return strcmp(clave->claveValor, key_value) == 0;


	}

	t_clave* key = list_find(listaKeys, coincideValor);
	t_proceso_esi* esi_a_desbloquear = queue_pop(key->colaBloqueados);
	list_add(colaListos, esi_a_desbloquear);

}
