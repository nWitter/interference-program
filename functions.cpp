#include "functions.h"

int interference_function(int func, Clock::time_point tZero, microsec activeT){
	int vector[vector_size];
	int cnt = 0;
	int b;
	for (int i = 0; i < vector_size; i++)
		vector[i] = 1.01;
	#pragma omp parallel for default(none) shared(vector, tZero, activeT, func, cnt) private(b)
	for (b = 0; b < vector_size; b++){		
		//int t = omp_get_thread_num();
		//printf("%d -- %d;\n", b, t);	
		while (timeInterv(tZero) < (activeT)) {
			for (int a = 0; a < vector_size; a++) {
				if(func == 1){
					functionCalc(vector, a);
				} else if(func == 2){
					functionMemory(vector, a);
				} else {
					functionMixed(vector, a);
				}
			}
		cnt++;
		}
	}
	return cnt;
}

void interference_function_fixed_length(int func, int loop_length, int repetitions){
	int vec[vector_size];
	for (int i = 0; i < vector_size; i++)
		vec[i] = 1.0;
	
	for (int b = 0; b < repetitions; b++) {
		interference_single(vec, vector_size, func, loop_length);
	}
}

void interference_single(int* vec, int len, int func, int rep){
	#pragma omp parallel for default(none) shared(vec, len, func, rep)
	for (int a = 0; a < len; a++){
		for (int c = 0; c < rep; c++) {
			if(func == 1){
				functionCalc(vec, a);
			} else if(func == 2){
				functionMemory(vec, a);
			} else {
				functionMixed(vec, a);
			}
		}
	}	
	return;
}




// 0 default
void functionMixed(int* v, int x){
	int t = (x + vector_size/2) % vector_size;
	v[x] = (v[x] + x) * v[t];
}

// 1
void functionCalc(int* v, int x){
	v[x] = (v[x] + 1.1) * 1.1;
}

// 2
void functionMemory(int* v, int x){
	int t = (x + vector_size/2) % vector_size;
	v[x] = v[t];
}

microsec timeInterv(Clock::time_point tZero){
	microsec d = std::chrono::duration_cast<microsec>(Clock::now() - tZero);
	return d;
}