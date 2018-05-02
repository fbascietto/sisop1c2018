/*
 * funcionesCoordinador.h
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#ifndef FUNCIONESCOORDINADOR_H_
#define FUNCIONESCOORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> // para el uso de threads
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../Recursos/estructuras.h"
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"
#include <commons/collections/list.h>


typedef struct {
	int socket;
} t_argumentos_thESI;

typedef struct {
	int id;
	char key[LONGITUD_CLAVE];
	char* value;
	char* accion;
} t_solicitud;

typedef struct {
	char* nombre;
	int socket;
	int * q_keys;

} t_instancia;


int coordinador_Puerto_Escucha;
char* coordinador_Algoritmo;
int cantidad_Entradas;
int tamanio_Entrada;
int retardo;
t_log_level T;
t_log_level I;
t_log_level E;
t_log* logT;
t_log* logI;
t_log* logE;

int posicion_puntero;
t_list * instancias;

void *esperarConexiones(void *args);
void cargarConfiguracion();
void despachar_solicitud(t_instancia* unaInstancia, t_solicitud una_solicitud);
void cargar_configuracion();
void configureLoggers();
void destroyLoggers();
void * atenderESI(void *args);
int crearInstancia(int nuevoSocket);
int enviarKey(char key[LONGITUD_CLAVE], int socket );
int enviarValue(char * value, int socket);
int enviarEntradaInstancia(char key[LONGITUD_CLAVE] , char * value, t_instancia * instancia);

#endif /* FUNCIONESCOORDINADOR_H_ */
