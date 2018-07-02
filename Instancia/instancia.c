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

		calcularSiguienteEntrada(1);

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

void calcularSiguienteEntrada(int lenValue){

	int size = list_size(tablaEntradas);
	int entradasOcupadas = 0;

	void calcularEntradasOcupadas(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;

		entradasOcupadas += (entrada->size / tamanioEntrada);

		if(entrada->size%tamanioEntrada){
			entradasOcupadas++;
		}
	}

	list_iterate(tablaEntradas,calcularEntradasOcupadas);

	int entradaLibre = findFreeBloque(t_inst_bitmap);

	if(entradasOcupadas==qEntradas){

		switch (reemplazo_Algoritmo){

			case CIRCULAR  :
			  numEntradaActual = calculoCircular(lenValue, entradasOcupadas, size);
			  break;
			case LRU  :
			  numEntradaActual = calculoLRU();
			  break;
			case BSU  :
			  numEntradaActual = 0; /* calculoBSU(); */
			break;
			default:
			  numEntradaActual = reemplazo_Algoritmo;
			/* Acusar error, exit_gracefully */
		}

	} else{numEntradaActual = entradaLibre; }
}

/*

char* coordinador_IP;
int coordinador_Puerto;
char* reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;

*/
