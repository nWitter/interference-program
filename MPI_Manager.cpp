#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <ratio>
#include <functional>
#include <string>
#include <vector>
#include <iostream>

#include "mpi.h"
#include "simple_loop.h"
#include "timed_loop.h"
#include "functions.h"

typedef std::chrono::steady_clock Clock;
typedef std::chrono::microseconds microsec;

const int ENI_SLEEP = -1;
const int ENI_INTERFERE = 1;
const int ENI_NULL = 0;

const int POLICY_RANDOM = 10;
const int POLICY_ROUNDROBIN = 11;
const int POLICY_FIXED = 12;


const int instance_hard_limit = 10; // avoid errors, no technical reasons
const int intervalBase = 1000000; // one second in microsec

// rnd {0,1} 5-digits
float rndNum(){
	return static_cast <float> (rand() % 10000) / 10000;
}

float intervallFinal(bool rnd, float min, float max){
	float fin;
	if(rnd){
		fin = min + (max - min) * rndNum();
	} else {
		fin = min;
	}
	return fin;
}

/*
-a {n} || --affected {n}
-ar {n} {m} || --affectedRnd {n} {m}
nodes affected simultaniously by interference

-i {n} || --intervall {n}
-ir {n} {m} || --intervallRnd {n} {m}
the timefraction per step the interference program is running, values greater 1 run multiple instances
e.g. -intervall 0.5 -> runs interference for 0.5s in each 1s timestep(default)

policy: default random, last assigned policy takes priority
-rr || --round_robin
-f || --fixed_nodes
--policy_random

-step {n} || --step_length {n}
set length of the timestep in seconds; default: 1
example: -step 0.5 -> timestep of 0.5s

--duration {n}
number of steps executed; interference runs indefenitely if no limit is specified

--seed {n}
seed for random numbers

--function {n}
change function used in interference
1 (default) calculation heavy
2 memory heavy
3 mixed

--timed_loop
using timed interference variant

*/
int main(int argc, char **argv)
{
	//mpi
	int numtasks, rank, dest, source, rc, count, tag = 1;
	char inmsg, outmsg = 'x';
	MPI_Status Stat;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	std::cout << "Starting: rank" << rank << ", tasks " << numtasks << "\n";
	
	// configurable by args
	int designation_policy = POLICY_RANDOM;
	int policy_round_robin_var = 0;
	
	float affected_num = 1;
	float affected_num_max = 1;
	bool affected_rnd = false;
	
	float intervall_time = 0.5;
	float intervall_time_max = 1;
	bool intervall_time_rnd = false;
	
	float step_length = 1.0;
	int function_type = 1;
	bool use_timed_loop = false;
	
	//
	int interference_duration = -1;
	bool interference_infinite = true;
		
	srand(time(NULL));
	
	for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
		std::cout << "reading arg " << i << ": " << arg;
		if(i < argc && (arg == "-a" || arg == "--affected")){
			std::cout << " affected ";
			float x = atof(argv[++i]);
			if(x >= 0)
				affected_num = x;
			else
				std::cout << "--- ignored arg: negative" << x;
		} else if(i+2 <= argc && (arg == "-ar" || arg == "--affectedRnd")){
			std::cout << " affectedRnd ";
			float x = atof(argv[++i]);
			float y = atof(argv[++i]);
			if(x >= 0 && y >= 0) {
				affected_num = x;
				affected_num_max = y;
				affected_rnd = true;
			} else
				std::cout << "--- ignored arg: negative" << x << y;
		} else if(i < argc && (arg == "-i" || arg == "--intervall")){
			std::cout << " intervall ";
			float x = atof(argv[++i]);
			if(x >= 0 && x <= 10)
				intervall_time = x;
			else
				std::cout << "--- ignored arg: negative or greater 10" << x;
		} else if(i+2 <= argc && (arg == "-ir" || arg == "--intervallRnd")){
			std::cout << " intervallRnd ";
			float x = atof(argv[++i]);
			float y = atof(argv[++i]);
			if(x >= 0 && y >= 0 && x <= 10 && y <= 10) {
				intervall_time = x;
				intervall_time_max = y;
				intervall_time_rnd = true;
			} else
				std::cout << "--- ignored arg: negative or greater 10" << x << y;
		} else if(i < argc && (arg == "-step" || arg == "--step_length")){
			std::cout << " step_length ";
			float x = atof(argv[++i]);
			if(x > 0)
				step_length = x;
			else
				std::cout << "--- ignored arg: negative" << x;
		} else if(arg == "-rr" || arg == "--round_robin"){
			std::cout << " round robin ";
			designation_policy = POLICY_ROUNDROBIN;
		} else if(arg == "-f" || arg == "--fixed_nodes"){
			std::cout << " fixed nodes ";
			designation_policy = POLICY_FIXED;
		} else if(arg == "--policy_random"){
			std::cout << " fixed nodes ";
			designation_policy = POLICY_RANDOM;
		} else if(arg == "--timed_loop"){
			std::cout << " using timed_loop ";
			use_timed_loop = true;
		} else if(i < argc && arg == "--duration"){
			std::cout << " duration ";
			int x = atoi(argv[++i]);
			if(x > 0){
				interference_duration = x;
				interference_infinite = false;
			} else
				std::cout << "--- ignored arg: negative" << x;
		} else if(i < argc && arg == "--seed"){
			std::cout << " seed ";
			int x = atoi(argv[++i]);
			srand(x);
		} else if(i < argc && arg == "--function"){
			std::cout << " function ";
			int x = atoi(argv[++i]);
			if(x > 0 && x < 4){
				function_type = x;
			} else
				std::cout << "--- ignored arg: available function options: 1, 2, 3" << x;
		} else {
			std::cout << "no valid string";
		}
		std::cout << "\n";
    }
	

	/*
	mpi buffer; use:
		0: interference state
		1: interference time per step
	
	*/
	const int bufferSize = 2;
	float* scatterBuffer = (float*)malloc(bufferSize * numtasks * sizeof(float));
	float inbuffer[bufferSize];
	
	
	//
	for (int x = 0; interference_infinite || x < interference_duration; x++) {
		Clock::time_point t0 = Clock::now();
		
		// communication between ranks
		// rank 0 controls interfering ranks
		if (rank == 0) {
			// affected nodes in time step
			int affected_final;
			if(affected_rnd){
				affected_final = affected_num + (int)((affected_num_max - affected_num) * rndNum());
			} else {
				affected_final = affected_num;
			}
			
			if(affected_final > numtasks)
				affected_final = numtasks;
			
			// final interference time is calculated seperately for each interfering node
			
			for(int a = 0; a < numtasks; a++){
				scatterBuffer[a * bufferSize] = ENI_SLEEP;
				scatterBuffer[a * bufferSize + 1] = 0;
				scatterBuffer[a * bufferSize + 2] = 0;
			}
			
			// designate active ranks for the next timestep according to policy
			if(designation_policy == POLICY_FIXED){
				for(int a = 0; a < affected_final; a++){
					scatterBuffer[a * bufferSize] = ENI_INTERFERE;
					scatterBuffer[a * bufferSize + 1] = intervallFinal(intervall_time_rnd, intervall_time, intervall_time_max);
				}				
			} else if(designation_policy == POLICY_ROUNDROBIN){
				for(int a = 0; a < affected_final; a++){
					int b = (policy_round_robin_var + a) % numtasks;
					scatterBuffer[b * bufferSize] = ENI_INTERFERE;
					scatterBuffer[b * bufferSize + 1] = intervallFinal(intervall_time_rnd, intervall_time, intervall_time_max);
				}
				policy_round_robin_var = (policy_round_robin_var + affected_final) % numtasks;
			} else if(designation_policy == POLICY_RANDOM){
				for(int a = 0; a < affected_final; a++){
					int rnd = (int) (rndNum() * (numtasks+1));
					
					// shift if selected node is already interfering
					int c = rnd * bufferSize;
					for(int b = 0; b < a; a++){
						if(scatterBuffer[c] == ENI_SLEEP){
							break;
						} else {
							c = (c + bufferSize) % (numtasks * bufferSize);
						}
					}				
					scatterBuffer[c] = ENI_INTERFERE;
					scatterBuffer[c + 1] = intervallFinal(intervall_time_rnd, intervall_time, intervall_time_max);	
				
				}
			}else {
				scatterBuffer[0] = ENI_INTERFERE;
			}
			
			//send
			MPI_Scatter(scatterBuffer, bufferSize, MPI_FLOAT, inbuffer, bufferSize, MPI_FLOAT, 0, MPI_COMM_WORLD);
		} else if (rank != 0) {
			MPI_Scatter(scatterBuffer, bufferSize, MPI_FLOAT, inbuffer, bufferSize, MPI_FLOAT, 0, MPI_COMM_WORLD);
		}
		
		Clock::time_point t1 = Clock::now();
		// interference
		// initially run on all nodes to start OMP
		if(inbuffer[0] == ENI_INTERFERE || x == 0){
			float total = inbuffer[1];
			if(total > 10)
				total = 10;
			else if(x == 0)
				total = 1;
			while (total > 0.1){
				int var;
				if(total > 1){
					var = intervalBase * step_length;
				} else {
					var = (int)(total * intervalBase * step_length);
				}
				total--;
				
				if(!use_timed_loop){					
					std::thread interf_thread(interferenceLoop, function_type, var);	
					interf_thread.detach();
				} else {
					std::thread interf_thread(interferenceTimed, function_type, var);	
					interf_thread.detach();				
				}
			}
			std::cout << "#" << rank << " n:" << x << "\t #interfed with " << inbuffer[1] << "\n";
		} else {
			std::cout << "#" << rank << " n:" << x << "\n";
		}
		
		// wait for rest of the timestep
		microsec ns = timeInterv(t0);
		int remainingInterv = intervalBase * step_length - ns.count();
		if(remainingInterv > 1){
			std::this_thread::sleep_for(microsec(remainingInterv));
		}
	}
	
	free(scatterBuffer);
	MPI_Finalize();
}


