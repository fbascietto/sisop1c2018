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
#define OBTENER_VALUE 63
#define COMPACTACION 64
#define EFECTUAR_DUMP 65
#define ENTRADAS_OCUPADAS 66
#define MANTENER_KEY 67
#define FIN 68


//Protocolo Planificador, Coordinador (75-100)
#define DONDE_ESTA_LA_CLAVE 75
#define CLAVE_ENCONTRADA 76
#define CLAVE_NO_ENCONTRADA 77
#define CLAVE_CREADA 78
#define OBTENER_VALOR_DE_KEY 79
#define CREAR_KEY_INICIALMENTE_BLOQUEADA 80
#define NO_HAY_INSTANCIAS_CONECTADAS 81


#define CLAVE_OTORGADA 90 //respuesta de GET
#define CLAVE_RESERVADA 92 //respuesta de SET
#define CLAVE_LIBERADA 93 //respuesta de STORE
#define CLAVE_NO_RESERVADA 95 //respuesta de SET y STORE


//Protocolo de Algoritmos de la instancia
#define CIRCULAR 101
#define LRU 102
#define BSU 103


#endif /* PROTOCOLO_H_ */
