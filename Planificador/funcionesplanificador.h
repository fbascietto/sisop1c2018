#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "planificador.h"

#ifndef FUNCIONESPLANIFICADOR_H_
#define FUNCIONESPLANIFICADOR_H_

void *planificar(void *args);
t_proceso_esi* recibirNuevoESI(int fd);
void moverAListos(t_proceso_esi* procesoEsi);
void recibirMensajeCliente(int socketCliente);
void recibirMensajeEsi(int socketCliente);
void recibirMensajeCoordinador(int socketCliente);

#endif

