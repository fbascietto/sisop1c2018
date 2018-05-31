/*
 * algoritmos.c
 *
 *  Created on: Apr 26, 2018
 *      Author: utnso
 */
#include "funcionesCoordinador.h"


void equitativeLoad(t_solicitud una_solicitud){
	t_instancia* instancia = list_get(instancias, proxima_posicion_instancia);
	despachar_solicitud(instancia, una_solicitud);
	proxima_posicion_instancia = siguiente(proxima_posicion_instancia, list_size(instancias));
}

int siguiente(int posicion_puntero, int tamanio){
	if((++posicion_puntero) < tamanio){
		return posicion_puntero;
	} else{
		return 0;
	}
}

void despachar_solicitud(t_instancia* instancia, t_solicitud una_solicitud){
	//TODO: implementar
}
