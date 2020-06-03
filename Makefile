CXX      := -c++
MPICXX   := mpiCC
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fopenmp -std=gnu++11
LDFLAGS  := -L/usr/lib -lstdc++ -lm

all: functions.o simple_loop.o timed_loop.o MPI_Manager.o MPI_Manager

functions.o: functions.h
	$(CXX) $(CXXFLAGS) -c functions.cpp
	
simple_loop.o: functions.h simple_loop.h
	$(CXX) $(CXXFLAGS) -c simple_loop.cpp

timingThread.o: simple_loop.h functions.h
	$(CXX) $(CXXFLAGS) -c timingThread.cpp

MPI_Manager.o: functions.h simple_loop.h timed_loop.h
	$(MPICXX) $(CXXFLAGS) -c MPI_Manager.cpp

MPI_Manager: MPI_Manager.o functions.o simple_loop.o timed_loop.o
	$(MPICXX) $(CXXFLAGS) -o MPI_Manager MPI_Manager.o functions.o simple_loop.o timed_loop.o
	


debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -fr functions.o
	-@rm -fr functions
	-@rm -fr simple_loop.o
	-@rm -fr simple_loop
	-@rm -fr timed_loop.o
	-@rm -fr timed_loop
	-@rm -fr MPI_Manager.o
	-@rm -fr MPI_Manager
	
clearOUT:
	-@rm eni*.out
	
clearTrace:
	-@rm main.*