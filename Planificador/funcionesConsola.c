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

		if(coincideValor(keyEncontrada->nombre, key_value)){
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
			return (t_proceso_esi*) list_remove(procesos, i);
		}


	}

	esiRemovido = NULL;
	return esiRemovido;

}


void block(char* key_value, int ESI_ID){

	t_clave* key = obtenerKey(key_value);

	bool coincideID(void* esi){
		t_proceso_esi* unEsi = (void*) esi;
		return unEsi->id == ESI_ID;
	}

	bool estaElESIBloqueado(void* key){
		t_clave* clave = (void*) key;
		return list_any_satisfy(clave->colaBloqueados->elements, coincideID);
	}



	if(key != NULL){

		if(esi_ejecutando != NULL){

			if(esi_ejecutando->id == ESI_ID){
				queue_push(key->colaBloqueados, esi_ejecutando);
			}

		}
		else{
			t_proceso_esi* esi_a_bloquear = removerEsiSegunID(colaListos->elements, ESI_ID);

			if(esi_a_bloquear != NULL){
				seQuitoUnEsiDeListos=true;
				queue_push(key->colaBloqueados, esi_a_bloquear);
				log_debug(logPlan, "esi %d enviado a la cola de bloqueados de la key %s", ESI_ID, key_value);

			}
			else{
				t_clave* clavePoseedora = list_find(listaKeys, estaElESIBloqueado);
				if(clavePoseedora !=NULL) log_warning(logPlan, "el proceso %d a matar ya esta bloqueado por la key %s", ESI_ID, clavePoseedora->nombre);
			}

		}

	}
	else{
		log_warning(logPlan, "la key ingresada no existe");
	}
}

void unblock(char* key_value){

	t_clave* key = obtenerKey(key_value);
	if(key != NULL){
		if(queue_size(key->colaBloqueados) > 0){
			t_proceso_esi* esi_a_desbloquear = queue_pop(key->colaBloqueados);
			cambiarEstimado(esi_a_desbloquear);
			moverAListos(esi_a_desbloquear);
			log_debug(logPlan, "esi %d desbloqueado", esi_a_desbloquear->id);
		} else{
			key->esi_poseedor = NULL;
			log_debug(logPlan, "no hay procesos bloqueados, se libero la clave");
		}
	} else{
		log_warning(logPlan, "la clave no existe");
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

/*
 * Consulta al coordinador el valor de una clave, imprime resultado.
 */
int obtenerValor(char* keySearch){
	enviarInt(socketConsolaCoordinador,OBTENER_VALOR_DE_KEY);
	enviarMensaje(socketConsolaCoordinador, keySearch);
	int busquedaClave;
	char* valor;

	recibirInt(socketConsolaCoordinador, &busquedaClave);

	char* msj = string_new();

	switch(busquedaClave){
	case CLAVE_ENCONTRADA:
		valor = recibirMensajeArchivo(socketConsolaCoordinador);
		string_append_with_format(&msj, "Valor de la clave: %s", valor);
		break;
	case CLAVE_NO_ENCONTRADA:
		string_append(&msj, "Clave no existente.");
		break;
	case CLAVE_CREADA:
		string_append(&msj, "Clave sin valor.");
		break;
	case NO_HAY_INSTANCIAS_CONECTADAS:
		string_append(&msj, "No hay instancias conectadas.");
		return NO_HAY_INSTANCIAS_CONECTADAS;
		break;
	}

	log_debug(logPlan, msj);

	free(msj);

	return 1;

}

/*
 * Consulta al coordinador la instancia contenedora de una key, imprime resultado
 */
void obtenerInstancia(char* keySearch){
	enviarInt(socketConsolaCoordinador,DONDE_ESTA_LA_CLAVE);

	enviarMensaje(socketConsolaCoordinador,keySearch);

	int busquedaClave;

	recibirInt(socketConsolaCoordinador, &busquedaClave);

	char* msj = string_new();
	char* msj2 = string_new();

	string_append(&msj, "Resultado de la búsqueda: ");

	switch(busquedaClave){
	case CLAVE_ENCONTRADA:
		string_append(&msj, "La clave fue encontrada.");
		string_append(&msj2, "La clave esta en: ");
		break;
	case CLAVE_NO_ENCONTRADA:
		string_append(&msj, "La clave no fue encontrada, se simula distribucion.");
		string_append(&msj2, "La clave se guardaria en: ");
		break;
	case NO_HAY_INSTANCIAS_CONECTADAS:
		string_append(&msj, "No hay instancias conectadas.");
		break;
	}
	log_debug(logPlan, msj);

	char* instanciaBusqueda = recibirMensajeArchivo(socketConsolaCoordinador);

	string_append(&msj2, instanciaBusqueda);
	log_debug(logPlan, msj2);

	free(msj);
	free(msj2);
	free(instanciaBusqueda);


}

void getStatus(char* keySearch){
	mostarEsiPoseedor(keySearch);
	if(obtenerValor(keySearch) != NO_HAY_INSTANCIAS_CONECTADAS){
		obtenerInstancia(keySearch);
	}
	listBlockedProcesses(keySearch);
}

void mostarEsiPoseedor(char* keySearch){

	t_clave* key = obtenerKey(keySearch);
	char* msj = string_new();

	if(key != NULL){

		if(key->esi_poseedor == NULL){

			string_append_with_format(&msj, "La clave no esta asignada a ningun proceso.");

		}else if(key->esi_poseedor->id == BLOQUEO_SISTEMA){

			string_append_with_format(&msj, "La clave esta bloqueada por el sistema.");

		}else{

			string_append_with_format(&msj, "La clave esta asignada al ESI %d", key->esi_poseedor->id);

		}

		log_debug(logPlan, msj);

	}

	free(msj);

}

void imprimirEsis(t_queue* bloqueados, char* msj) {

	int i;

	for (i = 0; i < list_size(bloqueados->elements); ++i) {
		t_proceso_esi* esi = list_get(bloqueados->elements, i);
		string_append_with_format(&msj, "ESI %d", esi->id);
		if (i + 1 < list_size(bloqueados->elements)) {
			string_append(&msj, ", ");
		} else {
			string_append(&msj, ".");
		}
	}

	log_debug(logPlan, msj);

}

void listBlockedProcesses(char* keySearch){

	t_clave* key = obtenerKey(keySearch);
	char* msj = string_new();

	if(key == NULL){

		string_append_with_format(&msj, "No se encontro la key con nombre: %s", keySearch);
		log_warning(logPlan, msj);
		free(msj);
		return;

	}

	if(key != NULL){
		t_queue* bloqueados = key->colaBloqueados;

		if(queue_is_empty(bloqueados)){
			string_append_with_format(&msj, "No hay bloqueados por la clave %s.", keySearch);
			log_debug(logPlan, msj);
		}else{
			string_append_with_format(&msj, "Listado de esis bloqueados por %s: ", keySearch);
			imprimirEsis(bloqueados, msj);
		}


	}

	free(msj);

}

void listFinished(){

	char* msj = string_new();

	if(queue_is_empty(colaTerminados)){

		string_append_with_format(&msj, "No hay procesos terminados.");
		log_debug(logPlan, msj);
		free(msj);
		return;

	}

	imprimirEsis(colaTerminados, msj);

	free(msj);

}


void liberarKeys(t_proceso_esi* esi){
	list_iterate(esi->clavesTomadas, liberarKey);
}

void liberarKey(void* key){

	t_clave* clave = (t_clave*) key;
	clave->esi_poseedor = NULL;

	if(queue_size(clave->colaBloqueados)>0){
		t_proceso_esi* esi_a_desbloquear = queue_pop(clave->colaBloqueados);
		cambiarEstimado(esi_a_desbloquear);
		moverAListos(esi_a_desbloquear);
	}

	log_trace(logPlan, "clave %s liberada", clave->nombre);

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

void quitarBloqueoSistema() {
	t_proceso_esi* esi;
	t_clave* clave;
	int i;
	for (i = 0; i < list_size(listaKeys); i++) {
		clave = list_get(listaKeys, i);
		if(!estaLibre(clave)){
			esi = clave->esi_poseedor;
			if (esi->id == BLOQUEO_SISTEMA) {
				//				liberarKeys(esiBloqueoSistema);

				for(int j=0; j<list_size(esi->clavesTomadas); j++){

					t_clave* clave = list_get(esi->clavesTomadas, j);
					clave->esi_poseedor = NULL;

					if(queue_size(clave->colaBloqueados)>0){
						t_proceso_esi* esi_a_desbloquear = queue_pop(clave->colaBloqueados);
						cambiarEstimado(esi_a_desbloquear);
						moverAListos(esi_a_desbloquear);
					}

					char* claveID = malloc(LONGITUD_CLAVE);
					strcpy(claveID, clave->nombre);
					log_trace(logPlan, "clave %s liberada", claveID);
					free(claveID);
				}

				list_destroy(esiBloqueoSistema->clavesTomadas);
				free(esiBloqueoSistema);
				clave->esi_poseedor=NULL;
				break;
			}
		}
	}
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

	if(ESI_ID == BLOQUEO_SISTEMA){
		quitarBloqueoSistema();
		log_trace(logPlan, "se liberaron las claves iniciales bloqueadas");
		return;
	}


	t_proceso_esi* proceso_a_matar;
	proceso_a_matar = esi_ejecutando;
	bool coincide;

	if(proceso_a_matar != NULL){
		coincide = coincideID(proceso_a_matar);
		if(coincide){
			log_trace(logPlan, "el proceso a matar estaba en ejecucion");
			liberarKeys(proceso_a_matar);
			finalizarESIEnEjecucion();
		}
	}

	if (proceso_a_matar == NULL || !coincide){

		t_clave* clavePoseedora = list_find(listaKeys, estaElESIBloqueado);

		if(clavePoseedora !=NULL){
			t_list* esisBloqueados = clavePoseedora->colaBloqueados->elements;
			proceso_a_matar = list_find(esisBloqueados, coincideID);
			list_remove_by_condition(esisBloqueados, coincideID);
			log_trace(logPlan, "el proceso %d a matar estaba bloqueado", proceso_a_matar->id);
		}

		else{

			proceso_a_matar = list_remove_by_condition(colaListos->elements, coincideID);

			if(proceso_a_matar != NULL){
				seQuitoUnEsiDeListos=true; //para los semaforos de planificacion
				log_trace(logPlan, "el proceso %d a matar estaba en listos", proceso_a_matar->id);
			}
			else{
				log_warning(logPlan, "No existe el id de proceso %d\n", ESI_ID);
			}

		}

	}

	if(proceso_a_matar != NULL){
		liberarKeys(proceso_a_matar);
		enviarInt(proceso_a_matar->fd, ABORTAR);
		finalizarESI(proceso_a_matar);
	}
}

void agregarElementos(t_list* origen, t_list* destino){

	int i;

	for(i=0; i<list_size(origen); i++){

		list_add(destino, list_get(origen, i));

	}

}

bool procesoYaDetectado(int id, t_list* deadlocks){

	int i;
	t_list* deadlock;
	t_proceso_esi* proceso;

	for(i=0; i<list_size(deadlocks); i++){

		deadlock = list_get(deadlocks, i);

		int j;

		for(j=0; j<list_size(deadlock); j++){

			proceso = list_get(deadlock, j);

			if(coincideID(id, proceso->id)) return true;

		}

	}

	return false;

}


void detectarDeadlock(){

	int i;
	t_clave* key;
	t_list* keysFiltradas = list_filter(listaKeys, estaTomada);

	for(i=0; i<list_size(keysFiltradas); i++){

		encontroDeadlock = false;
		yaImprimioDeadlock = false;

		t_list* procesosEnDeadlock = list_create();
		key = list_get(keysFiltradas, i);

		if(key->esi_poseedor->id != BLOQUEO_SISTEMA){

			if(!list_is_empty(deadlocks)){

				if(procesoYaDetectado(key->esi_poseedor->id, deadlocks)) continue;

			}

			list_add(procesosEnDeadlock, key->esi_poseedor);

			//keysAsignadas = obtenerKeysAsignadasDeUnProceso(key->esi_poseedor);

			verificarEsperaCircular(key->esi_poseedor->clavesTomadas, procesosEnDeadlock);

		}

	}

	if(list_is_empty(deadlocks)) log_info(logPlan, "No hay deadlock");

	list_clean(deadlocks);


}

void verificarEsperaCircular(t_list* keys, t_list* procesosEnDeadlock){

	int i;
	t_clave* key;

	t_list* procesosEnDeadlockAux = list_create();

	for(i=0; i<list_size(keys); i++){

		if(!list_is_empty(procesosEnDeadlock)) agregarElementos(procesosEnDeadlock, procesosEnDeadlockAux);

		key = list_get(keys, i);

		if(!list_is_empty(key->colaBloqueados->elements)) verificarEsperaCircularParaUnaKey(key, procesosEnDeadlockAux);

		if(yaImprimioDeadlock) break;

		if(encontroDeadlock) {
			imprimirIDs(procesosEnDeadlockAux);
			yaImprimioDeadlock = true;
			t_list* deadlockPosta = list_create();
			agregarElementos(procesosEnDeadlockAux, deadlockPosta);
			list_add(deadlocks, deadlockPosta);
		}

		list_clean(procesosEnDeadlockAux);

	}

	list_destroy(procesosEnDeadlockAux);

}

void verificarEsperaCircularParaUnaKey(t_clave* key, t_list* procesosEnDeadlock){

	int i;
	t_proceso_esi* proceso;
	t_proceso_esi* processToCompare;
	//t_list* keysAsignadas;
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

		if(coincideValor(key->nombre, keyEncontrada->nombre)) return true;

	}

	return false;

}



void imprimirIDs(t_list* procesosEnDeadlock){

	int i;
	t_proceso_esi* proceso;

	char* esis = string_new();
	string_append(&esis, "Estan en deadlock los procesos: ");

	for(i=0; i<list_size(procesosEnDeadlock); i++){

		proceso = list_get(procesosEnDeadlock, i);

		string_append_with_format(&esis, "%d", proceso->id);

		if(i + 1 != list_size(procesosEnDeadlock)){
			string_append(&esis, ", ");
		}else{
			string_append(&esis, ".");
		}

	}

	log_warning(logPlan, esis);

	free(esis);

}






