#include "coordinador.h"

int main(){

	cargar_configuracion();
	configureLoggers();
	pthread_mutex_init(&mx_logOp, NULL);
    generarlogDeOperaciones();

	int socketEscucha;
	fd_set fdSocketsEscucha;

	instancias = list_create();
	proxima_posicion_instancia = 0;

	claves_sin_instancia = list_create();

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
	pthread_mutex_destroy(&mx_logOp);
	return 0;
}
