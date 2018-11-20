#ifndef ONIBUS_HPP
#define ONIBUS_HPP

#include <pthread.h>
#include <semaphore.h>

#define TEMPO_PERCURSO 10000 // ms
#define MAX_CAPACITY 10

class Onibus {
	public:
		Onibus();
		~Onibus();
		
		void Start();
		void Join();

		long long GetETA();
		bool isEstacionado();

		void SetEstacionado( bool estacionado );

		bool tentaEntrar();
		void sentar();

		void printInfo();
	private:
		static int nextID;

		pthread_t thread;
		int myID;

		bool estacionou;
		pthread_mutex_t l_estacionou;
		
		pthread_cond_t podeSair;
		
		pthread_mutex_t l_lotacao;
		sem_t s_lotacao;

		bool chegouDestino;
		pthread_cond_t podeDescer;
		pthread_mutex_t l_chegouDestino;
		pthread_cond_t esvaziou;

		long long ETA;
		pthread_mutex_t l_ETA;

		void SetETA( long long newETA );

		static void* main( void* arg );
		static void esperaNaRodoviaria( Onibus* ref );
		static void esperaEsvaziar( Onibus* ref );
		static void chegaDestino( Onibus* ref );
};

#endif // ONIBUS_HPP
