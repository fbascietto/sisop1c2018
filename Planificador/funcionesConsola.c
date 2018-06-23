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
		if(esi_ejecutando != NULL){
			if(esi_ejecutando->id == ESI_ID){
				queue_push(key->colaBloqueados, esi_ejecutando);
				log_info(logPlan, "el esi estaba ejecutando");
			}
		}else{
			t_proceso_esi* esi_a_bloquear = removerEsiSegunID(colaListos->elements, ESI_ID);
			if(esi_a_bloquear != NULL){
				queue_push(key->colaBloqueados, esi_a_bloquear);
			}else{
				log_info(logPlan, "el esi no estaba ni en ejecucion ni en listos");
			}
		}
		log_info(logPlan, "esi %d enviado a la cola de bloqueados de la key %s", ESI_ID, key_value);
	} else{
		log_info(logPlan, "la key ingresada no existe");
	}
}

void unblock(char* key_value){

	t_clave* key = obtenerKey(key_value);
	if(key != NULL){
		if(queue_size(key->colaBloqueados) > 0){
			t_proceso_esi* esi_a_desbloquear = queue_pop(key->colaBloqueados);
			cambiarEstimado(esi_a_desbloquear);
			moverAListos(esi_a_desbloquear);
			log_info(logPlan, "esi %d desbloqueado", esi_a_desbloquear->id);
		} else{
			key->esi_poseedor = NULL;
			log_info(logPlan, "no hay procesos bloqueados, se libero la clave");
		}
	} else{
		log_info(logPlan, "la clave no existe");
	}

}

void pauseScheduler(){
	pausarPlanificacion=true;
}


void goOn(){
	pausarPlanificacion=false;
	pthread_mutex_unlock(&pausarPlanificacionSem);
}


void esperarPlanificador(){
	comandoConsola=true;
	pthread_mutex_lock(&iniciarConsolaSem);
}

void continuarPlanificador(){
	comandoConsola=false;
	pthread_mutex_unlock(&esperarConsolaSem);
}

void getStatus(char* keySearch){
	char* keyValue;
	char* mensajeBusqueda;

	t_clave* key = obtenerKey(keySearch);

	keyValue = (key != NULL) ? key->claveValor : NULL;

	enviarInt(socketConsolaCoordinador,DONDE_ESTA_LA_CLAVE);

	int busquedaClave;

	recibirInt(socketConsolaCoordinador, &busquedaClave);

	char* instanciaBusqueda = recibirMensajeArchivo(socketConsolaCoordinador);

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

	clave->esi_poseedor = NULL;

	if(queue_size(clave->colaBloqueados)>0){

		t_proceso_esi* esi_a_desbloquear = queue_pop(clave->colaBloqueados);

		moverAListos( esi_a_desbloquear);
	}

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

	bool coincideID(void* esi){
		t_proceso_esi* unEsi = (void*) esi;
		return unEsi->id == ESI_ID;
	}

	bool estaElESIBloqueado(void* key){
		t_clave* clave = (void*) key;
		return list_any_satisfy(clave->colaBloqueados->elements, coincideID);
	}


	t_proceso_esi* proceso_a_matar;
	proceso_a_matar = esi_ejecutando;
	bool coincide;

	if(proceso_a_matar != NULL){
		coincide = coincideID(proceso_a_matar);
		if(coincide){
			log_trace(logPlan, "el proceso a matar estaba en ejecucion");
			finalizarESIEnEjecucion();
		}
	}

	if (proceso_a_matar == NULL || !coincide){

		//		t_clave* clave_bloqueando_proceso = obtenerKeySegunProcesoBloqueado(ESI_ID);
		//		proceso_a_matar = encontrarEsiSegunID(clave_bloqueando_proceso->colaBloqueados->elements, ESI_ID);

		log_trace(logPlan, "el proceso a matar no estaba en ejecucion");

		t_clave* clavePoseedora = list_find(listaKeys, estaElESIBloqueado);

		if(clavePoseedora !=NULL){
			t_list* esisBloqueados = clavePoseedora->colaBloqueados->elements;
			proceso_a_matar = list_find(esisBloqueados, coincideID);
			list_remove_by_condition(esisBloqueados, coincideID);
			log_trace(logPlan, "el proceso %d a matar estaba bloqueado", proceso_a_matar->id);
		}

		//		if(proceso_a_matar != NULL){
		//
		//			removerEsiSegunID(clave_bloqueando_proceso->colaBloqueados->elements, proceso_a_matar->id);
		//
		//		}else{

		else{

			proceso_a_matar = list_remove_by_condition(colaListos->elements, coincideID);

			if(proceso_a_matar != NULL){
				log_trace(logPlan, "el proceso %d a matar estaba en listos", proceso_a_matar->id);
			}
			else{
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

void agregarElementos(t_list* origen, t_list* destino){

	int i;

	for(i=0; i<list_size(origen); i++){

		list_add(destino, list_get(origen, i));

	}

}


void detectarDeadlock(){

	int i;
	t_clave* key;
	t_list* keysAsignadas;
	t_list* procesosEnDeadlock = list_create();
	encontroDeadlock = false;

	for(i=0; i<list_size(listaKeys); i++){

		key = list_get(listaKeys, i);

		list_add(procesosEnDeadlock, key->esi_poseedor);

		//keysAsignadas = obtenerKeysAsignadasDeUnProceso(key->esi_poseedor);

		verificarEsperaCircular(key->esi_poseedor->clavesTomadas, procesosEnDeadlock);

		if(encontroDeadlock){
			break;
		}

		list_clean(procesosEnDeadlock);

	}

	list_destroy(procesosEnDeadlock);

}

void verificarEsperaCircular(t_list* keys, t_list* procesosEnDeadlock){

	int i;
	t_clave* key;

	t_list* procesosEnDeadlockAux = list_create();

	for(i=0; i<list_size(keys); i++){

		if(!list_is_empty(procesosEnDeadlock)) agregarElementos(procesosEnDeadlock, procesosEnDeadlockAux);

		key = list_get(keys, i);

		if(!list_is_empty(key->colaBloqueados->elements)) verificarEsperaCircularParaUnaKey(key, procesosEnDeadlockAux);

		if(encontroDeadlock){

			imprimirIDs(procesosEnDeadlockAux);
			break;

		}

		list_clean(procesosEnDeadlockAux);

	}

	list_destroy(procesosEnDeadlockAux);

}

void verificarEsperaCircularParaUnaKey(t_clave* key, t_list* procesosEnDeadlock){

	int i;
	t_proceso_esi* proceso;
	t_proceso_esi* processToCompare;
	t_list* keysAsignadas;
	t_clave* keyToCompare;
	t_list* procesosEnDeadlockAux = list_create();

	for(i=0; i<list_size(key->colaBloqueados->elements); i++){

		proceso = list_get(key->colaBloqueados->elements, i);

		//keysAsignadas = obtenerKeysAsignadasDeUnProceso(proceso);

		if(!list_is_empty(proceso->clavesTomadas)){

			if(!list_is_empty(procesosEnDeadlock)) agregarElementos(procesosEnDeadlock, procesosEnDeadlockAux);

			//se compara con el primer proceso (posicion 0) para ver si es bloqueado por alguna de las keys del proceso
			processToCompare = list_get(procesosEnDeadlockAux, 0);

			keyToCompare = obtenerKeySegunProcesoBloqueado(processToCompare->id);


			if(estaLaKey(proceso->clavesTomadas, keyToCompare)){

				encontroDeadlock = true;
				list_add(procesosEnDeadlock, proceso);
				break;

			}else{

				list_add(procesosEnDeadlockAux, proceso);
				verificarEsperaCircular(proceso->clavesTomadas, procesosEnDeadlockAux);

			}

		}

		list_clean(procesosEnDeadlockAux);

	}

	list_destroy(procesosEnDeadlockAux);


}


// t_list* obtenerKeysAsignadasDeUnProceso(t_proceso_esi* proceso){

// 	t_list* keysAsignadas = list_create();
// 	t_clave* key;
// 	int i;

// 	for(i=0; i<list_size(listaKeys); i++){

// 		key = list_get(listaKeys, i);

// 		if(coincideID(key->esi_poseedor->id, proceso->id)) list_add(keysAsignadas, key);

// 	}

// 	return keysAsignadas;

// }


bool estaLaKey(t_list* keys, t_clave* key){

	int i;
	t_clave* keyEncontrada;

	for(i=0; i<list_size(keys); i++){

		keyEncontrada = list_get(keys, i);

		if(coincideValor(key->claveValor, keyEncontrada->claveValor)) return true;

	}

	return false;

}



void imprimirIDs(t_list* procesosEnDeadlock){

	int i;
	t_proceso_esi* proceso;

	printf("Estan en deadlock los procesos: ");

	for(i=0; i<list_size(procesosEnDeadlock); i++){

		proceso = list_get(procesosEnDeadlock, i);

		printf("%d", proceso->id);

		if(i + 1 != list_size(procesosEnDeadlock)){
			printf(", ");
		}else{
			printf(".\n");
		}

	}

}






