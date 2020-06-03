#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <thread>
#include <time.h>
#include <ctime>
#include <chrono>
#include <ratio>
#include <vector>
#include <functional>
#include <csignal>

//#include "ScalingEnigmaThread.cpp"



typedef std::vector<double> d_vec;
typedef std::vector<bool> b_vec; 

typedef std::chrono::steady_clock Clock;
typedef std::chrono::milliseconds milliseconds;


time_t getTime() {
	return time(NULL);
}


// Does Things and reports back runtime 
//TODO interrupt
void ScalingEnigma(int scalingFactor, double const & runtime) {
	Clock::time_point t0 = Clock::now();
	volatile std::sig_atomic_t processing_interrupted = false;
	bool interr = false;
	{
		int const arrSize = 1 << 3;
		double numbers[arrSize];
		for (int i = 0; i < arrSize; i++)
			numbers[i] = 1.0;

		for (int i = 0; i < arrSize; i++) {
			for (int a = 0; a < scalingFactor; a++)
				for (int b = 0; b < scalingFactor; b++)
					numbers[i] = (numbers[a%arrSize] + numbers[b %arrSize]) * 3;

		}
	}
	double & tim = const_cast<double &>(runtime);
	Clock::time_point t1 = Clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
	tim = time_span.count();
	printf("Test %f,    \n", time_span.count());
}

void eat() {

}



int main(int argc, char **argv)
{
	srand (time(NULL));
	printf("Initiating\n");
	
	

	float time_fraction = .5;
	float targetFractionMin = 5;//atof(argv[1]);
	float targetFractionMax = targetFractionMin;//atof(argv[1]);

	int runtime = 60; //in seconds
	bool indef = false;
	int numThreads = 28;
	
	for(int a=1;a<argc;a++){
		if(a ==1)
			targetFractionMin = atof(argv[a]);
		if(a==2)
			runtime = atoi(argv[a]);
	}
	
	
	if (targetFractionMin > 1) targetFractionMin = 1 / targetFractionMin;
	if (targetFractionMax > 1) targetFractionMax = 1 / targetFractionMax;
	if(targetFractionMax < targetFractionMin){} //TODO
	
	
	//TODO
	float step_length = 1.0;

	time_t initTime = time(NULL);
	int step = 0;
	
	printf("Starting Interference:\nStep%f\nSlow Min/Max: %f, %f\n time: %f\n\n", step_length, targetFractionMin, targetFractionMin, runtime* step_length);
	

	const int storeSize = 3;
	bool* SEThreadInterrupted[storeSize];
	double* SEThreadRuntime[storeSize];
	int SE_Thread_Count[storeSize];

	int SEnumTimesamplesPerStep = 10;
	int SEnumTimesamplesTotal = 50;
	int SEtimesamplePosition = 0;
	d_vec SEtimesamples(SEnumTimesamplesTotal);
	for (int a = 0; a < SEnumTimesamplesTotal; a++)
		SEtimesamples[a] = -1;

	int SEinitialProbeSize = 50;
	if (SEinitialProbeSize > SEnumTimesamplesTotal) SEinitialProbeSize = SEnumTimesamplesTotal;
	double SEaverageTime = -1;

	// generate initial time
	std::vector<std::thread> probeThreads(SEinitialProbeSize);

	// TODO fast approximation for time during init
	// launching threads to calculate initial time
	for (int p = 0; p < SEinitialProbeSize; p++) {
		probeThreads[p] = std::thread(ScalingEnigma, 600, std::ref(SEtimesamples[p])); //std::ref(fr), std::vector<bool>::reference(probeInter[p])
	}
	for (auto&& i : probeThreads)
		i.join();

	// average
	double max = -1, min = 100;
	double total = 0;
	int cnt = 0;
	for (int p = 0; p < SEnumTimesamplesTotal; p++) {
		double var = SEtimesamples[p];
		if (var >= 0 && var < 99) {
			total += var;
			cnt++;
			if (var > max)
				max = var;
			if (var < min)
				min = var;
		}
	}
	if (cnt > 0)
		SEaverageTime = total / cnt;

	printf("valid times: %d\n", cnt);
	printf("avg: %f, max %f, min %f\n", SEaverageTime, max, min);


	/*
	while (indef || difftime(getTime(), initTime) < runtime) {
		time_t startTime = time(NULL);
		time_fraction = step_length * (targetFractionMin + rndNum() * (targetFractionMax - targetFractionMin));
		printf("starting step %d\tslow:%f\n", step++, time_fraction);

		const int calcScale = 1 << 10;
		double vector[calcScale];
		for (int i = 0; i < calcScale; i++)
			vector[i] = 1.0;
		int tmp = 0;


		while (difftime(getTime(), startTime) < time_fraction * step_length) {
			//scedule(static) 
			#pragma omp parallel for default(none) shared(vector, startTime, time_fraction, numThreads) num_threads(numThreads)
			for (int i = 0; i < calcScale; i++)
			{
				for (int a = 0; a < calcScale; a++) {
					vector[a] = (vector[a] + vector[a]) * 3;
				}
			}
			tmp++;
		}
		printf("step done \ttime:%f \ttotal calcs:%d\n", difftime(getTime(), startTime), tmp);
    	
		if(time_fraction != 1){
			int num_milliseconds = (int)(1000 * step_length * (1.0 - time_fraction));
			std::this_thread::sleep_for(std::chrono::milliseconds(num_milliseconds));
			printf("ending step \ttotal time: %f \ttime waited %d\n", difftime(getTime(), startTime), num_milliseconds);
		}
    }
	 */

	std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 * step_length * 10)));
   return 0;
}