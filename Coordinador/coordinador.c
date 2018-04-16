#include "coordinador.h"

int main(){
	cargar_configuracion();

	int socketEscucha;
	fd_set fdSocketsEscucha;

	FD_ZERO(&fdSocketsEscucha);
	socketEscucha = escuchar(coordinador_Puerto_Escucha);

	FD_SET(socketEscucha, &fdSocketsEscucha);
	t_esperar_conexion *esperarConexion;

	esperarConexion = malloc(sizeof(t_esperar_conexion));
	esperarConexion->fdSocketEscucha = fdSocketsEscucha;
	esperarConexion->socketEscucha = socketEscucha;

	esperarConexiones((void*) esperarConexion);
	return 0;
}

void *esperarConexiones(void *args) {

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args;
	printf("Esperando conexiones...\n");

	// ---------------ME QUEDO ESPERANDO UNA CONEXION NUEVA--------------
	while (1) {

		int nuevoSocket = -1;

		nuevoSocket = esperarConexionesSocket(&argumentos->fdSocketEscucha,argumentos->socketEscucha);

		if (nuevoSocket != -1) {
			// log_trace(logCoord,"Nuevo Socket!");
			printf("Nueva Conexion Recibida - Socket N°: %d\n",	nuevoSocket);

			int cliente;
			recibirInt(nuevoSocket,&cliente);

			switch(cliente){
			case ESI:
				printf("ESI.\n");
				break;
			case INSTANCIA:
				printf("INSTANCIA.\n");
				char* nombre = recibirMensajeArchivo(nuevoSocket);
				printf("%s.\n", nombre);
				break;
			default:
				printf("What.\n");
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

	if(config_has_property(infoConfig, "ESTIMACION_INICIAL")){
		cantidad_Entradas = config_get_int_value(infoConfig, "ESTIMACION_INICIAL");
	}

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		tamanio_Entrada = config_get_int_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		retardo = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

}
