/*
 * funcionesInstancia.h
 *
 *  Created on: Apr 30, 2018
 *      Author: utnso
 */

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


typedef struct {
	char key[40];
	int entry;
	int size;
} t_entrada;

t_list * tablaEntradas;
char* coordinador_IP;
int coordinador_Puerto;
char* reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;
int qEntradas;
int tamanioEntrada;
t_log_level T;
t_log_level I;
t_log_level E;
t_log* logT;
t_log* logI;
t_log* logE;

void eliminarEntrada(char * key);
int  almacenarEntrada(char key[40], FILE* archivoDatos, void * value);
FILE* inicializarPuntoMontaje(char * path, char * filename);
void cargar_configuracion();
void configureLoggers(char* name);
void destroyLoggers();
int escribirEntrada(t_entrada * entrada, FILE* archivoDatos, int socketConn);
void * recibirValue(socketConn);

#endif /* FUNCIONESINSTANCIA_H_ */
