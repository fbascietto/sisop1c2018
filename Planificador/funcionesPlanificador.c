#include "planificador.h"


void planificar(void *args1){

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args1;
	fd_set read_fds;
	fd_set* conexionesActivas = &fdConexiones;

	int fdmaxHelper;
	bool iterar;
	for(;;) {

		actualizarColaListos();
		ordenarListos();

		enviarEsiAEjecutar((t_proceso_esi*) list_get(colaListos, 0));

		//mutex con la funcion esperarConexionesESIs()
		fdmaxHelper = fdMaxConexionesActivas;
		read_fds = fdConexiones; // copiar fd_set
		// fin mutex con la funcion esperarConexionesESIs()

		//TODO: struct timeval para el ultimo parametro del select
		if (select(fdmaxHelper+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		int socketIterado;
		// run through the existing connections looking for data to read

		for(socketIterado = 0; socketIterado <= fdmaxHelper; socketIterado++) {

			if (FD_ISSET(socketIterado, &read_fds)) { // we got one!!

			} else {
				//Es un mensaje de un cliente ya existente
				recibirMensajeCliente(socketIterado);

				}
			}
		}
	}

void ordenarListos(){

	t_list* unaLista = colaListos;

	quick(unaLista, 0, list_size(colaListos)-1);

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
	list_iterate(colaListos, cambiarEstimado);
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
	list_add(colaListos,procesoEsi);
}

void recibirMensajeCliente(int socketCliente){
	int cliente;
	recibirInt(socketCliente,&cliente);
	switch(cliente){
	case ESI:
		recibirMensajeEsi(socketCliente);
		break;
	case COORDINADOR:
		recibirMensajeCoordinador(socketCliente);
		break;
	}
}

//TODO
void enviarEsiAEjecutar(t_proceso_esi* ESIMenorRafaga){
	esi_ejecutando = ESIMenorRafaga;
	enviarInt(esi_ejecutando->fd, EJECUTAR_LINEA);
}

void recibirMensajeEsi(int socketCliente){
	int mensaje;
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
				t_proceso_esi* ESIMenorRafaga = list_remove(colaListos, 0);
				if(ESIMenorRafaga->rafagaEstimada < esi_ejecutando->rafagaEstimada){
					moverAListos(esi_ejecutando);
					cambiarEstimado(esi_ejecutando);
					enviarEsiAEjecutar(ESIMenorRafaga);
				} else{
					list_add_in_index(colaListos, 0, ESIMenorRafaga);
				}
			}
	break;


	case EJECUCION_INVALIDA:;
	//TODO:
	break;
	case EN_ESPERA:;
	//TODO:
	break;
	}

}

void recibirMensajeCoordinador(int socketCliente){
	int mensaje;
	recibirInt(socketCliente,&mensaje);
	switch(mensaje){
	//TODO
	}
}

int conectarCoordinador(){
	int socketCoordinador = conectarseA(coordinador_IP, coordinador_Puerto);
	FD_SET(socketCoordinador, &fdConexiones);
	return socketCoordinador;
}


void esperarConexionesESIs(void* esperarConexion){
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

