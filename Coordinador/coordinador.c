#include "coordinador.h"

int main(){
	cargar_configuracion();
	configureLoggers();

	int socketEscucha;
	fd_set fdSocketsEscucha;

	instancias = list_create();
	posicion_puntero = 0;

	FD_ZERO(&fdSocketsEscucha);
	socketEscucha = escuchar(coordinador_Puerto_Escucha);

	FD_SET(socketEscucha, &fdSocketsEscucha);
	t_esperar_conexion *esperarConexion;

	esperarConexion = malloc(sizeof(t_esperar_conexion));
	esperarConexion->fdSocketEscucha = fdSocketsEscucha;
	esperarConexion->socketEscucha = socketEscucha;

	esperarConexiones((void*) esperarConexion);

	destroyLoggers();
	list_destroy(instancias);
	return 0;
}
