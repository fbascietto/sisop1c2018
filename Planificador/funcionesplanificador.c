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
					//TODO: ver que se hace con el nuevo descriptor
					int newfd = aceptarConexion(listener);
					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
					}
				} else {
					//Es un mensaje de un cliente ya existente
					int identificadorMsg;
					recibirInt(socketIterado,&identificadorMsg);
					//TODO: Atender mensaje de ESI
				}
			}
		}
	}
}
