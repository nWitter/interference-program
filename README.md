# interference-program
### 1. Dowload
git clone https://github.com/nWitter/interference-program
### 2. Build
make
### 3. Run
run the program directly: ./MPI_Manager
or use the SLUMRM_example

### Available Arguments:
-a {n} || --affected {n}
-ar {n} {m} || --affectedRnd {n} {m}
nodes affected simultaniously by interference

-i {n} || --intervall {n}
-ir {n} {m} || --intervallRnd {n} {m}
the timefraction per step the interference program is running, values greater 1 run multiple instances
e.g. -intervall 0.5 -> runs interference for 0.5s in each 1s timestep(default)

policy: random (default), last assigned policy takes priority
-rr || --round_robin
-f || --fixed_nodes
--policy_random

-step {n} || --step_length {n}
set length of the timestep in seconds; default: 1
example: -step 0.5 -> timestep of 0.5s

--duration {n}
number of steps executed; interference runs indefenitely if no limit is specified. Total runtime = duration * step length

--seed {n}
seed for random numbers

--function {n}
change function used in interference
1 calculation heavy (default)
2 memory heavy
3 mixed

--timed_loop
using timed interference variant
