/*
 * funcionesInstancia.c
 *
 *  Created on: Apr 27, 2018
 *      Author: utnso
 */
#include "funcionesInstancia.h"

t_entrada * almacenarEntrada(char * key, char * value ){

	t_entrada * entrada = malloc(sizeof(t_entrada));

	entrada->key = key;
	entrada->value = value;

	list_add(entradas,entrada);

	return entrada;
}

void eliminarEntrada(char * key){
	bool* findByKey(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;
		return (strcmp(entrada->key,key));
	}
	t_entrada * entrada =(t_entrada *) list_remove_by_condition(entradas,eliminarEntrada);
	free(entrada->key);
	free(entrada->value);
	free(entrada);
}
