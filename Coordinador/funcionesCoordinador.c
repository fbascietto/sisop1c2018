#include "funcionesCoordinador.h"

void *esperarConexiones(void *args) {

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args;

	t_argumentos_thESI * argsESI = malloc(sizeof(t_argumentos_thESI));

	printf("Esperando conexiones...\n");

	// ---------------ME QUEDO ESPERANDO UNA CONEXION NUEVA--------------

	while (1) {

		int nuevoSocket = -1;
		//char* nombre;

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
				//envia al esi su nro de socket para enviarle al planificador
				enviarInt(nuevoSocket, nuevoSocket);
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
				crearInstancia(nuevoSocket);
				break;

			default:
				log_error(logE,"Mensaje no identificado.");
				break;
			}
		}
	}
}

int crearInstancia(int nuevoSocket){
					t_instancia * instancia = malloc(sizeof(t_instancia));

					instancia->nombre = recibirMensajeArchivo(nuevoSocket);
					instancia->socket = nuevoSocket;
					if(enviarInt(nuevoSocket,cantidad_Entradas)<=0){
						return -1;
					}
					if(enviarInt(nuevoSocket,tamanio_Entrada)<=0){
						return -1;
					}

					list_add(instancias,instancia);
					log_trace(logT,"Conexión de %s.", instancia->nombre);
					return 1;
}

int enviarEntradaInstancia(char key[LONGITUD_CLAVE] , char * value, t_instancia * instancia){
	if(enviarKey(key,instancia->socket)<=0){
		log_error(logE,"no se pudo enviar entrada a la instancia %s",instancia->nombre);
		return -1;
	}
	if(enviarValue(value,instancia->socket)<=0){
		log_error(logE,"no se pudo enviar entrada a la instancia %s",instancia->nombre);
		return -1;
	}
	instancia->q_keys = instancia->q_keys +1;

	return 1;

}

int enviarKey(char key[LONGITUD_CLAVE], int socket ){
	int totalEnviado = 0;
	int lenEnviado = 0;
	while(totalEnviado < LONGITUD_CLAVE) {
			lenEnviado = 0;
			lenEnviado = send(socket, key[totalEnviado], LONGITUD_CLAVE-totalEnviado, 0);
			if(lenEnviado < 0){ perror("error al enviar\n"); return -1;}
			totalEnviado = totalEnviado + lenEnviado;
		}
	return totalEnviado;
}

int enviarValue(char * value, int socket){
	return enviarMensaje(value,socket);
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
