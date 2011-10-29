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
	#include <pthread.h>


//Constants
#define ROOT_RANK 0   
#define TAG 1 //Since its not important in this case the messageKey, let all of them be 1.

	//Will be used to identify the threads
		#define REQUESTER 0
		#define CALCULATOR 1
		#define WRITER 2

void performMasterTasks(int numberOfSlavesMasterShouldListen, int myRank, int rc);
void performSlaveTasks(int myRank, int rc);
void* tPerformSlaveRequesterTasks(void* data);

//Necessary information among the threads
		 struct slaveTasksData{
			int messageImReceiving; //Should be shared between the requester and the calculator
			int results;//Should be shared between the calculator and the writer
			int myRank; //Just for identification inside the thread of the current MPI_Process
		}sTasksData;


//Main code
int main(int argc, char *argv[]){   
	
	//Variables
	int myRank,numberOfMPIProcesses;   
	int rc;
	int numberOfSlavesMasterShouldListen;

	//Initialize the environment   
	MPI_Init(&argc, &argv); 
	
	//How many processes are associated with my standard communicator MPI_COMM_WORLD?
	MPI_Comm_size(MPI_COMM_WORLD,&numberOfMPIProcesses);   
	numberOfSlavesMasterShouldListen = numberOfMPIProcesses - 1; //The master wont listen to himself.
	
	//I need a rank for this communicator so this application can tell if I am the master or just some slave. Set my rank.
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);   
  
	//Am I the root process or just one of it's slave?
	if(myRank==ROOT_RANK)
		performMasterTasks(numberOfSlavesMasterShouldListen, myRank, rc);
	else 
		performSlaveTasks(myRank,rc);
	  	
	//End the parallel section of the code based on MPI. 
	MPI_Finalize();	
	
	//End execution.
	return (0);
}


void performMasterTasks(int numberOfSlavesMasterShouldListen, int myRank, int rc)
{
		//General message information
		int 	numberOfMessageCopies	=	1;
		int		messageKind				=	MPI_CHAR;
		MPI_Status status; //Necessary so the master know to which slave the requested data should be sent
		int slaveRankWhoSentTheRequest;
		int receivedAmountOfDataFromSlave;

		//Content of the message received
		char 	messageImReceiving;
		int		sourceRank				=	0;

		//Content of the message sent
		int 	messageImSending 		=	9;
		int		destinationRank;


		printf("I am the master process and my rank is %d!\n",myRank);

		//I shall be listening all my slave requests until they're all satisfied. One slave communicate with me exacly ONCE.
		while(numberOfSlavesMasterShouldListen > 0)
			{	printf("NumberOfRemainingSlaves: %d\n",numberOfSlavesMasterShouldListen);

			//Since I'm the master, I should send my slaves data to work on ONLY WHEN they request it.
		  	rc = MPI_Recv(&messageImReceiving, 1, MPI_CHAR, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);

			//Which of those slaves sent me the request?
			rc = MPI_Get_count(&status, MPI_FLOAT, &receivedAmountOfDataFromSlave);
			slaveRankWhoSentTheRequest = status.MPI_SOURCE;
			printf("The slave rank who sent the request was: %d\n",slaveRankWhoSentTheRequest);

			//I should answer him properly..
			destinationRank = slaveRankWhoSentTheRequest;

			//I should also be careful not to send the same part of the data to different slave processes so..
			messageImSending++; //Gets the next portion of the data

			//Send data to the slave process that requested it
		  	rc = MPI_Send(&messageImSending, numberOfMessageCopies, MPI_INT, destinationRank, TAG, MPI_COMM_WORLD);

			//One less slave to deal with.
			numberOfSlavesMasterShouldListen--;
		}

}
void performSlaveTasks(int myRank, int rc)
{
		printf("I am a slave and my rank is %d!\n",myRank);
		

		struct slaveTasksData * pointerSTasksData = &sTasksData;
		pointerSTasksData->myRank = myRank;
		
		
		//Let 0 be the function requester, 1 the calculator and 2 the writer of the results, see macros above
		pthread_t thread_id[3];
		
		//To get things going I should have 3 threads to help me out on this task..
		
		pthread_create( &thread_id[REQUESTER], NULL, tPerformSlaveRequesterTasks, pointerSTasksData);	
		
		//I must wait the requester to obtain the data before I move any further
		pthread_join( thread_id[REQUESTER], NULL);
		
		//Since I have the data, I may calculate it
	//	pthread_create( &thread_id[CALCULATOR], NULL, tPerformSlaveCalculatorTasks, &slaveTasksData);	
		
		//I must wait the calculator to calculate the data before I attempt to write the results to the file
	//	pthread_join( thread_id[CALCULATOR], NULL);
		
		//Since I have the results, I may write it to the file
	//	pthread_create( &thread_id[WRITER], NULL, tPerformSlaveWriterTasks, &slaveTasksData);	
	
	   pthread_exit(NULL);
	
		
}
void* tPerformSlaveRequesterTasks(void* data)
{
	struct slaveTasksData* slaveTasksData;
	
	struct slaveTasksData * pointerSTasksData = (struct slaveTasksData *)data;
		
		//General message information
		int 	numberOfMessageCopies	=	1;
		int		messageKind				=	MPI_CHAR;
		int rc;
		MPI_Status status; //This is a necessary parameter but won't be used here
		
		//Content of the message sent
		char	messageImSending 		=	'a';
		int		destinationRank			=	0;
		
		//Content of the message received
		int		sourceRank				=	0;
		
		
		//Since I'm a slave, I should request my master for some data to work on.
		rc = MPI_Send(&messageImSending, numberOfMessageCopies, messageKind, destinationRank, TAG, MPI_COMM_WORLD);
		
		//I shall slack until master send what I requested. I'm blocked until I receive the data.
		rc = MPI_Recv(&pointerSTasksData->messageImReceiving, numberOfMessageCopies, MPI_INT, sourceRank, TAG, MPI_COMM_WORLD, &status);
		
		//I should be a slave task and I should have received my data properly. 
		printf("I'm the slave task %d. The data I received is %d\n",pointerSTasksData->myRank,pointerSTasksData->messageImReceiving);
		
		pthread_exit(NULL);
	   
}
void* tPerformSlaveCalculatorTasks()
{
	
}
void* tPerformSlaveWriterTasks()
{
	
}
