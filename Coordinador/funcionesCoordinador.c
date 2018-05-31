#include "funcionesCoordinador.h"
#include <readline/readline.h>


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
				simulaEntrada(nuevoSocket);
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
					instancia->claves = list_create();
					list_add(instancias,instancia);
					log_trace(logT,"Conexión de %s.", instancia->nombre);
					return 1;
}

void eliminarInstancia(t_instancia * instancia){
	free(instancia->nombre);
	list_destroy(instancia->claves);
	free(instancia);
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
			lenEnviado = send(socket, &key[totalEnviado], LONGITUD_CLAVE-totalEnviado, 0);
			if(lenEnviado < 0){ perror("error al enviar"); return -1;}
			totalEnviado = totalEnviado + lenEnviado;
		}
	return totalEnviado;
}

int enviarValue(char * value, int socket){
	return enviarMensaje(socket, value);
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

void atenderESI(void *args){
	t_argumentos_thESI * argumentos = (t_argumentos_thESI *) args;
	int socket = argumentos->socket;
}


int ejecutarOperacionGET(char* key){
	int pos = -1;
	t_instancia * instancia;
	pos = buscarInstanciaContenedora(key, instancia);
	if(pos<0){
		log_trace(logT,"No se encontro la clave %s en ninguna instancia", key);
		if(elegirInstancia(instancia)<0){
			log_trace(logE,"No se pudo guardar la clave %s en ninguna instancia", key);
			return -1;
		}
	}

	return bloquearKey(key);
}


int bloquearKey(char * key){
	int a = -1;
	a = list_add(claves_bloqueadas, key);
	log_trace(logT, "clave %s bloqueada", key);
	return a;
}

int elegirInstancia(t_instancia * instancia){

	if(strcmp(coordinador_Algoritmo,"EL")==0){
		if(proxima_posicion_instancia  >= list_size(instancias) ){
			proxima_posicion_instancia = 0;
		}
		instancia = list_get(instancias,proxima_posicion_instancia);
		log_trace(logT,"se eligio la instancia %s para el guardado de clave", instancia->nombre);
		return proxima_posicion_instancia++;
	}else
	if(strcmp(coordinador_Algoritmo,"LSU")==0){
		//TODO
	}else
	if(strcmp(coordinador_Algoritmo,"KE")==0){
		//TODO
	}


	return -1;
}

int buscarInstanciaContenedora(char* key, t_instancia * instancia){
	int pos = -1;
	int encontro = 0;
	bool* contieneClave(void* parametro) {
			t_instancia* inst = (t_instancia*) parametro;
			encontro = contieneClaveInstancia(inst,key);
			++pos;
			return (encontro);
	}

	instancia = list_find(instancias,contieneClave);
	if(encontro){
		return pos;
	}else{
		return -1;
	}

}

int contieneClaveInstancia(t_instancia * instancia, char* key){

	bool* contieneClave(void* parametro) {
				char* clave = (char*) parametro;
				return (strcmp(clave,key)==0);
		}

	return list_size(list_filter(instancia->claves,contieneClave(key)));
}

/**************** FUNCIONES PRUEBA ************************/
void simulaEntrada(int socket){
	char* linea;
	linea = readline("ComandoESI:" );
	char** parametros;
	parametros = string_split(linea, " ");

	char key[LONGITUD_CLAVE];
	strcpy(key,parametros[0]);
	/*
	 * parametro 0 = key
	 * parametro 1 = value
	 *
	 * */
	enviarKey(key,socket);
	enviarValue(parametros[1],socket);
}
