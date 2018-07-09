#include "planificador.h"

void testearDeadlock(){
	t_proceso_esi * esi1 = malloc(sizeof(t_proceso_esi));
	t_proceso_esi * esi2 = malloc(sizeof(t_proceso_esi));
	t_proceso_esi * esi3 = malloc(sizeof(t_proceso_esi));
	t_proceso_esi * esi4 = malloc(sizeof(t_proceso_esi));

	t_clave* clave1 = malloc(sizeof(t_clave));
	t_clave* clave2 = malloc(sizeof(t_clave));
	t_clave* clave3 = malloc(sizeof(t_clave));
	t_clave* clave4 = malloc(sizeof(t_clave));


	esi1->id = 1;
	esi1->clavesTomadas = list_create();
	list_add(esi1->clavesTomadas, clave1);

	esi2->id = 2;
	esi2->clavesTomadas = list_create();
	list_add(esi2->clavesTomadas,clave2);

	esi3->id = 3;
	esi3->clavesTomadas = list_create();
	list_add(esi3->clavesTomadas,clave3);

	esi4->id = 4;
	esi4->clavesTomadas = list_create();

	clave1->esi_poseedor = esi1;
	strcpy(clave1->nombre, "key1");
	clave1->colaBloqueados = queue_create();
	queue_push(clave1->colaBloqueados, esi2);

	clave2->esi_poseedor = esi2;
	strcpy(clave2->nombre, "key2");
	clave2->colaBloqueados = queue_create();
	queue_push(clave2->colaBloqueados, esi3);

	clave3->esi_poseedor = esi3;
	strcpy(clave3->nombre, "key3");
	clave3->colaBloqueados = queue_create();
	queue_push(clave3->colaBloqueados, esi1);

	clave4->esi_poseedor = esi2;
	strcpy(clave4->nombre, "key4");
	clave4->colaBloqueados = queue_create();

	listaKeys = list_create();

	list_add(listaKeys, clave4);
	list_add(listaKeys, clave1);
	list_add(listaKeys, clave2);
	list_add(listaKeys, clave3);

	detectarDeadlock();
}
