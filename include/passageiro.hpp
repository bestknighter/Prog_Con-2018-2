#ifndef PASSAGEIRO_HPP
#define PASSAGEIRO_HPP

#include <vector>
#include <pthread.h>

#define MAX_TOL 5

class Onibus;

class Passageiro {
	public:
		Passageiro();
		~Passageiro();

		void Start();
		void Join();
	private:
		static int nextID;

		pthread_t thread;
		int myID;
		int tolerancia;
		Onibus* onibus;

		static void* main( void* arg );

		const std::vector< Onibus* > GetPreferencias();
};

#endif // PASSAGEIRO_HPP
