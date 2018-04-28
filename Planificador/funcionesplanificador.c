#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionesplanificador.h"

void *planificar(void *args){

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args;
	int listener = argumentos->socketEscucha;
	// main loop
	fd_set master;
	fd_set read_fds;
	int fdmax;
	FD_SET(listener,&master);
	fdmax = listener;
	for(;;) {
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		int socketIterado;
		// run through the existing connections looking for data to read
		for(socketIterado = 0; socketIterado <= fdmax; socketIterado++) {
			if (FD_ISSET(socketIterado, &read_fds)) { // we got one!!
				if (socketIterado == listener) {
					//Es una conexion nueva
					int newfd = aceptarConexion(listener);
					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						t_proceso_esi* nuevoEsi = recibirNuevoESI(newfd);
						moverAListos(nuevoEsi);
					}
				} else {
					//Es un mensaje de un cliente ya existente
					recibirMensajeCliente(socketIterado);
				}
			}
		}
	}
}

t_proceso_esi* recibirNuevoESI(int fd){
	t_proceso_esi* nuevoProcesoESI = malloc(sizeof(t_proceso_esi));
	nuevoProcesoESI->id = fd;
	nuevoProcesoESI->claves = queue_create();
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
