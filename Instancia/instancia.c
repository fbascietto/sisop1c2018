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

		t_list * whitelist = recibirClavesAMantener(coordinador_socket);

		inicializarPuntoMontaje(punto_Montaje, nombre_Instancia);
		t_inst_bitmap = crearBitmapVacio();

		tablaEntradas =  list_create();

		operacionNumero = 0;

		if(pthread_create(&threadDump,NULL,dump, NULL)){
			log_error(logE,"Error generando thread para Dump");
		}

		int check = reviewPuntoMontaje(whitelist);

		log_trace(logT,"Se inicializó la Instancia, se encontraron %d entradas de una ejecución anterior.", check);
		list_destroy(whitelist);
		bool mantenerLoop = true;
		while(mantenerLoop){
			int instruccion;

			if(recibirInt(coordinador_socket,&instruccion)<=0){
				log_error(logE,"coordinador desconectado");
				break;
			}

			pthread_mutex_lock(&mx_Dump);

			int cantidadEntradas;
			switch(instruccion){
				case ENVIO_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion SET.\n");
					cantidadEntradas = recibirEntrada(coordinador_socket);
					if(cantidadEntradas<=0){
						if(cantidadEntradas == SIN_ENTRADA){
							log_warning(logT, "No hay entradas atomicas para reemplazar");
						}
						log_warning(logT,"No fue posible guardar la entrada");
						if(enviarInt(coordinador_socket,ERROR_EJECUCION)<=0){
							log_error(logE,"coordinador desconectado");
							mantenerLoop = false;

						}
						break;
					}else{
						if(enviarInt(coordinador_socket,ENTRADAS_OCUPADAS)<=0){
							log_error(logE,"coordinador desconectado");
							mantenerLoop = false;
						} else {

						if(enviarInt(coordinador_socket,cuentaBloquesUsados(t_inst_bitmap))<=0){
							log_error(logE,"coordinador desconectado");
							mantenerLoop = false;
						};
						}
						break;
					}


					break;
				case STORE_ENTRADA:
					operacionNumero++;
					log_trace(logT,"Se recibe instruccion STORE.\n");
					ejecutarStore(coordinador_socket);
					if(enviarInt(coordinador_socket,cuentaBloquesUsados(t_inst_bitmap))<=0){
						log_error(logE,"coordinador desconectado");
						mantenerLoop = false;
						break;

					}
					break;
				case OBTENER_VALUE:
					operacionNumero++;
					log_trace(logT,"Se recibe solicitud para obener valor de key.\n");

					if(entregarValue(coordinador_socket)<=0){
						log_error(logE,"coordinador desconectado");
						mantenerLoop = false;
						break;
					}
					break;
				case COMPACTACION:
					log_trace(logT,"Se recibe orden de compactar.\n");
					compactar();
					break;
			}

			pthread_mutex_unlock(&mx_Dump);
		}

		close_gracefully();
	return 0;
}

void close_gracefully(){

	pthread_mutex_lock(&mx_Dump);

	pthread_cancel(threadDump);

	//pthread_mutex_unlock(&mx_Dump);

	void liberarEntrada(void * parametro){
		free(parametro);
	}
	destruir_bitmap(t_inst_bitmap);
	list_destroy_and_destroy_elements(tablaEntradas,liberarEntrada);

	free(coordinador_IP);
	free(punto_Montaje);
	free(nombre_Instancia);
	pthread_mutex_destroy(&mx_Dump);

	destroyLoggers();
}
