#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
	int tasks, rank, rc, x;
	char *proccess;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;
	
	rc = MPI_Init(&argc, &argv);
	if(rc == MPI_SUCCESS) {
		MPI_Comm_size(MPI_COMM_WORLD, &tasks);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		
		proccess = (char*) malloc(tasks * MPI_MAX_PROCESSOR_NAME * sizeof(char*));

		MPI_Get_processor_name(processor_name, &x);
		printf("My rank: %d\nMy hostname: %s\n", rank, processor_name);
		
		MPI_Allgather(processor_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, proccess, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, MPI_COMM_WORLD);
		
		for(x=0; x<tasks; x++) {
			printf("My rank %d\nMy neighbor: %d => %s\n", rank, x, &proccess[x * MPI_MAX_PROCESSOR_NAME]);
		}
	} else {
		printf("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	MPI_Finalize();
	
	return 0;
}
