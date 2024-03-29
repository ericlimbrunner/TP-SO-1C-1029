#include "usoVariado.h"

char* int_to_string(int numero){
	int length = snprintf(NULL,0,"%d",numero);
	char* string = malloc(length + 1);
	snprintf(string,length + 1,"%d",numero);
	return string;
}

int pasarMilisegundosASegundos(int milisegundos){
	return milisegundos * 0.001;
}

void printearArrayDeChars(char** arrayDeChars){
	int tamanioIpSeeds = tamanioArray(arrayDeChars);
	for (int i =0; i<tamanioIpSeeds; i++){
		printf("%s\n", arrayDeChars[i]);
	}

}

void printearArrayDeInts(int arrayDeInts[]){
	int tamanioIpSeeds = tamanioArray(arrayDeInts);
	for (int i =0; i<tamanioIpSeeds; i++){
		int a = (arrayDeInts[i]);
		printf("%d\n", a);
	}
}

void hacerFreeArray(void** array){
	for(int i = 0; array[i]!= NULL; i++){
		free(array[i]);
	}
}

int tamanioArray(void** array){
	int i = 0;
	while(array[i]!=NULL){
		i++;
	}
	return i;
}

long int cantLugaresEnArchivo(FILE* archivo){
	 long int sz;
	    fseek(archivo, 0L, SEEK_END);
	    sz = ftell(archivo);
	return sz;
}

int longitudArrayDePunteros(char **unArrayDePunteros){
	int i;
	for(i=0; unArrayDePunteros[i] != NULL ;i++);
	return i;
}

unsigned long int obtenerTimeStamp(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	double valor = ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
	return (unsigned long int) valor;
}


int buscarFinalValue(char** value){
	char c = '"';
	char *ptr = malloc(2*sizeof(char));
	ptr[0] = c;
	ptr[1] = '\0';

	int cantElementos= tamanioArray((void**)value);
	for(int i = 4; i< cantElementos; i++){
		if(string_ends_with(value[i], ptr)){
			free(ptr);
			return i;
		}
	}
	free(ptr);
	return 0;
}

char* armarValue2(char** value){
	int ultimaPosicion = tamanioArray((void**)value);
	char* operacionFinal = strdup(value[3]);
	for( int i = 4 ; i < ultimaPosicion; i ++){
		string_append(&operacionFinal, ";");
		string_append(&operacionFinal, value[i]);
	}
	return operacionFinal;
}
char* armarValue(char** value){
	int ultimaPosicion = buscarFinalValue(value);
	char* operacionFinal = strdup(value[3]);
	for( int i = 4 ; i <= ultimaPosicion; i ++){
		string_append(&operacionFinal, ";");
		string_append(&operacionFinal, value[i]);
	}
	return operacionFinal;
}

char* armarValue3(char** value){
	char** v = string_split(value[3], " ");
	int ultimaPosicion = tamanioArray(v);
	char* operacionFinal = strdup(v[0]);
	for( int i = 1 ; i < ultimaPosicion; i ++){
		string_append(&operacionFinal, ";");
		string_append(&operacionFinal, v[i]);
	}
	hacerFreeArray(v);
	free(v);
	return operacionFinal;
}

char * quitarComillas(char* ip){
	return string_substring(ip,1,(string_length(ip)-2));
}

int pasarUINT16AInt(uint16_t nro){
	char str[120000];

	int str_len = sprintf(str, "%d", nro);

	return atoi(str);
}
