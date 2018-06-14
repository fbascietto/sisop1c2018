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
#define PLANIFICADOR 4
#define CONSOLA_PLANIFICADOR 5 //El planificador tendra 2 sockets de conexion con Coordinador, uno especifico para la consola.

//Protocolo de Mensajes ESI,Planificador (11-25)

#define EJECUTAR_LINEA 11
#define ABORTAR 12
#define EJECUCION_OK 13
#define EN_ESPERA 14
#define EJECUCION_INVALIDA 15 //tambien la usa ESI, Coordinador
#define FINALIZACION_OK 16

//Protocolo de Mensajes ESI,Coordinador (26-40)

#define GET_KEY 26
#define SET_KEY 27
#define STORE_KEY 28
#define CLAVE_LIBRE 29
#define CLAVE_BLOQUEADA 30
#define CLAVE_INEXISTENTE 31
#define ERROR_EJECUCION 32

//Protocolo de tamaños
#define LONGITUD_CLAVE 41

//Protocolo de Algoritmos del Planificador
#define SJF_SIN_DESALOJO 51
#define SJF_CON_DESALOJO 52
#define HRRN 53

//Protocolo de Mensajes Coordinador/Instancia (61-75)
#define ENVIO_ENTRADA 61
#define STORE_ENTRADA 62

//Protocolo Planificador, Coordinador (75-100)
#define DONDE_ESTA_LA_CLAVE 75
#define CLAVE_ENCONTRADA 76
#define CLAVE_NO_ENCONTRADA 77

#define GET_KEY 80
#define SET_KEY 81
#define STORE_KEY 82
#define CLAVE_OTORGADA 90 //respuesta de GET
#define CLAVE_BLOQUEADA 91 //respuesta de GET
#define CLAVE_RESERVADA 92 //respuesta de SET
#define CLAVE_LIBERADA 93 //respuesta de STORE
#define CLAVE_INEXISTENTE 94 //respuesta de SET y STORE
#define CLAVE_NO_RESERVADA 95 //respuesta de SET y STORE


//Protocolo de Algoritmos de la instancia
#define CIRCULAR 76
#define LRU 77
#define BSU 78


#endif /* PROTOCOLO_H_ */
