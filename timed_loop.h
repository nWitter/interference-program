#ifndef TIMED_LOOP
#define TIMED_LOOP

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <thread>
#include <ratio>
#include <chrono>
#include <atomic>
#include <vector>
#include <csignal>
#include <unistd.h>

void interferenceTimed(int functionType, int interfereMicrosec);


#endif