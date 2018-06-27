
#ifndef FUNCIONESINSTANCIA_H_
#define FUNCIONESINSTANCIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h> // Para usar readline
#include "../Recursos/estructuras.h"
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"
#include <commons/collections/list.h>
#include <sys/stat.h> /* contiene las constantes para MKDIR */
#include <sys/mman.h> /* para el uso de MMAP */
#include <fcntl.h> /* para el uso de fallocate */
#include <errno.h>
#include <limits.h>


typedef struct {
	char key[LONGITUD_CLAVE];
	int entry;
	int size;
	int ultimaRef;

} t_entrada;

t_list * tablaEntradas;
char* coordinador_IP;
int coordinador_Puerto;
int reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;
int qEntradas;
int tamanioEntrada;
int numEntradaActual;
t_log_level T;
t_log_level I;
t_log_level E;
t_log* logT;
t_log* logI;
t_log* logE;
int operacionNumero;

void eliminarEntrada(char * key);

int  almacenarEntrada(char key[LONGITUD_CLAVE], int entradaInicial, int largoValue);
void leer_entrada(t_entrada* entrada, char* value);
int escribirEntrada(char * escribir);
void inicializarPuntoMontaje(char * path, char * filename);
void cargar_configuracion();
void configureLoggers(char* name);
void destroyLoggers();
int ejecutarStore(int coordinador_socket);
int recibirValue(int socketConn, char** bloqueArchivo);
int recibirEntrada(int socket);
int recibirKey(int socket, char key [LONGITUD_CLAVE]);
int persistir_clave(char key[LONGITUD_CLAVE]);
int algoritmoR(char* algoritmo);
void calcularSiguienteEntrada(int lenValue);
int calculoCircular(int lenValue);
int obtenerCantidadEntradasOcupadas();
int obtenerCantidadEntradasLibres();
bool obtenerEntrada(char key[LONGITUD_CLAVE],t_entrada ** entrada);
int calculoCantidadEntradas(int length);

void close_gracefully();

#endif /* FUNCIONESINSTANCIA_H_ */
