#ifndef RODOVIARIA_HPP
#define RODOVIARIA_HPP

#include <pthread.h>
#include <semaphore.h>
#include <vector>

#define N_VAGAS 4

class Onibus;

class Rodoviaria {
	public:
		static void Start();
		static void Join();
		static void WakeUp();
		static void Destroy();

		static void RegistrarOnibus( Onibus* onibus );

		static void esperarAtualizacao_Passageiro();
		static void entrarNaFila_Onibus( Onibus* onibus );
		static void sairDaRodoviaria_Onibus( Onibus* onibus );

		static std::vector< Onibus* >& GetListaOnibus();

		static void printInfo();
	private:
		Rodoviaria();
		~Rodoviaria();

		static Rodoviaria* instance;

		pthread_t thread;

		bool atualizou;
		pthread_mutex_t l_atualizou;
		pthread_cond_t acorda;

		pthread_mutex_t l_novosDados;
		pthread_cond_t tentarDeNovo;

		sem_t s_vagas;

		pthread_mutex_t l_estacionamento;
		std::vector< Onibus* > onibusRegistrados;

		static void* main( void* arg );
};

#endif // RODOVIARIA_HPP
