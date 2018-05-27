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
		t_proceso_esi* esi_a_bloquear = list_find(colaListos->elements, coincideID);
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
	list_add(colaListos->elements, esi_a_desbloquear);

}

void pauseScheduler(){
	sem_wait(pausarPlanificacion);
}

void goOn(){
	sem_post(pausarPlanificacion);
}

void listBlockedProcesses(char* keySearch){
	bool coincideValor(void* key){
		t_clave* clave = (t_clave*) key;
		return strcmp(clave->claveValor, keySearch) == 0;
	}
	void printEsi(void* procesoEsi){
		t_proceso_esi* esi = (t_proceso_esi*) procesoEsi;
		printf("ESI ID: ""%d"".\n", esi->id);
	}

	t_clave* key = list_find(listaKeys, coincideValor);

	if(key==NULL){
		printf("No se encontró el recurso con key: ""%s"".\n", keySearch);

	} else if (queue_is_empty(key->colaBloqueados)){
		printf("La lista de bloqueados está vacía.\n");

	} else {
		printf("La lista de bloqueados para la clave especificada es:\n");
		list_iterate(key->colaBloqueados->elements, printEsi);
	}
}

void matarProceso(int ESI_ID){

	t_proceso_esi* proceso_a_matar;

	bool coincideID(void* proceso){

		t_proceso_esi* esi = (t_proceso_esi*) proceso;

		return esi->id == ESI_ID;

	}

	bool coincideCola(void* key){

		t_clave* clave = (t_clave*) key;

		t_proceso_esi* un_esi = list_find(clave->colaBloqueados->elements, coincideID);

		return un_esi != NULL;

	}

	void liberarPrimero(void* key){

		t_clave* clave = (t_clave*) key;

		t_proceso_esi* esi_a_desbloquear = queue_pop(clave->colaBloqueados);

		list_add(colaListos->elements, esi_a_desbloquear);

	}

	bool coincideProceso(void* proceso){

		t_proceso_esi* esi = (t_proceso_esi*) proceso;

		return esi->id == proceso_a_matar->id;

	}

	proceso_a_matar = esi_ejecutando;

	if(coincideID(proceso_a_matar)){

		esi_ejecutando = NULL;

	}else{

		t_clave* clave_bloqueando_proceso = list_find(listaKeys, coincideCola);
		proceso_a_matar = list_find(clave_bloqueando_proceso->colaBloqueados->elements, coincideID);

		if(proceso_a_matar != NULL){

			list_remove_by_condition(clave_bloqueando_proceso->colaBloqueados->elements, coincideProceso);

		}else{

			proceso_a_matar = list_find(colaListos->elements, coincideID);

			if(proceso_a_matar != NULL){

				list_remove_by_condition(colaListos->elements, coincideProceso);

			}else{

				printf("No existe el id de proceso %d\n", ESI_ID);

			}

		}

	}


	if(proceso_a_matar != NULL){

		enviarInt(proceso_a_matar->fd, ABORTAR);

		list_iterate(proceso_a_matar->clavesTomadas, liberarPrimero);
		queue_push(colaTerminados, proceso_a_matar);

	}
}

