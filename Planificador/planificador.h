#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <readline/readline.h> // Para usar readline
#include "../Recursos/estructuras.h"
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"
#include <semaphore.h>

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

t_log_level LogL;
t_log* logPlan;

//semáforos
sem_t* pausarPlanificacion;

typedef struct {
	int id;
	int fd;
	t_list* clavesTomadas;
	int rafagaEstimada;
	int tiempoEspera;
} t_proceso_esi;

typedef struct {
	int idProceso;
	char claveValor[LONGITUD_CLAVE];
	t_queue* colaBloqueados;
} t_clave;

char* planificador_Algoritmo;
char* coordinador_IP;
int estimacion_inicial;
int coordinador_Puerto;
int planificador_Puerto_Escucha;
char** claves_Ini_Bloqueadas;

int fdMaxConexionesActivas;
fd_set fdConexiones;

t_list* listaKeys;
t_queue* colaListos;
t_queue* colaTerminados;

t_proceso_esi* esi_ejecutando;

void configureLogger();
void *esperarConexiones(void *args);
void cargar_configuracion();
void *esperarConexiones(void *args);
void *planificar(void *args);
t_proceso_esi* recibirNuevoESI(int idESI, int fd);
void esperarConexionesESIs(void* esperarConexion);
void moverAListos(t_proceso_esi* procesoEsi);
void recibirMensajeCliente(int socketCliente);
void recibirMensajeEsi(int socketCliente);
void recibirMensajeCoordinador(int socketCliente);
int conectarCoordinador();

//funciones consola
void * iniciaConsola();
void exit_gracefully(int return_nr);
bool coincideValor(void*);
void block(char*, int);
void unblock(char*);


#endif
