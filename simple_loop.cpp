#include "simple_loop.h"
#include "functions.h"

typedef std::vector<double> d_vec;

typedef std::chrono::steady_clock Clock;
typedef std::chrono::microseconds microsec;

int interferenceLoop(int functionType, int interfereMicrosec){
	Clock::time_point t0 = Clock::now();
	microsec activeTime = microsec(interfereMicrosec);

	int tmp = interference_function(functionType, t0, activeTime);
	return tmp;
}