#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
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



char* instanciaBusqueda;
int busquedaClave;

//semáforos
bool pausarPlanificacion;
pthread_mutex_t respuestaBusquedaClave;
pthread_mutex_t pausarPlanificacionSem;
sem_t productorConsumidor;
void inicializarSemaforos();
void destruirSemaforos();

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

//colas y listas
t_list* listaKeys;
t_queue* colaListos;
t_queue* colaTerminados;

//esi en ejecucion
t_proceso_esi* esi_ejecutando;
char* keySolicitada;

//funciones de colas
void inicializarColas();

//funciones de log y config
void configureLogger();
void cargar_configuracion();

//funciones sockets
void* escucharCoordinador(void* args);
void conectarCoordinador();
void *esperarConexiones(void *args);
t_proceso_esi* recibirNuevoESI(int idESI, int fd);
void* esperarConexionesESIs(void* esperarConexion);
bool recibirMensajeCliente(int socketCliente);
bool recibirMensajeEsi(int socketCliente);
void recibirInstancia(int socketCoordinador);

//funciones de planificacion
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
bool estaBloqueadoPorAlgunoDeLaCola(t_queue* bloqueadosPorEsi1, t_proceso_esi* esi1, t_proceso_esi* esiInterbloqueo);
bool estaBloqueado(t_proceso_esi* esi, t_clave* keyQueNecesita);



#endif
