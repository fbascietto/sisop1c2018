#include "instancia.h"
#include "../Biblioteca/biblio_sockets.c"

int main(){
	int coordinador_socket;

	cargar_configuracion();

	coordinador_socket = conectarseA(coordinador_IP, coordinador_Puerto);
	enviarInt(coordinador_socket, INSTANCIA);
	enviarMensajeArchivo(coordinador_socket,nombre_Instancia);
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

void cargar_configuracion(){

	t_config* infoConfig;

	infoConfig = config_create("../Configuracion/instancia.config");

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		coordinador_IP = config_get_string_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		coordinador_Puerto = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		reemplazo_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "PUNTO_MONTAJE")){
		punto_Montaje = config_get_string_value(infoConfig, "PUNTO_MONTAJE");
	}

	if(config_has_property(infoConfig, "NOMBRE")){
		nombre_Instancia = config_get_string_value(infoConfig, "NOMBRE");
	}

	if(config_has_property(infoConfig, "INTERVALO_DUMP")){
		intervalo_dump = config_get_int_value(infoConfig, "INTERVALO_DUMP");
	}

}
