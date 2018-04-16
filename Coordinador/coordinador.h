#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h> // Para usar readline
#include "../Biblioteca/estructuras.h"
#include "../Biblioteca/protocolo.h"
#include "../Biblioteca/biblio_sockets.c"

typedef struct {
	int socketEscucha;
	fd_set fdSocketEscucha;
} t_esperar_conexion;

int coordinador_Puerto_Escucha;
char* coordinador_Algoritmo;
int cantidad_Entradas;
int tamanio_Entrada;
int retardo;

void *esperarConexiones(void *args);

#endif /* COORDINADOR_H_ */
