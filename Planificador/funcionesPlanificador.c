#include "planificador.h"

void* escucharCoordinador(void* args){
	while(1){
		int mensaje;
		char* keyBuscada;
		t_clave* claveObtenida = NULL;
		recibirInt(socketCoordinador,&mensaje);
		log_trace(logPlan, "recibi mensaje del coordinador, recibi un %d", mensaje);

		switch(mensaje){
		case CLAVE_ENCONTRADA:
			busquedaClave = CLAVE_ENCONTRADA;
			recibirInstancia(socketCoordinador);
			break;
		case CLAVE_NO_ENCONTRADA:
			busquedaClave = CLAVE_NO_ENCONTRADA;
			recibirInstancia(socketCoordinador);
			break;
		case GET_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL || estaLibre(claveObtenida)){
				claveObtenida = crearNuevaKey(keyBuscada);
				asignarKey(claveObtenida, esi_ejecutando);
				enviarInt(socketCoordinador, CLAVE_OTORGADA);
			} else{
				strncpy(keySolicitada, keyBuscada, LONGITUD_CLAVE);
				enviarInt(socketCoordinador, CLAVE_BLOQUEADA);
			}
			break;
		case SET_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL){
				enviarInt(socketCoordinador, CLAVE_INEXISTENTE);
			} else if(claveObtenida->idProceso != esi_ejecutando->id){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			} else {
				enviarInt(socketCoordinador, CLAVE_RESERVADA);
			}
			break;
		case STORE_KEY:
			keyBuscada =recibirMensajeArchivo(socketCoordinador);
			claveObtenida = obtenerKey(keyBuscada);
			if(claveObtenida == NULL){
				enviarInt(socketCoordinador, CLAVE_INEXISTENTE);
			} else if(claveObtenida->idProceso != esi_ejecutando->id){
				enviarInt(socketCoordinador, CLAVE_NO_RESERVADA);
			} else {
				liberarKey(claveObtenida);
				enviarInt(socketCoordinador, CLAVE_LIBERADA);
			}
			break;
	}
}
}

t_clave* crearNuevaKey(char* clave){
	t_clave* nuevaKey = malloc(sizeof(t_clave));
	strcpy(nuevaKey->claveValor, clave);
	queue_create(nuevaKey->colaBloqueados);
	nuevaKey->idProceso=0;
	return nuevaKey;
}

void asignarKey(t_clave* clave,t_proceso_esi* esi){
	clave->idProceso = esi->id;
}

void recibirInstancia(int socketCoordinador){
	instanciaBusqueda = recibirMensajeArchivo(socketCoordinador);
	pthread_mutex_unlock(&respuestaBusquedaClave);
}

bool estaLibre(t_clave* clave){
	return clave->idProceso == 0;
}

void* planificar(void * args){


	int socketMejorEsi;
	bool replanificar = true;

	while(1){

		if(pausarPlanificacion){
			pthread_mutex_lock(&pausarPlanificacionSem);
		}


		sem_wait(&productorConsumidor);

		socketMejorEsi = enviarMejorEsiAEjecutar();

		while(replanificar) {
			replanificar = recibirMensajeCliente(socketMejorEsi);
			if(replanificar == true){
				enviarAEjecutar(esi_ejecutando);
			}
		}

		replanificar = true;

	}

}

void ordenarListos(){

	t_list* unaLista = colaListos->elements;

	quick(unaLista, 0, queue_size(colaListos)-1);

}

//algoritmo quicksort para ordenar listos
//pendiente testearlo
void quick(t_list* unaLista, int limite_izq, int limite_der){

	int izq,der;
	t_proceso_esi* pivote;

	izq= limite_izq;

	der = limite_der;

	pivote = list_get(unaLista, (izq+der)/2);

	t_proceso_esi* esiDer = list_get(unaLista, der);
	t_proceso_esi* esiIzq = list_get(unaLista, izq);

	do{

		while((esiIzq->rafagaEstimada < pivote->rafagaEstimada) && izq<limite_der){
			izq++;
			esiIzq = list_get(unaLista,izq);
		}

		while((pivote->rafagaEstimada < esiDer->rafagaEstimada) && der > limite_izq){
			der--;
			esiDer = list_get(unaLista, der);
		}

		if(izq <=der){

			esiIzq = list_replace(unaLista,izq,list_get(unaLista,der));
			list_replace(unaLista,der,esiIzq);

			izq++;
			der--;

		}

	}while(izq<=der);

	if(limite_izq<der){quick(unaLista,limite_izq,der);}

	if(limite_der>izq){quick(unaLista,izq,limite_der);}
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

bool recibirMensajeCliente(int socketCliente){
	int cliente;
	bool iterar = true;
	recibirInt(socketCliente,&cliente);
	switch(cliente){
	case ESI:
		iterar = recibirMensajeEsi(socketCliente);
		break;
	}
	return iterar;
}

/*Se asume que a esta funcion se llama cuando al menos hay un elemento*/
int enviarMejorEsiAEjecutar(){
	actualizarColaListos();
	//si la funcion de abajo esta comentada, deberia funcionar como FIFO
	//ordenarListos();
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
	recibirInt(socketCliente,&mensaje);
	switch(mensaje){

	/*
	 * si la ejecucion fue correcta:
	 * 		1- se actualiza la rafaga de cpu del esi
	 * 		2- se actualiza la cola
	 * 		3.0- SOLO SI ES SJF CON DESALOJO
	 * 		3.1- se chequea si hay algun valor de estimado menor en cuyo caso se reemplaza
	 */
	case EJECUCION_OK:;
	esi_ejecutando ->rafagaActual++;
	actualizarColaListos();
	if(planificador_Algoritmo == SJF_CON_DESALOJO){
		if(queue_size(colaListos)>0){
		//comentar esta funcion para que funcione como FIFO junto con enviarMejorESIAEjecutar()
		//ordenarListos();
		t_proceso_esi* ESIMenorRafaga = queue_pop(colaListos);
		if(ESIMenorRafaga == NULL){
			break; //osea, que no hay elementos en listos
		}
		sem_wait(&productorConsumidor);
		if(ESIMenorRafaga->rafagaEstimada < esi_ejecutando->rafagaEstimada){
			moverAListos(esi_ejecutando);
			cambiarEstimado(esi_ejecutando);
			enviarAEjecutar(ESIMenorRafaga);
		} else{
			sem_post(&productorConsumidor);
			queue_push(colaListos, ESIMenorRafaga);
		}
		}
	}
	break;


	//TODO:
	case EJECUCION_INVALIDA:;
	finalizarESIEnEjecucion();
	iterar = false;
	break;

	//TODO
	case EN_ESPERA:;
	moverABloqueados();
	iterar = false;
	break;

	}

	return iterar;
}

void moverABloqueados(){
	block(keySolicitada, esi_ejecutando->id);
}

void finalizarESIEnEjecucion(){
	t_proceso_esi* esi_terminado = esi_ejecutando;
	//TODO desconexion del fd_set del esi
	int socketESIFinalizado = (esi_terminado->fd);
	FD_CLR(socketESIFinalizado, &fdConexiones);

	enviarInt(esi_ejecutando->fd, ABORTAR);
	queue_push(colaTerminados, esi_terminado);
	liberarKeys(esi_terminado);
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


void* esperarConexionesESIs(void* esperarConexion){
	t_esperar_conexion* argumentos = (t_esperar_conexion*) esperarConexion;

	while(1){

		int conexionNueva = esperarConexionesSocket(&argumentos->fdSocketEscucha, argumentos->socketEscucha);

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
	}

}

