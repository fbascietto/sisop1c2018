#include "planificador.h"

int main(){
	configureLogger();
	iniciaConsola();
	return 0;
}

void configureLogger(){

	  LogL = LOG_LEVEL_TRACE;
	  logPlan = log_create("planificador.log","Planificador", true, LogL);

}

void iniciaConsola(){

	log_trace(logPlan,"Se inicializa la consola del planificador. \n");

	while(1) {
		char* linea = readline("DR-Planificador:" );

		if(linea)
			add_history(linea);

		if(!strncmp(linea, "exit", 4)) {
			log_trace(logPlan,"Consola recibe ""exit""");
			free(linea);
			exit_gracefully(1);
		} else if(!strncmp(linea, "pausa", 5))
		{
			log_trace(logPlan,"Consola recibe ""pausa""");
			free(linea);

		} else if(!strncmp(linea, "continuar", 9))
		{
			log_trace(logPlan,"Consola recibe ""continuar""");
			free(linea);

		} else if(!strncmp(linea, "bloquear", 8))
		{
			log_trace(logPlan,"Consola recibe ""bloquear""");
			free(linea);

		} else if(!strncmp(linea, "desbloquear", 11))
		{
			log_trace(logPlan,"Consola recibe ""desbloquear""");
			free(linea);

		} else if(!strncmp(linea, "listar", 6))
		{
			log_trace(logPlan,"Consola recibe ""listar""");
			free(linea);

		} else if(!strncmp(linea, "status", 6))
		{
			log_trace(logPlan,"Consola recibe ""status""");
			free(linea);

		} else if(!strncmp(linea, "kill", 4))
		{
			log_trace(logPlan,"Consola recibe ""kill""");
			free(linea);

		} else if(!strncmp(linea, "deadlock", 8))
		{
			log_trace(logPlan,"Consola recibe ""deadlock""");
			free(linea);

		} else if(!strncmp(linea, "info", 4))
		{
			log_trace(logPlan,"Consola recibe ""info""\n");
			printf("Destino-Rusia Planificador: Ayuda\n");
			printf("Los parámetros se indican con <> \n------\n");
			printf("pausa - Pausar planificación : El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado.\n");
			printf("continuar - Continuar planificación : Reanuda el Planificador.\n");
			printf("bloquear <clave> <ID>: Se bloqueará el proceso ESI hasta ser desbloqueado, especificado por dicho <ID> en la cola del recurso <clave>..\n");
			printf("desbloquear <clave>: Se desbloqueara el proceso ESI con el ID especificado. Solo se bloqueará ESIs que fueron bloqueados con la consola. Si un ESI está bloqueado esperando un recurso, no podrá ser desbloqueado de esta forma.\n");
			printf("listar <recurso>: Lista los procesos encolados esperando al recurso.\n");
			printf("kill <ID>: finaliza el proceso.\n");
			printf("deadlock: Analiza los deadlocks que existan en el sistema y a que ESI están asociados.\n");
			printf("exit - Sale del programa.\n");
			free(linea);
		}
		else {
			printf("No se reconoce el comando ""%s""\n",linea);
			printf("Para más información utilice el comando ""info"".\n");
			free(linea);
		}

	}
}

void exit_gracefully(int return_nr) {
  	log_destroy(logPlan);
	exit(return_nr);
}
