#include "instancia.h"

int main() {

		int coordinador_socket;

		cargar_configuracion();
		configureLoggers(nombre_Instancia);
		pthread_mutex_init(&mx_Dump, NULL);

		coordinador_socket = conectarseA(coordinador_IP, coordinador_Puerto);

		if(coordinador_socket<=0){
			log_error(logE,"Error al conectar al Coordinador.\n");
			exit(1);
		}

		enviarInt(coordinador_socket, INSTANCIA);
		enviarMensaje(coordinador_socket,nombre_Instancia);

		log_trace(logT,"Conectado a Coordinador OK.\n");

		recibirInt(coordinador_socket, &qEntradas);
		recibirInt(coordinador_socket, &tamanioEntrada);

		inicializarPuntoMontaje(punto_Montaje, nombre_Instancia);
		t_inst_bitmap = creaAbreBitmap(nombre_Instancia);

		tablaEntradas =  list_create();

		operacionNumero = 0;

		pthread_t threadDump;
		if(pthread_create(&threadDump,NULL,dump, NULL)){
			log_error(logE,"Error generando thread para Dump");
		}

		int check = reviewPuntoMontaje();

		log_trace("Se inicializó la Instancia, se encontraron %d entradas de una ejecución anterior.", check);

		while(1){
			int instruccion;

			recibirInt(coordinador_socket,&instruccion);

			pthread_mutex_lock(&mx_Dump);

			int cantidadEntradas;
			switch(instruccion){
				case ENVIO_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion SET.\n");
					cantidadEntradas = recibirEntrada(coordinador_socket);
					if(cantidadEntradas<=0){
										//TODO que hace si da error?
					}
					enviarInt(coordinador_socket,cuentaBloquesUsados(t_inst_bitmap));

					break;
				case STORE_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion STORE.\n");
					ejecutarStore(coordinador_socket);
					enviarInt(coordinador_socket,cuentaBloquesUsados(t_inst_bitmap));
					break;
				case OBTENER_VALUE:
					operacionNumero++;
					log_trace(logT,"Se recibe solicitud para obener valor de key.\n");
					entregarValue(coordinador_socket);
					break;
				case COMPACTACION:
					log_trace(logT,"Se recibe orden de compactar.\n");
					compactar();
					break;
			}

			pthread_mutex_unlock(&mx_Dump);
		}
		pthread_join(threadDump, NULL);
		close_gracefully();
	return 0;
}

void close_gracefully(){
	list_destroy(tablaEntradas);

	free(coordinador_IP);
	free(punto_Montaje);
	free(nombre_Instancia);
	pthread_mutex_destroy(&mx_Dump);

	destroyLoggers();
}


/*

char* coordinador_IP;
int coordinador_Puerto;
char* reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;

*/
