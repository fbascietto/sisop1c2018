#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionesplanificador.h"

void *planificar(void *args1){

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args1;
	fd_set read_fds;
	fd_set* conexionesActivas = &fdConexiones;

	int fdmaxHelper;

	for(;;) {



		//mutex con la funcion esperarConexionesESIs()
		fdmaxHelper = fdmax;
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
}

t_proceso_esi* recibirNuevoESI(int idESI, int fd){
	t_proceso_esi* nuevoProcesoESI = malloc(sizeof(t_proceso_esi));
	nuevoProcesoESI->id = idESI;
	nuevoProcesoESI->fd = fd;
	nuevoProcesoESI->clavesTomadas = queue_create();
	//TODO: setear rafaga estimado de archivo de config
	return nuevoProcesoESI;
}

void moverAListos(t_proceso_esi* procesoEsi){
	queue_push(colaListos,procesoEsi);
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

void recibirMensajeEsi(int socketCliente){
	int mensaje;
	recibirInt(socketCliente,&mensaje);
	switch(mensaje){
	case EJECUCION_OK:;
	//TODO:
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

int* conectarCoordinador(){
	int socketCoordinador = conectarseA(coordinador_IP, coordinador_Puerto);
	FD_SET(socketCoordinador, &fdConexiones);
	return &socketCoordinador;
}


void esperarConexionesESIs(void* esperarConexion, void* socketCoordinador){
	t_esperar_conexion argumentos = (t_esperar_conexion)* esperarConexion;
	&fdmax = (int*) socketCoordinador;
	while(1){
		int conexionNueva = esperarConexionesSocket(argumentos->fdSocketEscucha, *fdmax);

		//TODO: chequear que la conexion fue correcta

		int idESI;
		recibirInt(conexionNueva, &idESI);
		t_proceso_esi* nuevoESI = recibirNuevoESI(idESI, conexionNueva);
		moverAListos(nuevoESI);

		//mutex con la funcion planificar()

		FD_SET(conexionNueva, &fdConexiones);
		if (conexionNueva > *fdmax) {
			*fdmax = conexionNueva;
		}
	}

}

