/*
 * funcionesConsola.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include "planificador.h"


bool coincideID(int idP1, int idP2){

	return idP1 == idP2;

}


bool coincideValor(char* keyValue1, char* keyValue2){

	return strcmp(keyValue1, keyValue2) == 0;


}

bool coincideCola(t_queue* procesos, int idProceso){

	int i;
	t_proceso_esi* procesoEncontrado;

	for(i=0; i<list_size(procesos->elements); i++){

		procesoEncontrado = list_get(procesos->elements, i);

		if(coincideID(procesoEncontrado->id, idProceso)){
			return true;
		}

	}

	return false;

}


/*
 * Tener en cuenta que puede devolver NULL
 */
t_clave* obtenerKey(char* key_value){

	int i;
	t_clave* keyEncontrada;

	for(i=0; i<list_size(listaKeys); i++){

		keyEncontrada = list_get(listaKeys, i);

		if(coincideValor(keyEncontrada->claveValor, key_value)){
			return keyEncontrada;
		}

	}

	keyEncontrada = NULL;
	return keyEncontrada;

}


t_proceso_esi* removerEsiSegunID(t_list* procesos, int ID){

	int i;
	t_proceso_esi* esiRemovido;

	for(i=0; i<list_size(procesos); i++){

		esiRemovido = list_get(procesos, i);

		if(coincideID(esiRemovido->id, ID)){
			list_remove(procesos, i);
			break;
		}

		esiRemovido = NULL;

	}

	return esiRemovido;

}


void block(char* key_value, int ESI_ID){

	t_clave* key = obtenerKey(key_value);

	if(key != NULL){
		if(esi_ejecutando->id == ESI_ID){
			queue_push(key->colaBloqueados, esi_ejecutando);
		}else{
			t_proceso_esi* esi_a_bloquear = removerEsiSegunID(colaListos->elements, ESI_ID);
			if(esi_a_bloquear != NULL){
				sem_wait(&productorConsumidor);
				queue_push(key->colaBloqueados, esi_a_bloquear);
			}
		}
	}

}

void unblock(char* key_value){


	t_clave* key = obtenerKey(key_value);
	t_proceso_esi* esi_a_desbloquear = queue_pop(key->colaBloqueados);
	moverAListos(esi_a_desbloquear);

}

void pauseScheduler(){
	pausarPlanificacion = true;
}

void goOn(){
	pausarPlanificacion = false;
	pthread_mutex_unlock(&pausarPlanificacionSem);
}

void getStatus(char* keySearch){
	char* keyValue;
	char* mensajeBusqueda;

	t_clave* key = obtenerKey(keySearch);

	keyValue = (key != NULL) ? key->claveValor : NULL;

	enviarInt(socketCoordinador,DONDE_ESTA_LA_CLAVE);

	pthread_mutex_lock(&respuestaBusquedaClave);

	switch(busquedaClave){
	case CLAVE_ENCONTRADA:
		mensajeBusqueda = "La clave fue encontrada";
		break;
	case CLAVE_NO_ENCONTRADA:
		mensajeBusqueda = "La clave no fue encontrada, se simula distribucion";
		break;
	}

	t_queue* bloqueados = key->colaBloqueados;

	printf("Valor de clave: ""%s"".\n", keyValue);
	printf("Resultado de la búsqueda: ""%s"".\n", mensajeBusqueda);
	printf("Instancia: ""%s"".\n", instanciaBusqueda);
	printf("Listado de esi bloqueados por clave: \n");
	if(queue_is_empty(bloqueados)){
		printf("Vacío");
	}else{
		int i;
		for (i = 0; i < list_size(bloqueados->elements); ++i) {
			t_proceso_esi* esi = list_get(bloqueados->elements,i);
			printf("ESI id: ""%d"".\n",esi->id);
		}
	}

}

void listBlockedProcesses(char* keySearch){
	void printEsi(void* procesoEsi){
		t_proceso_esi* esi = (t_proceso_esi*) procesoEsi;
		printf("ESI ID: ""%d"".\n", esi->id);
	}

	t_clave* key = obtenerKey(keySearch);

	if(key==NULL){
		printf("No se encontró el recurso con key: ""%s"".\n", keySearch);

	} else if (queue_is_empty(key->colaBloqueados)){
		printf("La lista de bloqueados está vacía.\n");

	} else {
		printf("La lista de bloqueados para la clave especificada es:\n");
		list_iterate(key->colaBloqueados->elements, printEsi);
	}
}


void liberarKeys(t_proceso_esi* esi){
	list_iterate(esi->clavesTomadas, liberarKey);
}

void liberarKey(void* key){

	t_clave* clave = (t_clave*) key;

	t_proceso_esi* esi_a_desbloquear = queue_pop(clave->colaBloqueados);

	clave->esi_poseedor = NULL;

	queue_push(colaListos, esi_a_desbloquear);

}

t_clave* obtenerKeySegunProcesoBloqueado(int esiID){

	int i;
	t_clave* keyEncontrada;

	for(i=0; i<list_size(listaKeys); i++){

		keyEncontrada = list_get(listaKeys, i);

		if(coincideCola(keyEncontrada->colaBloqueados, esiID)){
			return keyEncontrada;
		}

	}

	keyEncontrada = NULL;
	return keyEncontrada;

}

t_proceso_esi* encontrarEsiSegunID(t_list* procesos, int ID){

	int i;
	t_proceso_esi* esiEncontrado;

	for(i=0; i<list_size(procesos); i++){

		esiEncontrado = list_get(procesos, i);

		if(coincideID(esiEncontrado->id, ID)){
			return esiEncontrado;
		}

	}

	esiEncontrado = NULL;
	return esiEncontrado;

}

void matarProceso(int ESI_ID){

	t_proceso_esi* proceso_a_matar;

	proceso_a_matar = esi_ejecutando;

	if(coincideID(proceso_a_matar->id, ESI_ID)){

		esi_ejecutando = NULL;

	}else{

		t_clave* clave_bloqueando_proceso = obtenerKeySegunProcesoBloqueado(ESI_ID);
		proceso_a_matar = encontrarEsiSegunID(clave_bloqueando_proceso->colaBloqueados->elements, ESI_ID);

		if(proceso_a_matar != NULL){

			removerEsiSegunID(clave_bloqueando_proceso->colaBloqueados->elements, proceso_a_matar->id);

		}else{

			proceso_a_matar = encontrarEsiSegunID(colaListos->elements, ESI_ID);

			if(proceso_a_matar != NULL){

				removerEsiSegunID(colaListos->elements, proceso_a_matar->id);

			}else{

				printf("No existe el id de proceso %d\n", ESI_ID);

			}

		}

	}


	if(proceso_a_matar != NULL){

		enviarInt(proceso_a_matar->fd, ABORTAR);
		liberarKeys(proceso_a_matar);
		queue_push(colaTerminados, proceso_a_matar);

	}
}

void detectarDeadlock(){

	int i;
	t_clave* key;
	t_proceso_esi* esiInterbloqueo = NULL;

	//----------------------------------------------

	for(i=0; i<list_size(listaKeys); i++){

		key = list_get(listaKeys, i);

		//		if(key->esi_poseedor == NULL){
		//
		//			//si no hay ningun esi que posea esa key
		//			list_remove(listaKeysReemplazo, i);
		//			i--;
		//
		//		}else{
		//
		//
		//
		//		}

		if(estaBloqueadoPorAlgunoDeLaCola(key->colaBloqueados, key->esi_poseedor, esiInterbloqueo)){

			printf("El ESI %d y el ESI %d estan en deadlock.\n", key->esi_poseedor->id, esiInterbloqueo->id);

		}

	}

}

bool estaBloqueadoPorAlgunoDeLaCola(t_queue* bloqueadosPorEsi1, t_proceso_esi* esi1, t_proceso_esi* esiInterbloqueo){


	t_clave* keyQueNecesitaEsi1;
	t_proceso_esi* esiAux;

	bool coincideProceso(void* proceso){

		t_proceso_esi* unEsi = (t_proceso_esi*) proceso;

		return unEsi->id == keyQueNecesitaEsi1->esi_poseedor->id;

	}


	if(estaBloqueado(esi1, keyQueNecesitaEsi1)){

		esiAux = list_find(bloqueadosPorEsi1->elements, coincideProceso);

		if(esiAux == NULL){
			//significa que no hay ningun deadlock directo
			return false;
		}

		esiInterbloqueo = esiAux;
		return true;

	}

	return false;

}

bool estaBloqueado(t_proceso_esi* esi, t_clave* keyQueNecesita){


	bool coincideID(void* procesoEsi){

		t_proceso_esi* proceso = (t_proceso_esi*) procesoEsi;

		return proceso->id == esi->id;

	}

	bool coincideCola(void* key){

		t_clave* clave = (t_clave*) key;

		t_proceso_esi* un_esi = list_find(clave->colaBloqueados->elements, coincideID);

		return un_esi != NULL;

	}

	t_clave* key;

	key = list_find(listaKeys, coincideCola);

	if(key == NULL){
		return false;
	}

	keyQueNecesita = key;
	return true;


}








