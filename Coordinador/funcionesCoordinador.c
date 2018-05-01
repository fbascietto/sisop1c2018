#include "funcionesCoordinador.h"

void *esperarConexiones(void *args) {

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args;

	t_argumentos_thESI * argsESI = malloc(sizeof(t_argumentos_thESI));

	printf("Esperando conexiones...\n");

	// ---------------ME QUEDO ESPERANDO UNA CONEXION NUEVA--------------

	while (1) {

		int nuevoSocket = -1;
		char* nombre;

		nuevoSocket = esperarConexionesSocket(&argumentos->fdSocketEscucha,argumentos->socketEscucha);

		if (nuevoSocket != -1) {
			// log_trace(logCoord,"Nuevo Socket!");
			printf("Nueva Conexion Recibida - Socket N°: %d\n",	nuevoSocket);

			int cliente;
			recibirInt(nuevoSocket,&cliente);

			switch(cliente){

			case ESI:
				// printf("ESI.\n");
				log_trace(logT,"Conexion de ESI.");
				pthread_t threadAtencionESI;
				argsESI->socket = nuevoSocket;
				if(pthread_create(&threadAtencionESI,NULL,atenderESI,(void*) argsESI)){
					log_error(logE,"Error generando thread para ESI");
				}
				/*
				if(pthread_join(threadAtencionESI, NULL)){
					log_error(logE,"Error uniendo thread para ESI");
				}*/
				break;

			case INSTANCIA:
				nombre = recibirMensajeArchivo(nuevoSocket);
				enviarInt(nuevoSocket,cantidad_Entradas);
				enviarInt(nuevoSocket,tamanio_Entrada);
				log_trace(logT,"Conexión de %s.", nombre);
				break;

			default:
				log_error(logE,"Mensaje no identificado.");
				break;
			}
		}
	}
}

void cargar_configuracion(){

	t_config* infoConfig;

	infoConfig = config_create("../Configuracion/coordinador.config");

	if(config_has_property(infoConfig, "PUERTO_ESCUCHA")){
		coordinador_Puerto_Escucha = config_get_int_value(infoConfig, "PUERTO_ESCUCHA");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		coordinador_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "CANTIDAD_ENTRADAS")){
		cantidad_Entradas = config_get_int_value(infoConfig, "CANTIDAD_ENTRADAS");
	}

	if(config_has_property(infoConfig, "TAMANIO_ENTRADA")){
		tamanio_Entrada = config_get_int_value(infoConfig, "TAMANIO_ENTRADA");
	}

	if(config_has_property(infoConfig, "RETARDO")){
		retardo = config_get_int_value(infoConfig, "RETARDO");
	}

}


void configureLoggers(){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;

	logT = log_create("../Logs/Coordinador.log", "Coordinador", false, T);
	logI = log_create("../Logs/Coordinador.log", "Coordinador", false, I);
	logE = log_create("../Logs/Coordinador.log", "Coordinador", true, E);
}

void destroyLoggers(){
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);
}

void *atenderESI(void *args){
	t_argumentos_thESI * argumentos = (t_argumentos_thESI *) args;
	int socket = argumentos->socket;



}
