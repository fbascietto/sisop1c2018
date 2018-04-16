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

void leerScript(char* ruta){

	int accion;
	int largo;
	char* linea;

	FILE* f1 = fopen(ruta, "r");
	if(f1 == NULL){
		char* error = string_new();
		string_append_with_format(&error, "No existe el script con ruta %s", ruta);
		perror(error);
		free(error);
		exit(0);
	}

	fseek(f1, 0, SEEK_END);
	largo = ftell(f1);

	rewind(f1);

	linea = malloc(largo);

	while(1){

		fgets(linea, largo, f1);

		recibirInt(planificador_socket, &accion);

		//switch(accion){


		//}

	}

	fclose(f1);
	free(linea);

}
