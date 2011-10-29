/**
Ideia principal desse codigo:

1. A estrutura principal de armazenamento é um vetor fixo cujo tamanho e o mesmo que o numero de funcoes.
	1.1 Cada posicao do vetor deve conter um ponteiro para char cujo tamanho sera alocado dinamicamente.
		1.1.1 O vetor char nada mais e que uma String que contem exclusivamente uma funcao de uma linha do arquivo
2. Um processo root_rank 0 devera usar o vetor obtido em 1 e executar MPI_BCast enviando uma funcao para cada processo child
3. Cada processo child devera ter 3 threads..

Obs: Por enquanto vou considerar que existem no codigo apenas um processo master e um processo slave dotado de 3 threads.
**/
 

//Libs
#include <mpi.h>   
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>   

//Constants
#define ROOT_RANK 0   

//Main code
int main(int argc, char *argv[]){   
	
	//Variables
	int myRank,numberOfMPIProcesses;   

  
	//Initialize the environment   
	MPI_Init(&argc, &argv); 
	
	//How many processes are associated with my standard communicator MPI_COMM_WORLD?
	MPI_Comm_size(MPI_COMM_WORLD,&numberOfMPIProcesses);   
	
	//I need a rank for this communicator so this application can tell if I am the master or just some slave. Set my rank.
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);   
  
	//Am I the root process or just one of its slave?
	if(myRank==ROOT_RANK)
	{   
		printf("I am the master process and my rank is %d!\n",myRank);

	}
	else 
		printf("I am a slave and my rank is %d!\n",myRank);
	
	//End the parallel section of the code based on MPI. 
	MPI_Finalize();	
	//End execution.
	return (0);
  }