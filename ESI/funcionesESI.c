/*
 * funcionesESI.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "esi.h"

t_config* cargar_configuracion(){

	t_config* infoConfig;

	/* para correr desde ECLIPSE
	infoConfig = config_create("../Recursos/Configuracion/esi.config");
	 */

	/* para correr desde CONSOLA*/
	infoConfig = config_create("../../Recursos/Configuracion/esi.config");


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

	/* para correr desde ECLIPSE *
	vaciarArchivo("../Recursos/Logs/ESI.log");
	logT = log_create("../Recursos/Logs/ESI.log","ESI", false, T);
	logI = log_create("../Recursos/Logs/ESI.log", "ESI", false, I);
	logE = log_create("../Recursos/Logs/ESI.log", "ESI", true, E);
	 */

	/* para correr desde CONSOLA */
	vaciarArchivo("../../Recursos/Logs/ESI.log");
	logT = log_create("../../Recursos/Logs/ESI.log","ESI", true, T);
	logI = log_create("../../Recursos/Logs/ESI.log", "ESI", true, I);
	logE = log_create("../../Recursos/Logs/ESI.log", "ESI", true, E);

}

void avisarAlPlanificador(int respuesta_del_coordinador, int position_to_reread, FILE* script){

	enviarInt(planificador_socket, ESI);

	switch(respuesta_del_coordinador){

	case CLAVE_LIBRE:

		enviarInt(planificador_socket, EJECUCION_OK);
		break;

	case CLAVE_BLOQUEADA:

		enviarInt(planificador_socket, EN_ESPERA);
		fseek(script, position_to_reread, SEEK_SET);
		break;

	case CLAVE_INEXISTENTE:

		enviarInt(planificador_socket, EJECUCION_INVALIDA);
		break;

	case EJECUCION_OK:

		enviarInt(planificador_socket, EJECUCION_OK);
		break;

	}


}

void correrScript(char* ruta){

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

	int position_before_read;

	while(1){

		log_info(logI, "Espero mensaje del planificador");

		recibirInt(planificador_socket, &accion);

		log_info(logI, "Recibi mensaje del planificador, recibi %d", accion);

		if(accion == EJECUTAR_LINEA){

			position_before_read = ftell(f1);
			fgets(linea, largo, f1);
			t_esi_operacion parsed = parse(linea);
			int respuesta;

			if(parsed.valido){
				switch(parsed.keyword){

				case GET:
					enviarInt(coordinador_socket, GET_KEY);
					enviarMensaje(coordinador_socket, parsed.argumentos.GET.clave);
					recibirInt(coordinador_socket, &respuesta);
					log_trace(logT, "recibi %d del coordinador", respuesta);
					avisarAlPlanificador(respuesta, position_before_read, f1);
					break;
				case SET:
					enviarInt(coordinador_socket, SET_KEY);
					enviarMensaje(coordinador_socket, parsed.argumentos.SET.clave);
					enviarMensaje(coordinador_socket, parsed.argumentos.SET.valor);
					recibirInt(coordinador_socket, &respuesta);
					log_trace(logT, "recibi %d del coordinador", respuesta);
					avisarAlPlanificador(respuesta, VALUE_NOT_USED, NULL);
					break;
				case STORE:
					enviarInt(coordinador_socket, STORE_KEY);
					enviarMensaje(coordinador_socket, parsed.argumentos.STORE.clave);
					recibirInt(coordinador_socket, &respuesta);
					log_trace(logT, "recibi %d del coordinador", respuesta);
					avisarAlPlanificador(respuesta, VALUE_NOT_USED, NULL);
					break;
				default:
					fprintf(stderr, "No pude interpretar <%s>\n", linea);
					exit(EXIT_FAILURE);

				}

			}

			destruir_operacion(parsed);
//			if(respuesta == CLAVE_INEXISTENTE){
//				break;
//			}

		}else{
			if(accion == ABORTAR){
				//avisarle al coordinador?
				break;
			}
		}
	}

	fclose(f1);
	free(linea);

}
