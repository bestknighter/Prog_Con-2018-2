/****************************************************************************
 *																			*
 * Arquivo EstudoDirigido3.c criado por Gabriel Barbosa (12/0050935)		*
 * Permite multiplos macacos e gorilas sem problemas de starvation			*
 * Codigo multiplataforma, mas testado somente no Windows					*
 * Usar -std=c11 para compilar												*
 *																			*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#ifdef _WIN32
	#include <windows.h>
	#define SLEEP(ms) Sleep( ms )
#else
	#include <unistd.h>
	#define SLEEP(ms) sleep( ms )
#endif

/* Por morro */
#define MAX_MACACOS	10
#define MAX_GORILLA	2

/* Tempos em milissegundos */
#define MACACO_MAX_TRAVEL_ADD	1000
#define MACACO_MIN_TRAVEL		1000
#define MACACO_MAX_WAIT_ADD		1500
#define MACACO_MIN_WAIT			500

#define GORILLA_MAX_TRAVEL_ADD	3000
#define GORILLA_MIN_TRAVEL		2000
#define GORILLA_MAX_WAIT_ADD	5000
#define GORILLA_MIN_WAIT		5000

/* Variaveis Globais */
int nMacacosA = 0;
int nMacacosB = 0;

pthread_mutex_t nMA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nMB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t corda = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;

/* Forward declaration das funcoes */
void* macacoA(void* arg);
void* macacoB(void* arg);

void* gorillaA(void* arg);
void* gorillaB(void* arg);

int main( int argc, char* argv[] ) {
	pthread_t thread;

	/* Imprime resumo das variaveis */
	printf("\nTemos %d macacos e %d gorilas em cada morro.\n", MAX_MACACOS, MAX_GORILLA);
	printf("Macacos levam de %0.2fs a %0.2fs para decidir atravessar e levam de %0.2fs a %0.2fs para terminarem a travessia.\n", MACACO_MIN_WAIT/1000.0, (MACACO_MIN_WAIT + MACACO_MAX_WAIT_ADD)/1000.0, MACACO_MIN_TRAVEL/1000.0, (MACACO_MIN_TRAVEL + MACACO_MAX_TRAVEL_ADD)/1000.0 );
	printf("Gorilas levam de %0.2fs a %0.2fs para decidir atravessar e levam de %0.2fs a %0.2fs para terminarem a travessia.\n\n", GORILLA_MIN_WAIT/1000.0, (GORILLA_MIN_WAIT + GORILLA_MAX_WAIT_ADD)/1000.0, GORILLA_MIN_TRAVEL/1000.0, (GORILLA_MIN_TRAVEL + GORILLA_MAX_TRAVEL_ADD)/1000.0 );
	SLEEP( 500 );

	srand( time(NULL) );

	/* Cria as threads */
	int r;
	for( int i = 0; i < MAX_MACACOS; i++ ) {
		r = rand();
		pthread_create( &thread, NULL, macacoA, &r);
	}
	for( int i = 0; i < MAX_MACACOS; i++ ) {
		r = rand();
		pthread_create( &thread, NULL, macacoB, &r);
	}
	for( int i = 0; i < MAX_GORILLA; i++ ) {
		r = rand();
		pthread_create( &thread, NULL, gorillaA, &r);
	}
	for( int i = 0; i < MAX_GORILLA; i++ ) {
		r = rand();
		pthread_create( &thread, NULL, gorillaB, &r);
	}

	/* So precisa esperar uma delas encerrar, ja que todas sao loop infinito */
	pthread_join( thread, NULL );
	return 0;
}

/**
 * Macacos e Gorilas:
 * - Usam o rand() gerado pela main para fazer o srand() individual dessa thread
 * - Esperam entre MACACO_MIN_WAIT e MACACO_MIN_WAIT + MACACO_MAX_WAIT_ADD milissegundos antes de tentar atravessar
 * - Levam entre MACACO_MIN_TRAVEL e MACACO_MIN_TRAVEL + MACACO_MAX_TRAVEL_ADD milissegundos para atravessar a corda inteira
 * - Ao final, se teletransportam de volta pro comeco e repete
 * Especificidades:
 * - A corda aguenta infinitos macacos, desde que eles estejam indo na mesma direcao
 * - A corda aguenta 1 (um) e somente 1 (um) gorila por vez, independente da direcao
 * - Nao podem haver macacos e gorilas simultaneamente na corda
**/
void* macacoA(void* arg) {
	srand( *((int *)arg) );

	while(1) {
		SLEEP( rand()%MACACO_MAX_WAIT_ADD + MACACO_MIN_WAIT ); // Espera querer atravessar

		pthread_mutex_lock(&turno);
		{ // Fiz isso somente para limitar o escopo da variavel n, por ter um nome generico e ser usada somente aqui
			// Coloquei em n (variavel local) para liberar o lock nMA e os outros macacos poderem continuar rodando caso ele pare no lock da corda
			pthread_mutex_lock(&nMA);
			int n = nMacacosA;
			pthread_mutex_unlock(&nMA);
			if( 0 == n ) pthread_mutex_lock(&corda); // Bloquear corda somente se for o primeiro macaco nessa direcao
		}
		pthread_mutex_unlock(&turno);

		// Sobe na corda
		pthread_mutex_lock(&nMA);
		++nMacacosA;
		printf("Macaco A subiu na corda.\tTemos %d macacos A na corda.\n", nMacacosA);
		pthread_mutex_unlock(&nMA);

		// Atravessa corda
		SLEEP( rand()%MACACO_MAX_TRAVEL_ADD + MACACO_MIN_TRAVEL );

		// Desce da corda
		pthread_mutex_lock(&nMA);
		--nMacacosA;
		printf("Macaco A desceu da corda.\tTemos %d macacos A na corda.\n", nMacacosA);
		if( 0 == nMacacosA ) pthread_mutex_unlock(&corda); // Libera a corda somente se for o ultimo macaco a descer dela
		pthread_mutex_unlock(&nMA);
	}

	pthread_exit(0);
}

// Mesmos comentarios para macacoA sao validos para macacoB. Codigos sao identicos
void* macacoB(void* arg) {
	srand( *((int *)arg) );

	while(1) {
		SLEEP( rand()%MACACO_MAX_WAIT_ADD + MACACO_MIN_WAIT );

		pthread_mutex_lock(&turno);
		{
			pthread_mutex_lock(&nMB);
			int n = nMacacosB;
			pthread_mutex_unlock(&nMB);
			if( 0 == n ) pthread_mutex_lock(&corda);
		}
		pthread_mutex_unlock(&turno);

		pthread_mutex_lock(&nMB);
		++nMacacosB;
		printf("Macaco B subiu na corda.\tTemos %d macacos B na corda.\n", nMacacosB);
		pthread_mutex_unlock(&nMB);

		SLEEP( rand()%MACACO_MAX_TRAVEL_ADD + MACACO_MIN_TRAVEL );

		pthread_mutex_lock(&nMB);
		--nMacacosB;
		printf("Macaco B desceu da corda.\tTemos %d macacos B na corda.\n", nMacacosB);
		if( 0 == nMacacosB ) pthread_mutex_unlock(&corda);
		pthread_mutex_unlock(&nMB);
	}

	pthread_exit(0);
}

void* gorillaA(void* arg) {
	srand( *((int *)arg) );

	while(1) {
		SLEEP( rand()%GORILLA_MAX_WAIT_ADD + GORILLA_MIN_WAIT ); // Esperar querer atravessar

		pthread_mutex_lock(&turno);
		pthread_mutex_lock(&corda); // Espera ser a vez dele e entao sobe a corda
		pthread_mutex_unlock(&turno);

		printf("Gorila A subiu na corda.\n");

		SLEEP( rand()%GORILLA_MAX_TRAVEL_ADD + GORILLA_MIN_TRAVEL ); // Atravessa

		pthread_mutex_unlock(&corda); // Desce da corda
		printf("Gorila A desceu da corda.\n");
	}

	pthread_exit(0);
}

// Mesmos comentarios de gorillaA sao validos aqui
void* gorillaB(void* arg) {
	srand( *((int *)arg) );

	while(1) {
		SLEEP( rand()%GORILLA_MAX_WAIT_ADD + GORILLA_MIN_WAIT );

		pthread_mutex_lock(&turno);
		pthread_mutex_lock(&corda);
		pthread_mutex_unlock(&turno);

		printf("Gorila B subiu na corda.\n");

		SLEEP( rand()%GORILLA_MAX_TRAVEL_ADD + GORILLA_MIN_TRAVEL );

		pthread_mutex_unlock(&corda);
		printf("Gorila B desceu da corda.\n");
	}

	pthread_exit(0);
}
