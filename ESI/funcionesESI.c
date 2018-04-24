/*
 * funcionesESI.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

t_config* cargar_configuracion(){

	t_config* infoConfig;

	infoConfig = config_create("../Configuracion/esi.config");

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		coordinador_IP = config_get_string_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		coordinador_puerto = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

	if(config_has_property(infoConfig, "IP_PLANIFICADOR")){
		planificador_IP = config_get_string_value(infoConfig, "IP_PLANIFICADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_PLANIFICADOR")){
		planificador_puerto = config_get_int_value(infoConfig, "PUERTO_PLANIFICADOR");
	}

	return infoConfig;

}

void configureLoggers(){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;
	logT = log_create("../Logs/ESI.log","ESI", false, T);
	logI = log_create("../Logs/ESI.log", "ESI", false, I);
	logE = log_create("../Logs/ESI.log", "ESI", true, E);

}

void leerScript(char* ruta){

	int accion;
	int largo;
	char* linea;

	FILE* f1 = fopen(ruta, "r");
	if(f1 == NULL){
		log_error(logE, "No existe el script con ruta %s", ruta);
		exit(0);
	}

	fseek(f1, 0, SEEK_END);
	largo = ftell(f1);

	rewind(f1);

	linea = malloc(largo);

	while(1){

		recibirInt(planificador_socket, &accion);

		switch(accion){

		case EJECUTAR_LINEA:

			fgets(linea, largo, f1);
			t_esi_operacion parsed = parse(linea);
			if(parsed.valido){
				switch(parsed.keyword){

				case GET:
					enviarInt(coordinador_socket, GET_KEY);
					//enviar la key
					break;
				case SET:
					enviarInt(coordinador_socket, SET_KEY);
					//enviar la key
					break;
				case STORE:
					enviarInt(coordinador_socket, STORE_KEY);
					//enviar la key
					break;
				default:
					fprintf(stderr, "No pude interpretar <%s>\n", linea);
					exit(EXIT_FAILURE);

				}

			}

		}


	}

	fclose(f1);
	free(linea);

}
