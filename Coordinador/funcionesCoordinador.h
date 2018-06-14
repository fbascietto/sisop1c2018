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
	int socketESI;
} t_argumentos_thESI;

typedef struct {
	int socketPlanificador;
} t_argumentos_thPlanificador;

typedef struct {
	int id;
	char key[LONGITUD_CLAVE];
	char* value;
	char* accion;
} t_solicitud;

typedef struct {
	char* nombre;
	int socketInstancia;
	t_list * claves;
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

FILE* logOp;

t_argumentos_thPlanificador * argsPlanificador;
t_argumentos_thPlanificador * argsConsolaPlanificador;
pthread_mutex_t mx_logOp;

int proxima_posicion_instancia;
t_list * instancias;

t_list * claves_sin_instancia;

void *esperarConexiones(void *args);
void cargarConfiguracion();
void despachar_solicitud(t_instancia* unaInstancia, t_solicitud una_solicitud);
void cargar_configuracion();
void configureLoggers();
void destroyLoggers();
void* atenderESI(void *args);
void atenderPlanificador(void *args);
void atenderConsolaPlanificador(void *args);
void recibirMensajeConsolaPlanificador(int socket);
void recibirMensajePlanificador(int socket);
int crearInstancia(int nuevoSocket);
void eliminarInstancia(t_instancia * instancia);
int enviarKey(char key[LONGITUD_CLAVE], int socket );
int enviarValue(char * value, int socket);
int enviarEntradaInstancia(char key[LONGITUD_CLAVE] , char * value, t_instancia * instancia);
int elegirInstancia(t_instancia * instancia);
int bloquearKey(char key[LONGITUD_CLAVE]);
void liberar_clave(char key[LONGITUD_CLAVE]);
int ejecutarOperacionGET(int socket);
int ejecutar_operacion_set(int socket);
int ejecutar_operacion_set_instancia(char key[LONGITUD_CLAVE], char * value, t_instancia * instancia);
int ejecutar_operacion_store(int socket);
int ejecutar_operacion_store_instancia(char key[LONGITUD_CLAVE], t_instancia * instancia);
int contieneClaveInstancia(t_instancia * instancia, char key[LONGITUD_CLAVE]);
int buscarInstanciaContenedora(char key[LONGITUD_CLAVE], t_instancia * instancia);
int simularBuscarInstanciaContenedora(char key[LONGITUD_CLAVE], t_instancia* instancia);
int elegirInstancia(t_instancia * instancia);
void recibirMensajeESI(int socket);
void generarlogDeOperaciones();
int logueaOperacion(char* operacion, char* key, char* value, int socket);
bool key_creada(char * key);



//funciones a borrar
void simulaEntrada(int socket);


#endif /* FUNCIONESCOORDINADOR_H_ */
