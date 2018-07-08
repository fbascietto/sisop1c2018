#include "instancia.h"

int main() {

		int coordinador_socket;

		cargar_configuracion();

		configureLoggers(nombre_Instancia);

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


		// TODO: levantar tabla de entradas anterior, de ser necesario



		while(1){

			int instruccion;
			recibirInt(coordinador_socket,&instruccion);
			int cantidadEntradas;
			switch(instruccion){
				case ENVIO_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion SET.\n");
					cantidadEntradas = recibirEntrada(coordinador_socket);
					if(cantidadEntradas<=0){
										//TODO que hace si da error?
					}
					enviarInt(coordinador_socket,obtenerCantidadEntradasOcupadas());
					break;
				case STORE_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion STORE.\n");
					ejecutarStore(coordinador_socket);
					enviarInt(coordinador_socket,obtenerCantidadEntradasOcupadas());
					break;
				case OBTENER_VALUE:
					operacionNumero++;
					log_trace(logT,"Se recibe solicitud para obener valor de key.\n");
					entregarValue(coordinador_socket);
					break;

			}


		}

		close_gracefully();
	return 0;
}

void close_gracefully(){
	list_destroy(tablaEntradas);

	free(coordinador_IP);
	free(punto_Montaje);
	free(nombre_Instancia);

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
