#include "planificador.h"

void* escucharCoordinador(void* args){
	while(1){
		int mensaje;
		char* keyBuscada;
		t_clave* claveObtenida = NULL;
		recibirInt(socketCoordinador,&mensaje);
		log_trace(logPlan, "recibi mensaje del coordinador, recibi un %d", mensaje);

		switch(mensaje){
		case GET_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL || estaLibre(claveObtenida)){
				claveObtenida = crearNuevaKey(keyBuscada);
				asignarKey(claveObtenida, esi_ejecutando);
				enviarInt(socketCoordinador, CLAVE_OTORGADA);
			} else{
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
			} else if(claveObtenida->esi_poseedor->id != esi_ejecutando->id){
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
			} else if(claveObtenida->esi_poseedor->id != esi_ejecutando->id){
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
	strcpy(nuevaKey->claveValor, clave);
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

void esperar() {
	if (pausarPlanificacion) {
		pthread_mutex_lock(&pausarPlanificacionSem);
	}
	if (comandoConsola) {
		pthread_mutex_unlock(&iniciarConsolaSem);
		pthread_mutex_lock(&esperarConsolaSem);
	}
}

void* planificar(void * args){

	bool replanificar = true;
	pthread_mutex_lock(&pausarPlanificacionSem);

	while(1){

		sem_wait(&productorConsumidor);

		while(!queue_is_empty(colaListos)){

			esperar();
			actualizarColaListos();
			ordenarListos(); //sin esta funcion deberia funcionar como FIFO
			esi_ejecutando = queue_pop(colaListos);
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

}

// Swap de los esis
void swap(int x, int y, t_list* unaLista)
{
	t_proceso_esi* esiX = list_get(unaLista, x);
	t_proceso_esi* esiY = list_get(unaLista, y);

	list_replace(unaLista, x, esiY);
	list_replace(unaLista, y, esiX);
}

/* In this function last element is chosen as pivot,
   then elements are arranged such that,all elements
   smaller than pivot are arranged to left of pivot
   and all greater elements to right of pivot */
int partition(t_list* unaLista, int start, int end)
{
	t_proceso_esi* pivot = list_get(unaLista, end);    // choosing pivot element
	t_proceso_esi* esiAux;
	int pIndex = start;  // Index of first element
	int i;
	for (i=start; i<=end-1; i++)
	{
		/* If current element is smaller than or
         equal to pivot then exchange it with element
         at pIndex and increment the pIndex*/
		esiAux = list_get(unaLista,i);
		if (esiAux->rafagaEstimada <= pivot->rafagaEstimada)
		{
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
	if (start < end)
	{
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


void cambiarEstimado(void* esi){
	t_proceso_esi* unEsi = (t_proceso_esi*) esi;
	switch(planificador_Algoritmo){
	case SJF_SIN_DESALOJO:
		unEsi -> rafagaEstimada = promedioExponencial(unEsi);
		break;
	case SJF_CON_DESALOJO:
		unEsi -> rafagaEstimada = promedioExponencial(unEsi);
		break;
	case HRRN:
		unEsi -> rafagaEstimada = estimacionHRRN(unEsi);
		break;
	}
}


int promedioExponencial(t_proceso_esi* unEsi){
	return (unEsi->rafagaActual * estimacion_inicial) + (unEsi->rafagaEstimada * (1 - estimacion_inicial));
}

int estimacionHRRN(t_proceso_esi* unEsi){
	int promedio = promedioExponencial(unEsi);
	return (promedio + unEsi ->tiempoEspera + 1) / promedio;
}

t_proceso_esi* recibirNuevoESI(int idESI, int fd){
	t_proceso_esi* nuevoProcesoESI = malloc(sizeof(t_proceso_esi));
	nuevoProcesoESI->id = idESI;
	nuevoProcesoESI->fd = fd;
	nuevoProcesoESI->clavesTomadas = list_create();
	nuevoProcesoESI -> rafagaActual = 0;
	nuevoProcesoESI ->tiempoEspera = 0;
	nuevoProcesoESI ->rafagaEstimada = estimacion_inicial;
	return nuevoProcesoESI;
}

void moverAListos(t_proceso_esi* procesoEsi){
	queue_push(colaListos,procesoEsi);
	log_trace(logPlan, "ESI %d agregado a la cola de listos!", procesoEsi->id);
	sem_post(&productorConsumidor);
}

/*Se asume que a esta funcion se llama cuando al menos hay un elemento*/
int enviarMejorEsiAEjecutar(){
	actualizarColaListos();
	//si la funcion de abajo esta comentada, deberia funcionar como FIFO
	ordenarListos();
	return enviarAEjecutar(queue_pop(colaListos));
}

int enviarAEjecutar(t_proceso_esi* ESIMenorRafaga){
	esi_ejecutando = ESIMenorRafaga;
	int socketEsiEjectutando = esi_ejecutando->fd;
	enviarInt(socketEsiEjectutando, EJECUTAR_LINEA);
	return socketEsiEjectutando;
}

bool recibirMensajeEsi(int socketCliente){

	int mensaje;
	bool iterar = true;
	recibirInt(socketCliente, &mensaje);
	log_trace(logPlan, "recibi %d", mensaje);


	switch(mensaje){

	case EJECUCION_OK:;
	/*
	 * si la ejecucion fue correcta:
	 * 		1- se actualiza la rafaga de cpu del esi
	 * 		2- se actualiza la cola
	 * 		3.0- SOLO SI ES SJF CON DESALOJO
	 * 		3.1- se chequea si hay algun valor de estimado menor en cuyo caso se reemplaza
	 */
	esi_ejecutando ->rafagaActual++;
	actualizarColaListos();

	if(planificador_Algoritmo == SJF_CON_DESALOJO){

		if(queue_size(colaListos)>0){

			//comentar esta funcion para que funcione como FIFO junto con enviarMejorESIAEjecutar()
			ordenarListos();
			t_proceso_esi* ESIMenorRafaga = queue_pop(colaListos);
			sem_wait(&productorConsumidor);

			if(ESIMenorRafaga->rafagaEstimada < esi_ejecutando->rafagaEstimada){
				moverAListos(esi_ejecutando);
				cambiarEstimado(esi_ejecutando);
				esi_ejecutando = ESIMenorRafaga;
			} else{
				sem_post(&productorConsumidor);
				queue_push(colaListos, ESIMenorRafaga);
			}
		}
	}
	break;


	//TODO:
	case EJECUCION_INVALIDA:;
	enviarInt(socketCliente, ABORTAR);
	finalizarESIEnEjecucion();
	iterar = false;
	break;

	//TODO
	case EN_ESPERA:;
	moverABloqueados();
	iterar = false;
	break;


	case FINALIZACION_OK:;
	finalizarESIEnEjecucion();
	iterar=false;
	break;
	}

	return iterar;
}

void moverABloqueados(){
	t_proceso_esi* esi = esi_ejecutando;
	block(keySolicitada, esi->id);
	esi_ejecutando = NULL;
}

void finalizarESIEnEjecucion(){
	t_proceso_esi* esi_terminado = esi_ejecutando;
	esi_ejecutando = NULL;
	//TODO desconexion del fd_set del esi
	int socketESIFinalizado = esi_terminado->fd;
	FD_CLR(socketESIFinalizado, &fdConexiones);
	//TODO hacer free
	//	queue_push(colaTerminados, esi_terminado);
	liberarKeys(esi_terminado);
	free(esi_terminado);
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

	log_trace(logPlan, "Conectado consola con el Coordinador");
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

	log_trace(logPlan, "Conectado con el Coordinador");
}

void* esperarConexionesClientes(void* esperarConexion){
	t_esperar_conexion* argumentos = (t_esperar_conexion*) esperarConexion;

	int cliente;

	while(1){

		int conexionNueva = esperarConexionesSocket(&argumentos->fdSocketEscucha, argumentos->socketEscucha);

		recibirInt(conexionNueva, &cliente);

		switch(cliente){

		case ESI:

			log_trace(logPlan, "Recibi un nuevo ESI!");
			//TODO: chequear que la conexion fue correcta

			int idESI;
			recibirInt(conexionNueva, &idESI);
			log_trace(logPlan, "ID del ESI %d", idESI);

			t_proceso_esi* nuevoESI = recibirNuevoESI(idESI, conexionNueva);
			log_trace(logPlan, "ESI creado!");
			moverAListos(nuevoESI);

			//mutex con la funcion planificar()

			FD_SET(conexionNueva, &fdConexiones);
			if (conexionNueva > fdMaxConexionesActivas) {
				fdMaxConexionesActivas = conexionNueva;
			}

			break;


		}

	}

}

