#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <readline/history.h>
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

#define ESI_IMPOSTOR -10

//structs
typedef struct {
	int id;
	int fd;
	t_list* clavesTomadas;
	int rafagaEstimada;
	int rafagaActual;
	int tiempoEspera;
} t_proceso_esi;

typedef struct {
	t_proceso_esi* esi_poseedor;
	char claveValor[LONGITUD_CLAVE];
	t_queue* colaBloqueados;
} t_clave;

//semáforos
bool pausarPlanificacion;
bool comandoConsola;
bool espera;
pthread_mutex_t pausarPlanificacionSem;
pthread_mutex_t iniciarConsolaSem;
pthread_mutex_t esperarConsolaSem;
sem_t productorConsumidor;
void inicializarSemaforos();
void destruirSemaforos();
void esperarPlanificador();
void continuarPlanificador();
void esperar();


//archivo de config y logs
int planificador_Algoritmo;
char* planificador;
char* coordinador_IP;
int estimacion_inicial;
int coordinador_Puerto;
int planificador_Puerto_Escucha;
char** claves_Ini_Bloqueadas;
t_log_level LogL;
t_log* logPlan;

//select y fds
int fdMaxConexionesActivas;
fd_set fdConexiones;
int socketCoordinador;
int socketConsolaCoordinador;

//colas y listas
t_list* listaKeys;
t_queue* colaListos;
t_queue* colaTerminados;

//esi en ejecucion
t_proceso_esi* esi_ejecutando;
char* keySolicitada;

//auxiliar para funcion de deadlock
bool encontroDeadlock;
bool yaImprimioDeadlock;

//funciones de colas
void inicializarColas();
void cargarKeysBloqueadasIniciales();

//funciones de log y config
void configureLogger();
void cargar_configuracion();

//funciones sockets
void* escucharCoordinador(void* args);
void conectarCoordinador();
void conectarConsolaACoordinador();
void *esperarConexiones(void *args);
t_proceso_esi* recibirNuevoESI(int idESI, int fd);
void* esperarConexionesClientes(void* esperarConexion);
bool recibirMensajeEsi(int socketCliente);

//funciones de planificacion
void 	iniciarVariablesGlobales();
void* planificar(void* args);
void moverAListos(t_proceso_esi* procesoEsi);
void ordenarListos();
void quick(t_list* unaLista, int limite_izq, int limite_der);
void actualizarColaListos();
void cambiarEstimado(void* unEsi);
int promedioExponencial(t_proceso_esi* unEsi);
int estimacionHRRN(t_proceso_esi* unEsi);
int enviarMejorEsiAEjecutar();
int enviarAEjecutar(t_proceso_esi* ESIMenorRafaga);
void finalizarESIEnEjecucion();
void moverABloqueados();

//funciones de keys
void liberarKeys(t_proceso_esi* esi);
void liberarKey(void* key);
t_clave* obtenerKey(char* key_value);
t_clave* obtenerKeySegunProcesoBloqueado(int esiID);
void asignarKey(t_clave* clave,t_proceso_esi* esi);
bool estaLibre(t_clave* clave);
t_clave* crearNuevaKey(char* clave);
bool estaTomada(t_clave* clave);


//funciones consola
void * iniciaConsola();
void exit_gracefully(int return_nr);
void block(char*, int);
void unblock(char*);
void pauseScheduler();
void goOn();
void getStatus(char* keySearch);
void listBlockedProcesses(char* keySearch);
void matarProceso(int ESI_ID);
void detectarDeadlock();


//funciones auxiliares
bool coincideID(int idP1, int idP2);
bool coincideValor(char*, char*);
bool coincideCola(t_queue*, int);
t_proceso_esi* removerEsiSegunID(t_list* procesos, int ID);
void verificarEsperaCircular(t_list* keys, t_list* procesosEnDeadlock);
void verificarEsperaCircularParaUnaKey(t_clave* key, t_list* procesosEnDeadlock);
t_list* obtenerKeysAsignadasDeUnProceso(t_proceso_esi* proceso);
bool estaLaKey(t_list* keys, t_clave* key);
void imprimirIDs(t_list* procesosEnDeadlock);
void agregarElementos(t_list* origen, t_list* destino);

//funciones mock
void testearDeadlock();


#endif
