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
#include "funcionesCoordinador.h"

t_list* instancias;
//para equitative load, posicion del puntero en la lista
int posicion_puntero;

#endif /* COORDINADOR_H_ */
