#!/bin/bash
#########################
#
# script starting the matrix-example with interference, used in the slurm example
#

MATRIX_PATH=../chameleon-apps/applications/matrix_example/main

echo $@
echo "starting"
./MPI_Manager $@ &

#task distribution
TSK="100"
if [[ $SLURM_NTASKS == "2" ]]; then
	TSK="100 100"
fi
if [[ $SLURM_NTASKS == "4" ]]; then
	TSK="100 100 100 100"
fi
if [[ $SLURM_NTASKS == "8" ]]; then
	TSK="100 100 100 100 100 100 100 100"
fi
if [[ $SLURM_NTASKS == "16" ]]; then
	TSK="100 100 100 100 100 100 100 100 100 100 100 100 100 100 100 100"
fi

$MATRIX_PATH $1 $TSK > $2

exit