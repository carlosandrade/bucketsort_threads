/******************************************************************************
* FILE: buffsend.c
* DESCRIPTION:
*   Demonstrates MPI buffered send operations  
* LAST REVISED: 3/25/99 Blaise Barney
******************************************************************************/

#include "mpi.h" 
#include <stdio.h>
#include <stdlib.h>

#define NELEM 100000

int main(argc,argv)
int argc;
char *argv[];
{

int     numtasks, rank, rc, i, dest=1, tag=111, source=0, size;
double  data[NELEM], result;
void    *buffer;

MPI_Status status;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&rank);

if (numtasks != 2) {
  printf("Please run this test with 2 tasks. Terminating\n");
  MPI_Finalize();
  }
printf ("MPI task %d started...\n", rank);


/******************************* Send task **********************************/
if (rank == 0) {

  /* Initialize data */
  for(i=0; i<NELEM; i++)
       data[i] =  (double)random();

  /* Determine size of buffer needed including any required MPI overhead */
  MPI_Pack_size (NELEM, MPI_DOUBLE, MPI_COMM_WORLD, &size);
  size = size +  MPI_BSEND_OVERHEAD;
  printf("Using buffer size= %d\n",size);

  /* Attach buffer, do buffered send, and then detach buffer */
  buffer = (void*)malloc(size);
  rc = MPI_Buffer_attach(buffer, size);
  if (rc != MPI_SUCCESS) {
    printf("Buffer attach failed. Return code= %d Terminating\n", rc);
    MPI_Finalize();
    }
  rc = MPI_Bsend(data, NELEM, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
  printf("Sent message. Return code= %d\n",rc);
  MPI_Buffer_detach(&buffer, &size);
  free (buffer);


  }


/****************************** Receive task ********************************/
if (rank == 1) {
  MPI_Recv(data, NELEM, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &status);
  printf("Received message. Return code= %d\n",rc); 
  }


MPI_Finalize();
}
