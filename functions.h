#ifndef FUNCTIONS
#define FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <vector>
#include <chrono>
#include <iostream>

typedef std::chrono::steady_clock Clock;
typedef std::chrono::microseconds microsec;

const int vector_size = 1 << 10;

int interference_function(int func, Clock::time_point tZero, microsec activeT);
void interference_function_fixed_length(int func, int loop_length, int repetitions);
void interference_single(int* vec, int len, int func, int rep);

void functionMixed(int* v, int x);
void functionCalc(int* v, int x);
void functionMemory(int* v, int x);

void functionCalcSingle(int sc);

microsec timeInterv(Clock::time_point tZero);


#endif