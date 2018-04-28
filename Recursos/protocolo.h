/*
 * protocolo.h
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

//Protocolo de procesos (1-10)
#define ESI 1
#define COORDINADOR 2
#define INSTANCIA 3

//Protocolo de Mensajes ESI,Planificador (11-25)

#define EJECUTAR_LINEA 11
#define ABORTAR 12
#define EJECUCION_OK 13
#define EN_ESPERA 14
#define EJECUCION_INVALIDA 15 //tambien la usa ESI, Coordinador

//Protocolo de Mensajes ESI,Coordinador (26-40)

#define GET_KEY 26
#define SET_KEY 27
#define STORE_KEY 28
#define CLAVE_LIBRE 29
#define CLAVE_BLOQUEADA 30
#define CLAVE_INEXISTENTE 31


#endif /* PROTOCOLO_H_ */
