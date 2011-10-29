#include "mpi.h"
#include <stdio.h>
	#include <stdlib.h>


int main(argc,argv) 
int argc;
char *argv[];  {
int numtasks, rank, dest, source, rc, count, tag=1;  

MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  dest = 1;
  source = 1;

char teste[5][4];

/*
int i;
	char **teste = malloc(3 * sizeof(char *));
	teste[0] = malloc(3 * 3 * sizeof(char));
	for(i = 1; i < 3; i++)
		teste[i] = teste[0] + i * 3;
*/

teste[0][0] = 'f';
teste[0][1] = 'u';
teste[0][2] = 'n';
teste[0][3] = 'f';

teste[1][0] = 'm';
teste[1][1] = 'n';
teste[1][2] = 'b';
teste[1][3] = 'v';
	
char inmsg[3][3], outmsg[3][3];
	
	
	
//	printf("Rank 0 inmsg before: %s\n",inmsg);
//	printf("Rank 0 outmsg before : %s\n",outmsg);
	
	
  rc = MPI_Send(&teste[0], 6, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  rc = MPI_Recv(&inmsg, 2, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);

//	printf("Rank 0 inmsg after: %s\n",inmsg);
//	printf("Rank 0 outmsg after : %s\n",outmsg);
  } 

else if (rank == 1) {
  dest = 0;
  source = 0;
char inmsg[3][3];
	char outmsg[3][3];
/*	da pau se usar isso!!
	char** inmsg = calloc(3, sizeof(char*));
	int i;
	for(i=0; i<3; i++)
	  inmsg[i] = calloc(3, sizeof(char));
	*/	int i;
	
	/*
	int **inmsg = malloc(3 * sizeof(char *));
	for(i = 0; i < 3; i++)
		inmsg[i] = malloc(3 * sizeof(char));

	*/
	
/*	
	char **inmsg = malloc(3 * sizeof(char *));
	inmsg[0] = malloc(3 * 3 * sizeof(char));
	for(i = 1; i < 3; i++)
		inmsg[i] = inmsg[0] + i * 3;
*/		
	inmsg[0][0] = '2';
	inmsg[0][1] = '2';
	inmsg[0][2] = '2';
	inmsg[0][3] = '2';
	
	inmsg[1][0] = '2';
	inmsg[1][1] = '2';
	inmsg[1][2] = '2';
	inmsg[1][3] = '2';
	
	inmsg[2][0] = '2';
	inmsg[2][1] = '2';
	inmsg[2][2] = '2';
	inmsg[2][3] = '2';

	int j;
	
	for(j=0;j<3;j++)
		printf("%c",inmsg[0][j]);
	printf("\n");

	for(j=0;j<3;j++)
		printf("%c",inmsg[1][j]);
	printf("\n");

	for(j=0;j<3;j++)
		printf("%c",inmsg[2][j]);
	printf("\n");
	
//	printf("Rank 1 inmsg before: %s\n",inmsg);
//	printf("Rank 1 outmsg before : %s\n",outmsg);
	
  rc = MPI_Recv(&inmsg, 5000, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);


for(j=0;j<3;j++)
	printf("%c",inmsg[0][j]);
printf("\n");

for(j=0;j<3;j++)
	printf("%c",inmsg[1][j]);
printf("\n");

for(j=0;j<3;j++)
	printf("%c",inmsg[2][j]);


  rc = MPI_Send(&outmsg, 2, MPI_CHAR, dest, tag, MPI_COMM_WORLD);

printf("\n");
	
printf("chegou vivo aqui\n");



//printf("Rank 1 after outmsg: %s\n",outmsg);
  }

rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
printf("Task %d: Received %d char(s) from task %d with tag %d \n",
       rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);


MPI_Finalize();
}