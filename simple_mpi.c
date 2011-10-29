//Source: https://computing.llnl.gov/tutorials/mpi/#Getting_Started
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_message_function( void *ptr );

  int main(argc,argv) 
  int argc;
  char *argv[]; {
  int  numtasks, rank, rc; 


pthread_t thread1, thread2;
char *message1 = "Thread 1";
char *message2 = "Thread 2";
int  iret1, iret2;

//iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
//iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);


//pid_t pID = fork();




  rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS) {
    printf ("Error starting MPI program. Terminating.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
    }

  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  printf ("Number of tasks= %d My rank= %d\n", numtasks,rank);

  /*******  do some work *******/

  MPI_Finalize();
  }



void *print_message_function( void *ptr )
{

     char *message;

     message = (char *) ptr;

     printf("%s \n", message);

}
