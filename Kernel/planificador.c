#include "planificador.h"

void* ejecutar(){
	while(1){
		sem_wait(&execute_sem);
		pthread_mutex_lock(&config_sem);
		int quantum = configKernel.quantum;
		int sleep_time = configKernel.sleep_execution;
		pthread_mutex_unlock(&config_sem);
		//Si el grado de multiprogramación baja, se ejecutaría esta porción de código
		/*pthread_mutex_lock(&multiProcesamiento_sem);
		int multi = cambioMultiProcesamiento;
		log_info(loggerKernel,"multi: %d",multi);
		if(multi < 0){
			cambioMultiProcesamiento++;
			pthread_mutex_unlock(&multiProcesamiento_sem);
			pthread_mutex_lock(&hilos_ejec_sem);
			if(!list_is_empty(hilos_ejec)){
				list_remove_and_destroy_element(hilos_ejec,0,(void*) free);
			}
			pthread_mutex_unlock(&hilos_ejec_sem);
			sem_post(&execute_sem);
			return NULL;
		}
		pthread_mutex_unlock(&multiProcesamiento_sem);*/
		pthread_mutex_lock(&queue_ready_sem);
		t_lcb* lcb = queue_pop(queue_ready);
		pthread_mutex_unlock(&queue_ready_sem);
		lcb->estado = EXEC;
		while(quantum > 0 && lcb->program_counter < list_size(lcb->operaciones)){
			t_LQL_operacion* operacion = obtener_op_actual(lcb);
			switch(operacion->keyword){
				case SELECT:
					lql_select(operacion);
					break;
				case INSERT:
					lql_insert(operacion);
					break;
				case CREATE:
					lql_create(operacion);
					break;
				case DESCRIBE:
					lql_describe(operacion);
					break;
				case DROP:
					lql_drop(operacion);
					break;
				case JOURNAL:
					pthread_mutex_lock(&memorias_sem);
					lql_journal(memorias, operacion);
					pthread_mutex_unlock(&memorias_sem);
					break;
				case ADD:
					lql_add(operacion);
					break;
				case RUN:
					lql_run(abrirArchivo(operacion->argumentos.RUN.path), operacion);
					break;
				case METRICS:
					lql_metrics();
					operacion->success = true;
					break;
			}
			if(!operacion->success){
				lcb->abortar = true;
				break;
			}
			lcb->program_counter++;
			lcb->abortar = false;
			quantum--;
			usleep(sleep_time*100);
		}
		if(lcb->program_counter >= list_size(lcb->operaciones) || lcb->abortar){
			pasar_lcb_a_exit(lcb);
		}
		else{
			pasar_lcb_a_ready(lcb);
		}
	}
	return NULL;
}


FILE* abrirArchivo(char* path){
	FILE* f = fopen(path,"r");
	if(f == NULL){
		log_error(loggerKernel,"Error al abrir el archivo LQL");
		return f;
	}
	return f;
}

void lql_select(t_LQL_operacion* operacion){
	pthread_mutex_lock(&tablas_sem);
	t_tabla* tabla = devuelve_tabla(operacion->argumentos.SELECT.nombre_tabla);
	pthread_mutex_unlock(&tablas_sem);
	if(tabla == NULL){
		log_error(loggerKernel,"La tabla de nombre:  %s no existe",operacion->argumentos.SELECT.nombre_tabla);
		free_tabla(tabla);
		operacion->success = false;
		return;
	}
	t_memoria* memoria = obtener_memoria_consistencia(tabla->consistencia,operacion->argumentos.SELECT.key);
	if(!memoria->valida){
		log_error(loggerKernel,"No hay memoria para el criterio: %s de la tabla: %s",tabla->consistencia,operacion->argumentos.SELECT.nombre_tabla);
		free_memoria(memoria);
		operacion->success = false;
		return;
	}
	char* respuesta = opSELECT(memoria->socket_mem,operacion->argumentos.SELECT.nombre_tabla, operacion->argumentos.SELECT.key);
	puts(respuesta);
	operacion->success = true;
	free(respuesta);
	return;
}

void lql_insert(t_LQL_operacion* op){
	pthread_mutex_lock(&tablas_sem);
	t_tabla* tabla = devuelve_tabla(op->argumentos.INSERT.nombre_tabla);
	pthread_mutex_unlock(&tablas_sem);
	if(tabla == NULL){
		log_error(loggerKernel,"La tabla de nombre:  %s no existe",op->argumentos.INSERT.nombre_tabla);
		free_tabla(tabla);
		op->success = false;
		return;
	}
	t_memoria* memoria = obtener_memoria_consistencia(tabla->consistencia,op->argumentos.INSERT.key);
	if(!memoria->valida){
		log_error(loggerKernel,"No hay memoria para el criterio: %s de la tabla: %s",tabla->consistencia,op->argumentos.INSERT.nombre_tabla);
		free_memoria(memoria);
		op->success = false;
		return;
	}
	unsigned long int timestamp = obtenerTimeStamp();
	char* resp = opINSERT(memoria->socket_mem, op->argumentos.INSERT.nombre_tabla, op->argumentos.INSERT.key,op->argumentos.INSERT.valor,timestamp);
	puts(resp);
	op->success = true;
	free(resp);
	return;
}

void lql_create(t_LQL_operacion* op){
	t_memoria* memoria = obtener_memoria_consistencia(op->argumentos.CREATE.tipo_consistencia,-1);
	if(!memoria->valida){
		log_error(loggerKernel,"No hay memoria para el criterio: %s de la tabla: %s",op->argumentos.CREATE.tipo_consistencia,op->argumentos.CREATE.nombre_tabla);
		free_memoria(memoria);
		op->success = false;
		return;
	}
	char* resp = opCREATE(memoria->socket_mem, op->argumentos.CREATE.nombre_tabla, op->argumentos.CREATE.tipo_consistencia,
			op->argumentos.CREATE.numero_particiones, op->argumentos.CREATE.compactation_time);
	puts(resp);
	op->success = true;
	free(resp);
	return;
}

void lql_describe(t_LQL_operacion* op){
	if(string_is_empty(op->argumentos.DESCRIBE.nombre_tabla)){
		t_memoria* mem = random_memory(memorias);
		char* resp = opDESCRIBE(mem->socket_mem, "");
		puts(resp);
		free(resp);
		op->success = true;
	}
	else{
		pthread_mutex_lock(&tablas_sem);
		t_tabla* tabla = devuelve_tabla(op->argumentos.DESCRIBE.nombre_tabla);
		pthread_mutex_unlock(&tablas_sem);
		if(tabla == NULL){
			log_error(loggerKernel,"La tabla de nombre:  %s no existe",op->argumentos.DESCRIBE.nombre_tabla);
			free_tabla(tabla);
			op->success = false;
			return;
		}
		t_memoria* memoria = obtener_memoria_consistencia(tabla->consistencia,-1);
		if(!memoria->valida){
			log_error(loggerKernel,"No hay memoria para el criterio: %s de la tabla: %s",tabla->consistencia,op->argumentos.DESCRIBE.nombre_tabla);
			free_memoria(memoria);
			op->success = false;
			return;
		}
		char* resp = opDESCRIBE(memoria->socket_mem,op->argumentos.DESCRIBE.nombre_tabla);
		puts(resp);
		free(resp);
		op->success = true;
	}
}

void lql_drop(t_LQL_operacion* op){
	pthread_mutex_lock(&tablas_sem);
	t_tabla* tabla = devuelve_tabla(op->argumentos.DROP.nombre_tabla);
	pthread_mutex_unlock(&tablas_sem);
	if(tabla == NULL){
		log_error(loggerKernel,"La tabla de nombre:  %s no existe",op->argumentos.DROP.nombre_tabla);
		free_tabla(tabla);
		op->success = false;
		return;
	}
	t_memoria* memoria = obtener_memoria_consistencia(tabla->consistencia,-1);
	if(!memoria->valida){
		log_error(loggerKernel,"No hay memoria para el criterio: %s de la tabla: %s",tabla->consistencia,op->argumentos.DROP.nombre_tabla);
		free_memoria(memoria);
		op->success = false;
		return;
	}
	char* resp = opDROP(memoria->socket_mem, op->argumentos.DROP.nombre_tabla);
	char** valor =string_split(resp, " ");
	if(!strcasecmp(valor[0],"ERROR")){
		log_error(loggerKernel, "No se pudo borrar la tabla %s ya que no existe", op->argumentos.DROP.nombre_tabla);
	}
	else{
		log_info(loggerKernel, "Se borro la tabla %s correctamente", op->argumentos.DROP.nombre_tabla);
	}
	puts(resp);
	freeParametros(valor);
	free(resp);
	op->success = true;
	return;
}

void lql_journal(t_list* list_mem, t_LQL_operacion* op){
	for(int i = 0; i < list_size(list_mem); i++){
		/*t_memoria* memoria = list_get(list_mem,i);
		char* resp = opJOURNAL(memoria->socket_mem);
		char** valor =string_split(resp, " ");
		if(!strcasecmp(valor[0],"ERROR")){
			log_error(loggerKernel, "Hubo un problema al realizar el JOURNAL de la memoria numero %d", memoria->id_mem);
		}
		else{
			log_info(loggerKernel, "Se hizo JOURNAL de la memoria numero %d",memoria->id_mem);
		}
		freeParametros(valor);
		free(resp);*/
	}
	op->success = true;

	return;
}

void lql_add(t_LQL_operacion* op){
	if(!memoria_existente(memorias, op->argumentos.ADD.nro_memoria)){
		log_error(loggerKernel,"La memoria %d no existe o no es conocida por el Kernel",op->argumentos.ADD.nro_memoria);
		op->success = false;
		return;
	}
	if(string_equals_ignore_case(op->argumentos.ADD.criterio,"SC")){
		pthread_mutex_lock(&strong_consistency_sem);
		if(strong_consistency == NULL){
			strong_consistency = obtener_memoria_por_id(op->argumentos.ADD.nro_memoria);
			strong_consistency->valida = true;
			log_info(loggerKernel,"Se ha asignado la memoria %d al criterio Strong Consistency",strong_consistency->id_mem);
			pthread_mutex_unlock(&strong_consistency_sem);
			op->success = true;
			return;
		}
		else{
			log_error(loggerKernel,"El criterio Strong Consistency ya posee una memoria asociada");
			pthread_mutex_unlock(&strong_consistency_sem);
			op->success = true;
			return;
		}
	}
	if(string_equals_ignore_case(op->argumentos.ADD.criterio,"SHC")){
		pthread_mutex_lock(&strong_hash_consistency_sem);
		if(memoria_existente(strong_hash_consistency,op->argumentos.ADD.nro_memoria)){
			log_error(loggerKernel,"La memoria %d ya se encuentra asignada al criterio Strong Hash Consistency",op->argumentos.ADD.nro_memoria);
			pthread_mutex_unlock(&strong_hash_consistency_sem);
			op->success = true;
			return;
		}
		else{
			list_add(strong_hash_consistency,obtener_memoria_por_id(op->argumentos.ADD.nro_memoria));
			lql_journal(strong_hash_consistency, op);
			log_info(loggerKernel,"Se ha asignado la memoria %d al criterio Strong Hash Consistency",op->argumentos.ADD.nro_memoria);
			pthread_mutex_unlock(&strong_hash_consistency_sem);
			op->success = true;
			return;
		}
	}
	if(string_equals_ignore_case(op->argumentos.ADD.criterio,"EC")){
		pthread_mutex_lock(&eventual_consistency_sem);
		if(memoria_existente(eventual_consistency,op->argumentos.ADD.nro_memoria)){
			log_error(loggerKernel,"La memoria %d ya se encuentra asignada al criterio Eventual Consistency",op->argumentos.ADD.nro_memoria);
			pthread_mutex_unlock(&eventual_consistency_sem);
			op->success = true;
			return;
		}
		else{
			list_add(eventual_consistency,obtener_memoria_por_id(op->argumentos.ADD.nro_memoria));
			log_info(loggerKernel,"Se ha asignado la memoria %d al criterio Eventual Consistency",op->argumentos.ADD.nro_memoria);
			pthread_mutex_unlock(&eventual_consistency_sem);
			op->success = true;
			return;
		}
	}
	return;
}

void lql_run(FILE* archivo, t_LQL_operacion* op){
	if(archivo == NULL){
		op->success = false;
		return;
	}
	log_info(loggerKernel,"Iniciando ejecución de archivo LQL");
	char* linea = NULL;
	size_t len = 0;
	t_lcb* lcb = crear_lcb();
	 while ((getline(&linea, &len, archivo)) != -1) {
	        t_LQL_operacion* operacion = parse(linea);
	        if(operacion->valido){
	        	op->success = true;
	        	agregar_op_lcb(lcb,operacion);
	        }
	        else {
	            log_error(loggerKernel, "La linea %s no es valida", linea);
	            op->success = false;
	            return;
	        }
	}
	if(!list_is_empty(lcb->operaciones)){
		pasar_lcb_a_ready(lcb);
	}
	fclose(archivo);
	if (linea){
		free(linea);
	}
}

void lql_metrics(){
	puts("Metricas en desarrollo");
	return;
}

void* timer(){
	while(1){
		usleep(30000000);
		lql_metrics();
		pthread_mutex_lock(&queue_exit_sem);
		queue_clean_and_destroy_elements(queue_exit,(void*) free_lcb);
		pthread_mutex_unlock(&queue_exit_sem);
	}
	return NULL;
}

t_tabla* devuelve_tabla(char* nombre){

	bool same_table(t_tabla* table){
		return string_equals_ignore_case(table->nombre_tabla,nombre);
	}

	return list_find(tablas,(void*) same_table);
}

t_memoria* obtener_memoria_consistencia(char* consistencia, int key){
	t_memoria* mem = NULL;
	if(string_equals_ignore_case("SC",consistencia)){
		pthread_mutex_lock(&strong_consistency_sem);
		if(strong_consistency == NULL){
			pthread_mutex_unlock(&strong_consistency_sem);
			mem = (t_memoria*)malloc(sizeof(t_memoria));
			mem->valida = false;
			return mem;
		}
		mem = strong_consistency;
		pthread_mutex_unlock(&strong_consistency_sem);
		return mem;
	}
	else if(string_equals_ignore_case("SHC",consistencia)){
		pthread_mutex_lock(&strong_hash_consistency_sem);
		if(!list_is_empty(strong_hash_consistency)){
			if(key >= 0){
				mem = hash_memory(key);
			}
			else{
				mem = random_memory(strong_hash_consistency);
			}
			mem->valida = true;
			pthread_mutex_unlock(&strong_hash_consistency_sem);
			return mem;
		}
		else{
			pthread_mutex_unlock(&strong_hash_consistency_sem);
			mem = (t_memoria*)malloc(sizeof(t_memoria));
			mem->valida = false;
			return mem;
		}
	}
	else if(string_equals_ignore_case("EC",consistencia)){
		pthread_mutex_lock(&eventual_consistency_sem);
		if(!list_is_empty(eventual_consistency)){
			mem = random_memory(eventual_consistency);
			mem->valida = true;
			pthread_mutex_unlock(&eventual_consistency_sem);
			return mem;
		}
		else{
			pthread_mutex_unlock(&eventual_consistency_sem);
			mem = (t_memoria*)malloc(sizeof(t_memoria));
			mem->valida = false;
			return mem;
		}
	}
	mem = (t_memoria*)malloc(sizeof(t_memoria));
	mem->valida = false;
	return mem;
}

t_memoria* obtener_memoria_por_id(int id){
	bool same_id(t_memoria* mem){
		return mem->id_mem == id;
	}

	return list_find(memorias,(void*) same_id);
}

t_memoria* hash_memory(int key){
	int index = key % list_size(strong_hash_consistency);
	return list_get(strong_hash_consistency,index);
}

t_memoria* random_memory(t_list* lista){
	int index = random() % list_size(lista);
	return list_get(lista,index);
}

