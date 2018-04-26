#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <readline/readline.h> // Para usar readline
#include "../Recursos/estructuras.h"
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"

int coordinador_Puerto_Escucha;
char* coordinador_Algoritmo;
int cantidad_Entradas;
int tamanio_Entrada;
int retardo;

t_list* instancias;
//para equitative load, posicion del puntero en la lista
int posicion_puntero;


typedef struct {
	int id;
} t_instancia;

typedef struct {
	int id;
	char* key;
	char* clave;
	char* accion;
} t_solicitud;

void *esperarConexiones(void *args);
void cargarConfiguracion();
void despachar_solicitud(t_instancia* unaInstancia, t_solicitud una_solicitud);

#endif /* COORDINADOR_H_ */
