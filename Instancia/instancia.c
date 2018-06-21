#include "instancia.h"
#include <CUnit/Basic.h>
// #include "cunit_def.h"

int main() {
/*if ((argc > 1) && (strcmp(argv[1], "-testea") == 0)){
	printf("corriendo test");


	CU_initialize_registry();

	CU_register_suites(suites);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}else {*/
		int coordinador_socket;
		FILE* archivoDatos;

		cargar_configuracion();

		configureLoggers(nombre_Instancia);

		coordinador_socket = conectarseA(coordinador_IP, coordinador_Puerto);
		enviarInt(coordinador_socket, INSTANCIA);
		enviarMensaje(coordinador_socket,nombre_Instancia);

		recibirInt(coordinador_socket, &qEntradas);
		recibirInt(coordinador_socket, &tamanioEntrada);

		archivoDatos = inicializarPuntoMontaje(punto_Montaje, nombre_Instancia);

		tablaEntradas =  list_create();
		operacionNumero = 0;
		// TODO: levantar tabla de entradas anterior, de ser necesario

		calcularSiguienteEntrada()

		while(1){

			int instruccion;
			recibirInt(coordinador_socket,&instruccion);
			operacionNumero++;
			int cantidadEntradas;
			switch(instruccion){
				case ENVIO_ENTRADA:
					cantidadEntradas = recibirEntrada(coordinador_socket,archivoDatos);

					if(cantidadEntradas<=0){
										//TODO que hace si da error?
					}

					enviarInt(coordinador_socket,obtenerCantidadEntradasOcupadas());
					break;
				case STORE_ENTRADA:
					ejecutarStore(coordinador_socket);
					enviarInt(coordinador_socket,obtenerCantidadEntradasOcupadas());

			}


		}


		destroyLoggers();

	//}


	return 0;
}


void calcularSiguienteEntrada(){
	switch (reemplazo_Algoritmo){

		case CIRCULAR  :
		  numEntradaActual = calculoCircular();
		  break;
		case LRU  :
		  numEntradaActual = 0; /* calculoLRU(); */
		  break;
		case BSU  :
		  numEntradaActual = 0; /* calculoBSU(); */
		break;
		default:
		  numEntradaActual = reemplazo_Algoritmo;
		/* Acusar error, exit_gracefully */
	}
}

/*

char* coordinador_IP;
int coordinador_Puerto;
char* reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;

*/
