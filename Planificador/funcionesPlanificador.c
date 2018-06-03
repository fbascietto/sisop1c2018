#include "planificador.h"

void* escucharCoordinador(void* args){
	while(1){
		int mensaje;
		recibirInt(socketCoordinador,&mensaje);
		switch(mensaje){
		case CLAVE_ENCONTRADA:
			busquedaClave = CLAVE_ENCONTRADA;
			recibirInstancia(socketCoordinador);
			break;
		case CLAVE_NO_ENCONTRADA:
			busquedaClave = CLAVE_NO_ENCONTRADA;
			recibirInstancia(socketCoordinador);
			break;
		}
	}
}

void recibirInstancia(int socketCoordinador){
	instanciaBusqueda = recibirMensajeArchivo(socketCoordinador);
	pthread_mutex_unlock(&respuestaBusquedaClave);
}

void* planificar(void * args){

	int socketMejorEsi = enviarMejorEsiAEjecutar();

	while(1){

		if(pausarPlanificacion){
			pthread_mutex_lock(&pausarPlanificacionSem);
		}

		bool replanificar = false;

		if(socketMejorEsi == 0){
			replanificar = true;
		}

		if(replanificar){
			socketMejorEsi = enviarMejorEsiAEjecutar();
		}

		while(replanificar) {
			replanificar = recibirMensajeCliente(socketMejorEsi);

		}


	}

}

void ordenarListos(){

	t_list* unaLista = colaListos->elements;

	quick(unaLista, 0, list_size(colaListos->elements)-1);

}

//algoritmo quicksort para ordenar listos
//pendiente testearlo
void quick(t_list* unaLista, int limite_izq, int limite_der){

	int izq,der;
	t_proceso_esi* pivote;

	izq= limite_izq;

	der = limite_der;

	pivote = list_get(unaLista, (izq+der)/2);

	t_proceso_esi* temporal;
	t_proceso_esi* esiDer;
	t_proceso_esi* esiIzq;

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

			temporal= list_get(unaLista,izq);

			list_replace(unaLista,izq,list_get(unaLista,der));
			list_replace(unaLista,der,temporal);

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
	return (promedio + unEsi ->tiempoEspera) / promedio;
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
	list_add(colaListos->elements,procesoEsi);
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

//TODO
int enviarMejorEsiAEjecutar(){

	if(list_size(colaListos->elements) > 0 ){
		actualizarColaListos();
		ordenarListos();
		esi_ejecutando = list_remove(colaListos->elements, 0);
		int socketEsiEjectutando = esi_ejecutando->fd;
		enviarInt(socketEsiEjectutando, EJECUTAR_LINEA);
		return socketEsiEjectutando;
	} else{
		return 0;
	}
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
		ordenarListos();
		t_proceso_esi* ESIMenorRafaga = list_remove(colaListos->elements, 0);
		if(ESIMenorRafaga->rafagaEstimada < esi_ejecutando->rafagaEstimada){
			moverAListos(esi_ejecutando);
			cambiarEstimado(esi_ejecutando);
			enviarMejorEsiAEjecutar(ESIMenorRafaga);
		} else{
			list_add_in_index(colaListos->elements, 0, ESIMenorRafaga);
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
	queue_push(colaTerminados, esi_terminado);
	liberarKeys(esi_terminado);


}

void recibirMensajeCoordinador(int socketCliente){
	int mensaje;
	recibirInt(socketCliente,&mensaje);
	switch(mensaje){
	//TODO
	}
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

}


void* esperarConexionesESIs(void* esperarConexion){
	t_esperar_conexion* argumentos = (t_esperar_conexion*) esperarConexion;

	while(1){
		int conexionNueva = esperarConexionesSocket(&argumentos->fdSocketEscucha, argumentos->socketEscucha);

		//TODO: chequear que la conexion fue correcta

		int idESI;
		recibirInt(conexionNueva, &idESI);
		t_proceso_esi* nuevoESI = recibirNuevoESI(idESI, conexionNueva);
		moverAListos(nuevoESI);

		//mutex con la funcion planificar()

		FD_SET(conexionNueva, &fdConexiones);
		if (conexionNueva > fdMaxConexionesActivas) {
			fdMaxConexionesActivas = conexionNueva;
		}
	}

}

