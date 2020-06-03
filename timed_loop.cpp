#include "functions.h"
#include "simple_loop.h"
#include "timed_loop.h"


typedef std::chrono::steady_clock Clock;
typedef std::chrono::microseconds microseconds;

volatile std::atomic<bool> processing_interrupted;
std::atomic<double> average;
std::atomic<int> scale_for_average;

const int maximum_attempts = 3;
const double maximum_fraction = 0.5;

int loop_cnt = 0;
int loop_duration = 0;

extern "C" void interrupt_processing(int s) {
	printf("got signal %d", s);
    processing_interrupted = true;
}

int tNow(Clock::time_point tZero){
	auto t1 = Clock::now();
	auto m = std::chrono::duration_cast<microseconds>(t1 - tZero);
	return m.count();
}

void generate_repetition_num(int functionType, int interfereMicrosec){
	// initiate loop variables before measuring duration
	auto t0 = Clock::now();
	int vector[vector_size];
	for (int i = 0; i < vector_size; i++)
		vector[i] = 1.0;
	
	// try a single repetition count max x times
	int current_attempts = 0;
	int current_rep = 4;
	int next_rep = 4;
	int current_duration = 0;
	
	/*
	stop when on repetition:
	exceeds fraction of target time
	exceeds 1 ms
	is too long to run uninterrupted
	*/
	while(
		current_duration > interfereMicrosec * maximum_fraction ||
		current_duration > 1000 ||
		current_attempts > maximum_attempts
	) {		
		//theory: maximize sceduled time
		std::this_thread::sleep_for(microseconds(1));
		processing_interrupted = false;
		t0 = Clock::now();
	
		// attempt to run one repetition without interruprion by the scheduler
		std::signal(SIGCONT, &interrupt_processing);
		interference_single(vector, vector_size, functionType, current_rep);
		std::signal(SIGCONT, SIG_DFL);
		
		if(!processing_interrupted){			
			current_rep = next_rep;
			next_rep *= 2;
			current_duration = tNow(t0);
			current_attempts = 0;
		} else {
			current_attempts++;
		}
	}
	// pass values
	loop_cnt = current_rep;
	loop_duration = current_duration;
}

void interferenceTimed(int functionType, int interfereMicrosec){
	generate_repetition_num(functionType, interfereMicrosec);
	
	int final_repetitions = (int)(interfereMicrosec / loop_duration) + 1;
	
	pid_t pid = fork();
	if(pid == 0){
		// child
		// run interference in new child process
		interference_function_fixed_length(functionType, loop_cnt, final_repetitions);		
	} else if (pid == 1) {
		// nothing
	} else {
		printf("this is wrong: fork didn't work");
	}	
}