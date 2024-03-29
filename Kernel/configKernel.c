#include "configKernel.h"

int leerConfigKernel(){
	archivoConfigKernel = config_create(PATH_KERNEL_CONFIG);
	log_info(loggerKernel, "Leyendo archivo de configuracion");

	if (config_has_property(archivoConfigKernel, "IP_MEMORIA")) {
		configKernel.ip_memoria = malloc(1+strlen(config_get_string_value(archivoConfigKernel, "IP_MEMORIA")));
		strcpy(configKernel.ip_memoria,config_get_string_value(archivoConfigKernel,"IP_MEMORIA"));
	} else {
		log_error(loggerKernel,"No se encontro la key IP_MEMORIA en el archivo de configuracion");
		return -1;
	}

	if (config_has_property(archivoConfigKernel, "PUERTO_MEMORIA")) {
		configKernel.puerto_memoria = config_get_int_value(archivoConfigKernel,"PUERTO_MEMORIA");
	} else {
		log_error(loggerKernel,"No se encontro la key PUERTO_MEMORIA en el archivo de configuracion");
		return -1;
	}

	if (config_has_property(archivoConfigKernel, "QUANTUM")) {
		configKernel.quantum = config_get_int_value(archivoConfigKernel,"QUANTUM");
	} else {
		log_error(loggerKernel,"No se encontro la key QUANTUM en el archivo de configuracion");
		return -1;
	}
	if (config_has_property(archivoConfigKernel, "MULTIPROCESAMIENTO")) {
		actualizar_multiprocesamiento();
	} else {
		log_error(loggerKernel,"No se encontro la key MULTIPROCESAMIENTO en el archivo de configuracion");
		return -1;
	}

	if (config_has_property(archivoConfigKernel, "METADATA_REFRESH")) {
		configKernel.metadata_refresh = config_get_int_value(archivoConfigKernel,"METADATA_REFRESH");
	} else {
		log_error(loggerKernel,"No se encontro la key METADATA_REFRESH en el archivo de configuracion");
		return -1;
	}

	if (config_has_property(archivoConfigKernel, "SLEEP_EJECUCION")) {
		configKernel.sleep_execution = config_get_int_value(archivoConfigKernel,"SLEEP_EJECUCION");
	} else {
		log_error(loggerKernel,"No se encontro la key SLEEP_EJECUCION en el archivo de configuracion");
		return -1;
	}

	if (config_has_property(archivoConfigKernel, "TIEMPO_GOSSIPING")) {
		configKernel.tiempoGossiping = config_get_int_value(archivoConfigKernel,"TIEMPO_GOSSIPING");
	} else {
		log_error(loggerKernel,"No se encontro la key TIEMPO_GOSSIPING en el archivo de configuracion");
		exit(EXIT_FAILURE);
	}

	config_destroy(archivoConfigKernel);
	return 1;
}


void verificarArchivoConfigKernel(){
	if(leerConfigKernel() == -1){
		exit_gracefully(EXIT_FAILURE);
		return;
	}
	mostrarDatosArchivoConfigKernel();
}

void mostrarDatosArchivoConfigKernel(){
	log_info(loggerKernel,"IP_MEMORIA: %s",configKernel.ip_memoria);
	log_info(loggerKernel,"PUNTO_MEMORIA: %d",configKernel.puerto_memoria);
	log_info(loggerKernel,"QUANTUM: %d",configKernel.quantum);
	log_info(loggerKernel,"MULTIPROCESAMIENTO: %d",configKernel.multiprocesamiento);
	log_info(loggerKernel,"METADATA_REFRESH: %d",configKernel.metadata_refresh);
	log_info(loggerKernel,"SLEEP_EXECUTION: %d",configKernel.sleep_execution);
	log_info(loggerKernel,"TIEMPO GOSSIPING: %d",configKernel.tiempoGossiping);
	return;
}

void actualizar_multiprocesamiento(){
	configKernel.multiprocesamiento = config_get_int_value(archivoConfigKernel,"MULTIPROCESAMIENTO");
	for(int i = 0; i<configKernel.multiprocesamiento; i++){
		pthread_t hilo;
		int thread_id = pthread_create(&hilo,NULL,ejecutar,NULL);
		list_add(hilos_ejec,(void*)thread_id);
		pthread_detach(hilo);
	}
	return;
}

void* observer_config(){
	int file_descriptor,file_observer;
	int event_size = sizeof(struct inotify_event);
	int max_cant_cambios = 1024;
	int max_file_name_size = 24;
	int buffer_size = max_cant_cambios* (event_size + max_file_name_size);
	char buffer[buffer_size];

	file_descriptor = inotify_init();
	file_observer = inotify_add_watch(file_descriptor,PATH_KERNEL_CONFIG,IN_MODIFY);

	while(1){
		read(file_descriptor,buffer,buffer_size);
		pthread_mutex_lock(&config_sem);
		actualizarArchivoConfig();
		pthread_mutex_unlock(&config_sem);
	}

	inotify_rm_watch(file_descriptor,file_observer);
	close(file_descriptor);
	return NULL;
}

void actualizarArchivoConfig(){
	archivoConfigKernel = config_create(PATH_KERNEL_CONFIG);
	if(config_has_property(archivoConfigKernel, "QUANTUM")) {
		configKernel.quantum = config_get_int_value(archivoConfigKernel,"QUANTUM");
	}
	else{
		log_error(loggerKernel,"No se encontro la key QUANTUM en el archivo de configuracion");
		return;
	}
	if(config_has_property(archivoConfigKernel, "METADATA_REFRESH")) {
		configKernel.metadata_refresh = config_get_int_value(archivoConfigKernel,"METADATA_REFRESH");
	}
	else{
		log_error(loggerKernel,"No se encontro la key METADATA_REFRESH en el archivo de configuracion");
		return;
	}
	if(config_has_property(archivoConfigKernel, "SLEEP_EJECUCION")) {
		configKernel.sleep_execution = config_get_int_value(archivoConfigKernel,"SLEEP_EJECUCION");
	}
	else{
		log_error(loggerKernel,"No se encontro la key SLEEP_EJECUCION en el archivo de configuracion");
		return;
	}
	config_destroy(archivoConfigKernel);
	mostrarDatosArchivoConfigKernel();
}
