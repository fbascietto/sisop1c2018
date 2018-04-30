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
#include <commons/collections/list.h>

typedef struct {
	char * key;
	char * value;
	int entry;
	int size;
} t_entrada;


t_list * entradas;


void eliminarEntrada(char * key);
t_entrada * almacenarEntrada(char * key, char * value );

#endif /* FUNCIONESINSTANCIA_H_ */
