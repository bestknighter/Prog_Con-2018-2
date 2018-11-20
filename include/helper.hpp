#ifndef HELPER_HPP
#define HELPER_HPP

#include <cstdlib>
#include <ctime>

#ifdef _WIN32
	#include <windows.h>
	
	#define SLEEP(ms) Sleep(ms)
#else
	#define _BSD_SOURCE
	#include <unistd.h>

	#define SLEEP(ms) usleep( 1000*(ms) )
#endif

#define RAND_INTERVAL(min, max) ( rand()%( (max) - (min) ) + (min) )
#define RAND_INTERVAL2(mid, rng) ( rand()%( 2 * (rng) ) + (mid) - (rng) )

#include <cstdio>

#endif // HELPER_HPP
