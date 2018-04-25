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

//Protocolo de Mensajes ESI, Coordinador (26-40)

#define GET_KEY 26
#define SET_KEY 27
#define STORE_KEY 28


#endif /* PROTOCOLO_H_ */
