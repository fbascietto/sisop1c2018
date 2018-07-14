#ifndef FUNCIONESINSTANCIA_H_
#define FUNCIONESINSTANCIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <readline/readline.h> // Para usar readline
#include <pthread.h> // para el uso de threads
#include <unistd.h>
#include "../Recursos/estructuras.h"
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"
#include "../Biblioteca/file_cleaner.h"
#include <commons/collections/list.h>
#include <sys/stat.h> /* contiene las constantes para MKDIR */
#include <dirent.h>
#include <sys/mman.h> /* para el uso de MMAP */
#include <fcntl.h> /* para el uso de fallocate */
#include <errno.h>
#include <limits.h>

#define ROUNDUP(x,y) ((x - 1) / y + 1)
#define SIN_ENTRADA -10

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
t_bitarray* t_inst_bitmap;
pthread_t threadDump;
pthread_mutex_t mx_Dump;

/** Inicializacion **/
void cargar_configuracion();
void configureLoggers(char* name);
void destroyLoggers();
void inicializarPuntoMontaje(char * path, char * filename);
int reviewPuntoMontaje(t_list * whitelist);
int archivoAentrada(char* filename);
t_list* recibirClavesAMantener(int coordinador_socket);

/** Funciones de manejo de Entradas **/
int  almacenarEntrada(char * key, t_entrada * entrada, int largoValue);
void leer_entrada(t_entrada* entrada, char** value);
void escribirEntrada(char * escribir, int pos, char * nombre_archivo);
void eliminarEntrada(char * key);
int recibirEntrada(int socket);
bool obtenerEntrada(char *key,t_entrada ** entrada);
int calculoCantidadEntradas(int length);

/** Funciones de manejo de Key/Value **/
int recibirValue(int socketConn, char** bloqueArchivo);
int recibirKey(int socket, char** key);
int entregarValue(int socket);

/** Funciones STORE **/
int ejecutarStore(int coordinador_socket);
int persistir_clave(char *key);

/** Funciones de administracion de espacio **/
int algoritmoR(char* algoritmo);
int calcularSiguienteEntrada(int lenValue, t_entrada ** entrada, int socket);
int calculoCircular(int bloques, t_entrada ** entrada);
int calculoBSU(int bloques, t_entrada ** entrada);
int calculoLRU(int bloques, t_entrada ** entrada);

/** Compactación y dump **/
bool compactar();
void dump();

/** Funciones de bitmap **/
t_bitarray *crearBitmapVacio();
int findFreeBloque(t_bitarray* t_fs_bitmap);
int findNFreeBloques(t_bitarray* t_fs_bitmap, int n);
int cuentaBloquesLibre(t_bitarray* t_fs_bitmap);
int cuentaBloquesUsados(t_bitarray* t_fs_bitmap);
t_bitarray *limpiar_bitmap(char* nombre_Instancia, t_bitarray* bitmap);
void destruir_bitmap(t_bitarray* bitmap);


void close_gracefully();

#endif /* FUNCIONESINSTANCIA_H_ */
