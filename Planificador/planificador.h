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

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

t_log_level LogL;
t_log* logPlan;

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

int fdmax;
fd_set fdConexiones;

t_queue* colaListos;
t_queue* colaTerminados;

void configureLogger();
void *esperarConexiones(void *args);
void * iniciaConsola();
void cargar_configuracion();
void *esperarConexiones(void *args);
void exit_gracefully(int return_nr);
int* conectarCoordinador();


#endif
