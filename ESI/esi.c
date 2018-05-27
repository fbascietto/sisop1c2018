#include "esi.h"

int main(int args, char* argv[]) {

	/* Descomentar mas tarde...
	  if(args != 2){

		log_error(error_log, "Cantidad de parametros incorrectos.");
		exit(0);

	}*/

	ruta_script_ejecuciones = argv[1];

	t_config* config = cargar_configuracion();

	configureLoggers();

	coordinador_socket = 0;
	planificador_socket = 0;

	//Intenta conectarse al coordinador

	while(coordinador_socket == 0){

		coordinador_socket = conectarseA(coordinador_IP, coordinador_puerto);

	}

	//protocolo (handshake)
	enviarInt(coordinador_socket, ESI);

	//recibe identificador de ESI para enviar a planificador
	recibirInt(coordinador_socket, &id_proceso);

	while(planificador_socket == 0){

		planificador_socket = conectarseA(planificador_IP, planificador_puerto);

	}

//	//protocolo (handshake)
//	enviarInt(planificador_socket, ESI);

	//envia identificador de ESI al planificador
	enviarInt(planificador_socket, id_proceso);

	correrScript(ruta_script_ejecuciones);

	config_destroy(config);
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);

	return 0;

}
