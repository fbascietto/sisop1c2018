/*
 * funcionesESI.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

void cargar_configuracion(){

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

}
