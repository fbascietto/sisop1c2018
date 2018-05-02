#include "instancia.h"
//#include "instanciaTest.h"

int main(int argc, char **argv) {
	/*if (strcmp(argv[1], "-testea") == 0){
		printf("corriendo test");
		correrTests();
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

			while(1){
				if(recibirEntrada(coordinador_socket,archivoDatos)<=0){
					//TODO que hace si da error?
				}

			}


			destroyLoggers();

	//}


		return 0;
}



/*

char* coordinador_IP;
int coordinador_Puerto;
char* reemplazo_Algoritmo;
char* punto_Montaje;
char* nombre_Instancia;
int intervalo_dump;

*/
