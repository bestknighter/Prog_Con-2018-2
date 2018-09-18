/************************************************************************************************************************************
 *																																	*
 * Arquivo EstudoDirigido2.c criado por Gabriel Barbosa (12/0050935)																*
 * Resolve starvation dos escritores mas ainda apresenta o problema de starvation de leitores										*
 * Utiliza uma solução diferente, porém relativamente similar, ao explicado em aula													*
 * Usar -std=c11 para compilar																										*
 *																																	*
************************************************************************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "unistd.h"

// Numero de leitores
#define L 8

// Numero de escritores
#define E 2

int nReading = 0;
int isWriting = 0;
int nToWrite = 0;
pthread_mutex_t mutNReading = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutNToWrite = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writing = PTHREAD_MUTEX_INITIALIZER;

void* leitor(void* arg) {
	int id = *((int*) arg);
	free(arg);
	printf("Iniciando thread leitora id#%d\n", id);
	while(1) {
		sleep(1); // Simula processamento pre e pos leitura de dados
		
		printf("Leitora #%d quer ler.\n", id);
		while(isWriting || nToWrite>0){} // Espera não ter ninguem escrevendo ou querendo escrever
		
		// Diz que esta lendo
		pthread_mutex_lock(&mutNReading);
		++nReading;
		pthread_mutex_unlock(&mutNReading);
		printf("Leitora #%d lendo.\n", id);
		sleep(2); // Simula leitura
		
		// Diz que terminou de ler
		printf("Leitora #%d terminou de ler.\n", id);
		pthread_mutex_lock(&mutNReading);
		--nReading;
		pthread_mutex_unlock(&mutNReading);
	}
	pthread_exit(0);
}

void* escritor(void* arg) {
	int id = *((int*) arg);
	free(arg);
	printf("Iniciando thread escritora id#%d\n", id);
	while(1){
		sleep(10); // Simula processamento pre e pos escrita
		
		// Diz que quer escrever
		printf("Escritora #%d quer escrever.\n", id);
		pthread_mutex_lock(&mutNToWrite);
		++nToWrite;
		pthread_mutex_unlock(&mutNToWrite);
		while(nReading){} // Espera todos os leitores que estavam lendo terminarem
		
		// Comeca a escrever
		pthread_mutex_lock(&writing);
		isWriting = 1;
		printf("Escritora #%d escrevendo.\n", id);
		sleep(5); // Simula tempo de escrita
		
		// Diz que terminou de escrever
		isWriting = 0;
		pthread_mutex_unlock(&writing);
		printf("Escritora #%d terminou de escrever.\n", id);
		pthread_mutex_lock(&mutNToWrite);
		--nToWrite; // e não quer mais escrever
		pthread_mutex_unlock(&mutNToWrite);
	}
	pthread_exit(0);
}

int main() {
	pthread_t a[L+E];
	int i = 0;
	int * id;
	
	for(int j = 0; j < L+E; j++){
		id = (int *) malloc(sizeof(int));
		*id = i++;
		if( j<L ) {
			pthread_create(&a[j], NULL, leitor, (void *) (id));
		} else {
			pthread_create(&a[j], NULL, escritor, (void *) (id));
		}
	}
	
	for (int j = 0; j < L+E ; j++) {
		pthread_join(a[j],NULL);
	}  
	printf("TERMINANDO\n");
	return 0;
}

















