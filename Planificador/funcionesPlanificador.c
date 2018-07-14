#include "planificador.h"

void* escucharCoordinador(void* args){
	while(1){
		int mensaje;
		char* keyBuscada;
		t_clave* claveObtenida = NULL;
		recibirInt(socketCoordinador,&mensaje);
		//		log_trace(logPlan, "recibi mensaje del coordinador, recibi un %d", mensaje);

		switch(mensaje){
		case GET_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL){
				claveObtenida = crearNuevaKey(keyBuscada);
				asignarKey(claveObtenida, esi_ejecutando);
				enviarInt(socketCoordinador, CLAVE_OTORGADA);
			} else if(estaLibre(claveObtenida)){
				asignarKey(claveObtenida, esi_ejecutando);
				enviarInt(socketCoordinador, CLAVE_OTORGADA);
			}
			else{
				strncpy(keySolicitada, keyBuscada, LONGITUD_CLAVE); //keySolicitada se usa para bloquear la clave cuando reciba el mensaje del esi
				enviarInt(socketCoordinador, CLAVE_BLOQUEADA);
			}
			free(keyBuscada);
			break;
		case SET_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL){
				enviarInt(socketCoordinador, CLAVE_INEXISTENTE);
			}else if(claveObtenida->esi_poseedor == NULL){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			}else if(claveObtenida->esi_poseedor->id != esi_ejecutando->id){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			} else {
				enviarInt(socketCoordinador, CLAVE_RESERVADA);
			}
			free(keyBuscada);
			break;
		case STORE_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL){
				enviarInt(socketCoordinador, CLAVE_INEXISTENTE);
			} else if(claveObtenida->esi_poseedor == NULL){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			}else if(claveObtenida->esi_poseedor->id != esi_ejecutando->id){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			} else {
				liberarKey(claveObtenida);
				enviarInt(socketCoordinador, CLAVE_LIBERADA);
			}
			free(keyBuscada);
			break;
		}
	}
}

t_clave* crearNuevaKey(char* clave){
	t_clave* nuevaKey = malloc(sizeof(t_clave));
	strcpy(nuevaKey->nombre, clave);
	nuevaKey->colaBloqueados = queue_create();
	nuevaKey->esi_poseedor = NULL;
	list_add(listaKeys,nuevaKey);
	return nuevaKey;
}

void asignarKey(t_clave* clave,t_proceso_esi* esi){
	list_add(esi->clavesTomadas, clave);
	clave->esi_poseedor = esi;
}

bool estaLibre(t_clave* clave){
	return clave->esi_poseedor == NULL;
}

bool estaTomada(void* key){
	t_clave* clave = (t_clave*) key;
	return clave->esi_poseedor != NULL;
}

void esperar() {
	if (pausarPlanificacion) {
		pthread_mutex_lock(&pausarPlanificacionSem);
	}
	if (comandoConsola) {
		pthread_mutex_unlock(&iniciarConsolaSem);
		pthread_mutex_lock(&esperarConsolaSem);
	}
	if(conexionEsi){
		pthread_mutex_unlock(&nuevoEsiSem);
		pthread_mutex_lock(&esperarNuevoEsiSem);
	}
}

//todo <--- no sacarlo, nunca encuentro esta funcion entre todas
void* planificar(void * args){

	bool replanificar = true;
	pthread_mutex_lock(&pausarPlanificacionSem);
	pthread_mutex_lock(&nuevoEsiSem);

	while(1){

		seQuitoUnEsiDeListos = false;
		sem_wait(&productorConsumidor);
		esperar();

		if(seQuitoUnEsiDeListos){

			seQuitoUnEsiDeListos = false;

		}else{

			actualizarColaListos();
			ordenarListos(); //sin esta funcion deberia funcionar como FIFO
			esi_ejecutando = queue_pop(colaListos); //agarra al primero de la lista
//			esi_ejecutando->rafagaActualPrevia = esi_ejecutando->rafagaActual;
				esi_ejecutando->rafagaActual=0;
			//			esi_ejecutando->tiempoEspera=0;
			//			esi_ejecutando->rafagaEstimada = esi_ejecutando->rafagaEstimadaSiguiente;
			log_info(logPlan, "esi %d enviado a ejecutar", esi_ejecutando->id);

			while(replanificar) {

				enviarAEjecutar(esi_ejecutando);
				replanificar = recibirMensajeEsi(esi_ejecutando->fd);
				esperar();
				//en caso de que se ejecute kill
				if(esi_ejecutando == NULL) break;

			}
			replanificar = true;
		}
	}
}

void ordenarListos(){

	t_list* unaLista = colaListos->elements;
	quick(unaLista, 0, queue_size(colaListos)-1);

	mostrarColaListos();
}


void mostrarColaListos(){
	log_warning(logPlan, "cola de listos de arriba a abajo");
	for(int i=0; i<queue_size(colaListos); i++){
		t_proceso_esi* esi = list_get(colaListos->elements, i);
		log_trace(logPlan, "esi %d con estimado n+1: %f", esi->id, esi->rafagaEstimadaSiguiente);
		log_trace(logPlan, "rafaga estimada n: %f", esi->rafagaEstimada);
		log_trace(logPlan, "tiempo espera: %d", esi->tiempoEspera);
		log_trace(logPlan, "rafaga actual: %d", esi->rafagaActual);
	}
}


// Swap de los esis
void swap(int x, int y, t_list* unaLista)
{
	t_proceso_esi* esiX = list_get(unaLista, x);
	t_proceso_esi* esiY = list_get(unaLista, y);

	list_replace(unaLista, x, esiY);
	list_replace(unaLista, y, esiX);
}

bool mejorEstimado(t_proceso_esi* esiA, t_proceso_esi* esiB){
	switch(planificador_Algoritmo){
	case HRRN:;
	return (esiA->rafagaEstimadaSiguiente > esiB->rafagaEstimadaSiguiente) || ((esiA->rafagaEstimadaSiguiente == esiB->rafagaEstimadaSiguiente) && (esiA->ordenLlegada < esiB->ordenLlegada));
	break;
	default:;
	return (esiA->rafagaEstimadaSiguiente < esiB->rafagaEstimadaSiguiente) || ((esiA->rafagaEstimadaSiguiente == esiB->rafagaEstimadaSiguiente) && (esiA->ordenLlegada < esiB->ordenLlegada));
	break;
	}
}

/* In this function last element is chosen as pivot,
   then elements are arranged such that,all elements
   smaller than pivot are arranged to left of pivot
   and all greater elements to right of pivot */
int partition(t_list* unaLista, int start, int end){
	t_proceso_esi* pivot = list_get(unaLista, end);    // choosing pivot element
	t_proceso_esi* esiAux;
	int pIndex = start;  // Index of first element
	int i;
	for (i=start; i<=end-1; i++){
		/* If current element is smaller than or
         equal to pivot then exchange it with element
         at pIndex and increment the pIndex*/
		esiAux = list_get(unaLista,i);
		if (mejorEstimado(esiAux, pivot)){
			swap(pIndex, i, unaLista);
			pIndex=pIndex+1;
		}
	}
	/*exchange pivot with pIndex at the completion
    of loop*/
	swap(pIndex, end, unaLista);
	return pIndex;
}

/* The main function that implements QuickSort
    A[] --> array to be sorted,
    start  --> Starting index,
    end  --> Ending index */
void quick(t_list* unaLista, int start, int end){
	if (start < end){
		/* p is pivot index after partitioning*/
		int p = partition(unaLista, start, end);
		// Recursively sort elements left of pivot
		// and elements right of pivot
		quick(unaLista, start, p-1);
		quick(unaLista, p+1, end);
	}
}




void actualizarColaListos(){
	if(planificador_Algoritmo == HRRN){
		list_iterate(colaListos->elements, cambiarEstimado);
	}
}


void agregarEspera(void* esi){
	t_proceso_esi* unEsi = (t_proceso_esi*) esi;
	unEsi->tiempoEspera++;
}

void cambiarEstimado(void* esi){
	t_proceso_esi* unEsi = (t_proceso_esi*) esi;
	switch(planificador_Algoritmo){
	case SJF_SIN_DESALOJO:
		unEsi -> rafagaEstimadaSiguiente = promedioExponencial(unEsi);
		break;
	case SJF_CON_DESALOJO:
		unEsi -> rafagaEstimadaSiguiente = promedioExponencial(unEsi);
		break;
	case HRRN:
		unEsi -> rafagaEstimadaSiguiente = estimacionHRRN(unEsi);
		break;
	}
}


float promedioExponencial(t_proceso_esi* unEsi){
	float actual = unEsi->rafagaActual * alfa;
	float estimado = unEsi->rafagaEstimada * (1.0 - alfa);
	//	log_info(logPlan, "promedio exponencial del esi %d", unEsi->id);
	//	log_info(logPlan, "(%d * %f) + (%f * (1.0 - %f) = %f", unEsi->rafagaActual, alfa, unEsi->rafagaEstimada, alfa, (actual + estimado));
	return actual + estimado;
}

float estimacionHRRN(t_proceso_esi* unEsi){
	float promedio = promedioExponencial(unEsi);
	float estimacion = 1.0 + (unEsi->tiempoEspera / promedio);
	//	log_info(logPlan, "estimacion HRRN: 1.0 + (%d / %f) = %f", unEsi->tiempoEspera, promedio, estimacion);
	return estimacion;
}

t_proceso_esi* recibirNuevoESI(int idESI, int fd){
	t_proceso_esi* nuevoProcesoESI = malloc(sizeof(t_proceso_esi));
	nuevoProcesoESI->id = idESI;
	nuevoProcesoESI->fd = fd;
	nuevoProcesoESI->clavesTomadas = list_create();
	nuevoProcesoESI->ordenLlegada = ordenDeLlegada++;
	nuevoProcesoESI -> rafagaActual = 0;
	nuevoProcesoESI ->tiempoEspera = 0;
	nuevoProcesoESI ->rafagaActualPrevia = 0;
	nuevoProcesoESI ->rafagaEstimada = estimacion_inicial;
	nuevoProcesoESI ->rafagaEstimadaSiguiente = estimacion_inicial;
	return nuevoProcesoESI;
}

void moverAListos(t_proceso_esi* procesoEsi){

	procesoEsi->rafagaEstimada = procesoEsi->rafagaEstimadaSiguiente;
	procesoEsi->ordenLlegada = ordenDeLlegada++;
	procesoEsi->tiempoEspera=0;

	cambiarEstimado(procesoEsi);
	queue_push(colaListos,procesoEsi);
	log_trace(logPlan, "ESI %d agregado a la cola de listos!", procesoEsi->id);
	sem_post(&productorConsumidor);
}

int enviarAEjecutar(t_proceso_esi* ESIMenorRafaga){
	esi_ejecutando = ESIMenorRafaga;
	int socketEsiEjectutando = esi_ejecutando->fd;
	int resultadoEnvio = enviarInt(socketEsiEjectutando, EJECUTAR_LINEA);
	return resultadoEnvio;
}

bool recibirMensajeEsi(int socketCliente){

	int mensaje;
	bool iterar = true;

	if(recibirInt(socketCliente, &mensaje) <= 0){
		finalizarESIEnEjecucion();
		iterar = false;
		return iterar;
	}

	esi_ejecutando->rafagaActual++;
	list_iterate(colaListos->elements, agregarEspera);
	log_trace(logPlan, "mensaje %d del ESI", mensaje);
	switch(mensaje){

	/*
	 * si la ejecucion fue correcta:
	 * 		1- se actualiza la rafaga de cpu del esi
	 * 		2- se actualiza la cola
	 * 		3.0- SOLO SI ES SJF CON DESALOJO
	 * 		3.1- se chequea si hay algun valor de estimado menor en cuyo caso se reemplaza
	 */
	case EJECUCION_OK:;

	if(planificador_Algoritmo == SJF_CON_DESALOJO){

		if(queue_size(colaListos)>0){

			//comentar esta funcion para que funcione como FIFO junto con enviarMejorESIAEjecutar()
			ordenarListos();
			t_proceso_esi* ESIMenorRafaga = queue_pop(colaListos);
			sem_wait(&productorConsumidor);

			if(esi_ejecutando->rafagaActualPrevia != 0){
				esi_ejecutando->rafagaActual += esi_ejecutando->rafagaActualPrevia;
				esi_ejecutando->rafagaActualPrevia=0;
			}


			if(ESIMenorRafaga->rafagaEstimadaSiguiente < (esi_ejecutando->rafagaEstimadaSiguiente - esi_ejecutando->rafagaActual)){
				log_warning(logPlan,"el ESI %d tiene menor rafaga que el esi %d, que esta en ejecucion", ESIMenorRafaga->id, esi_ejecutando->id);
				log_warning(logPlan,"intercambiando esis");

				esi_ejecutando->ordenLlegada = ordenDeLlegada++;
				esi_ejecutando->tiempoEspera=0;
				esi_ejecutando->rafagaActualPrevia = esi_ejecutando->rafagaActual;
				//				esi_ejecutando->rafagaEstimada = esi_ejecutando->rafagaEstimadaSiguiente;
				queue_push(colaListos,esi_ejecutando);
				log_trace(logPlan, "ESI %d agregado a la cola de listos!", esi_ejecutando->id);
				sem_post(&productorConsumidor);

				esi_ejecutando = ESIMenorRafaga;
				esi_ejecutando->rafagaActual=0;
			} else{
				sem_post(&productorConsumidor);
				queue_push(colaListos, ESIMenorRafaga); //es lo mismo donde se agregue porque despues se ordena la lista mas adelante
			}
		}
	}
	break;


	case EJECUCION_INVALIDA:;
	enviarInt(socketCliente, ABORTAR);
	finalizarESIEnEjecucion();
	iterar = false;
	break;

	case EN_ESPERA:;
	moverABloqueados();
	iterar = false;
	break;


	case FINALIZACION_OK:;
	liberarKeys(esi_ejecutando);
	finalizarESIEnEjecucion();
	iterar=false;
	break;

	default:;
	log_error(logPlan, "mensaje del ESI %d no reconocido, recibi %d. Abortando ESI", esi_ejecutando, mensaje);
	finalizarESIEnEjecucion();
	iterar = false;
	}

	return iterar;
}

void moverABloqueados(){
	t_proceso_esi* esi = esi_ejecutando;
	//	esi->tiempoEspera=0;
	block(keySolicitada, esi->id);
	esi_ejecutando = NULL;
}

void finalizarESI(t_proceso_esi* esi_terminado) {
	int socketESIFinalizado = esi_terminado->fd;
	FD_CLR(socketESIFinalizado, &fdConexiones);
	close(socketESIFinalizado);
	queue_push(colaTerminados, esi_terminado);
	log_info(logPlan, "ESI %d movido a la cola de terminados", esi_terminado->id);
}

void finalizarESIEnEjecucion(){
	t_proceso_esi* esi_terminado = esi_ejecutando;
	esi_ejecutando = NULL;
	finalizarESI(esi_terminado);
}

void conectarConsolaACoordinador(){

	socketConsolaCoordinador  = 0;

	int a = 3;

	while(1){

		socketConsolaCoordinador = conectarseA(coordinador_IP, coordinador_Puerto);

		if(socketConsolaCoordinador == 0){
			sleep(a++);
		}else{
			break;
		}
	}

	enviarInt(socketConsolaCoordinador, CONSOLA_PLANIFICADOR);

	log_info(logPlan, "Conectado consola con el Coordinador");
}

void conectarCoordinador(){

	socketCoordinador = 0;

	int a = 3;

	while(1){

		socketCoordinador = conectarseA(coordinador_IP, coordinador_Puerto);

		if(socketCoordinador == 0){
			sleep(a++);
		}else{
			break;
		}
	}

	enviarInt(socketCoordinador, PLANIFICADOR);

	log_info(logPlan, "Conectado con el Coordinador");
}

void* esperarConexionesClientes(void* esperarConexion){
	t_esperar_conexion* argumentos = (t_esperar_conexion*) esperarConexion;

	int cliente;

	while(1){

		int conexionNueva = esperarConexionesSocket(&argumentos->fdSocketEscucha, argumentos->socketEscucha);

		recibirInt(conexionNueva, &cliente);

		switch(cliente){

		case ESI:

			log_warning(logPlan, "Recibi un nuevo ESI!");

			int idESI;
			recibirInt(conexionNueva, &idESI);
			log_debug(logPlan, "ID del ESI %d", idESI);

			t_proceso_esi* nuevoESI = recibirNuevoESI(idESI, conexionNueva);
			log_debug(logPlan, "ESI creado!");

			if(pausarPlanificacion || queue_is_empty(colaListos)){
				queue_push(colaListos,nuevoESI);
				log_trace(logPlan, "ESI %d agregado a la cola de listos!", nuevoESI->id);
				sem_post(&productorConsumidor);
			}else{
				conexionEsi=true;
				pthread_mutex_lock(&nuevoEsiSem);
				queue_push(colaListos,nuevoESI);
				log_trace(logPlan, "ESI %d agregado a la cola de listos!", nuevoESI->id);
				sem_post(&productorConsumidor);
				conexionEsi=false;
				pthread_mutex_unlock(&esperarNuevoEsiSem);
			}

			//mutex con la funcion planificar()

			FD_SET(conexionNueva, &fdConexiones);
			if (conexionNueva > fdMaxConexionesActivas) {
				fdMaxConexionesActivas = conexionNueva;
			}

			break;


		}

	}

}

