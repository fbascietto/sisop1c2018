#include "esi.h"

int main(int args, char* argv[]) {


	t_config* config = cargar_configuracion();

	configureLoggers();

	/* Descomentar mas tarde...*/
	if(args != 2){

		log_error(logE, "Cantidad de parametros incorrectos.");
		exit(0);

	}

	ruta_script_ejecuciones = argv[1];

	coordinador_socket = 0;
	planificador_socket = 0;

	//Intenta conectarse al coordinador
	int a = 3;
	while(1){

		coordinador_socket = conectarseA(coordinador_IP, coordinador_puerto);

		if(coordinador_socket != 0){
			break;
		}else{
			sleep(a++);
		}
	}

	log_info(logI, "Conectado con el coordinador");

	//protocolo (handshake)
	enviarInt(coordinador_socket, ESI);

	//recibe identificador de ESI para enviar a planificador
	recibirInt(coordinador_socket, &id_proceso);

	log_trace(logT, "ID del ESI: %d", id_proceso);

	a = 3;
	while(planificador_socket == 0){

		planificador_socket = conectarseA(planificador_IP, planificador_puerto);

		if(planificador_socket != 0){
			break;
		}else{
			sleep(a++);
		}
	}

	log_info(logI, "Conectado con el planificador");

	//	//protocolo (handshake)
	//	enviarInt(planificador_socket, ESI);

	//envia identificador de ESI al planificador
	enviarInt(planificador_socket, id_proceso);


	log_info(logI, "Comienzo a correr script");

	correrScript(ruta_script_ejecuciones);

	config_destroy(config);
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);

	return 0;

}
