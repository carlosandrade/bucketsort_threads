/**
Ideia principal desse codigo:

<Isso ja foi modificado consideravelmente, preciso reescrever>

1. A estrutura principal de armazenamento é um vetor fixo cujo tamanho e o mesmo que o numero de funcoes.
	1.1 Cada posicao do vetor deve conter um ponteiro para char cujo tamanho sera alocado dinamicamente.
		1.1.1 O vetor char nada mais e que uma String que contem exclusivamente uma funcao de uma linha do arquivo
2. Um processo root_rank 0 devera usar o vetor obtido em 1 e executar MPI_BCast enviando uma funcao para cada processo child
3. Cada processo child devera ter 3 threads..

Obs: Por enquanto vou considerar que existem no codigo apenas um processo master e um processo slave dotado de 3 threads.


Notar que as mensagens trocadas entre mestre e escravo foram classificadas atraves de TAG e TAGFIM

TAG sao usadas para as comunicacoes normais de passagem de funcao do mestre para os escravos
TAGFIM e usado pelos escravos para que o mestre saiba quando cada uma delas terminou a thread de calculo, 
uma vez que ela tenha terminado uma mensagem e enviada pelo mestre que identificara a TAGFIM e por sua vez
enviara a mensagem FIM conforme a especificacao da atividade apenas TODOS os escravos tiverem terminado o calculo

Por sua vez, um processo escravo so sai do metodo SlaveTasks apos receber essa mensagem por conta da mesma ser bloqueante

Isso garante que o barrier do processo escravo so seja executado apos o recebimento da mensagem fim ao processo escravo retornar
para main, o que tambem fica em conformidade com a especificacao
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
#define TAGFIM 2 //Will be used for fin messages

	//Will be used to identify the threads
		#define REQUESTER 0
		#define CALCULATOR 1
		#define WRITER 2

void performMasterTasks(int numberOfSlavesMasterShouldListen,int numberOfSlavesMasterShouldWaitForCalculation, int myRank, int rc);
void performSlaveTasks(int myRank, int rc);

//Metodo de threads

void* tPerformSlaveWriterTasks(void* data);
void* tPerformSlaveCalculatorTasks(void* data);
void* tPerformSlaveRequesterTasks(void* data);
void* tPerformSlaveCalculatorTasksOver();


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
	int numberOfSlavesMasterShouldWaitForCalculation;

	//Initialize the environment   
	MPI_Init(&argc, &argv); 
	
	//How many processes are associated with my standard communicator MPI_COMM_WORLD?
	MPI_Comm_size(MPI_COMM_WORLD,&numberOfMPIProcesses);   
	numberOfSlavesMasterShouldListen = numberOfMPIProcesses - 1; //The master wont listen to himself.
	numberOfSlavesMasterShouldWaitForCalculation = numberOfSlavesMasterShouldListen;
	
	//I need a rank for this communicator so this application can tell if I am the master or just some slave. Set my rank.
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);   
  
	//Am I the root process or just one of it's slave?
	if(myRank==ROOT_RANK)
		performMasterTasks(numberOfSlavesMasterShouldListen,numberOfSlavesMasterShouldWaitForCalculation, myRank, rc);
	else 
		performSlaveTasks(myRank,rc);
		
	//On this point, I, as a process (master or slave) must wait my comrades to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	  	
	//End the parallel section of the code based on MPI. 
	MPI_Finalize();	
	
	//End execution.
	return (0);
}


void performMasterTasks(int numberOfSlavesMasterShouldListen, int numberOfSlavesMasterShouldWaitForCalculation, int myRank, int rc)
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
		
		//For sending FIM message
		int slaveRank;
		int numberOfSlaveProcess = numberOfSlavesMasterShouldListen;
		


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
		while(numberOfSlavesMasterShouldWaitForCalculation > 0)
		{
			//Since I'm the master, I should now wait for my slaves to tell me that they finished calculating the data 
		  	rc = MPI_Recv(&messageImReceiving, 1, MPI_CHAR, MPI_ANY_SOURCE, TAGFIM, MPI_COMM_WORLD, &status);
			numberOfSlavesMasterShouldWaitForCalculation--;
			
			printf("Number of calculation tasks from my slaves remaining: %d\n",numberOfSlavesMasterShouldWaitForCalculation);
		}
		//Only after all my slaves finished calculation I should tell them that it is over and that they must cast a barrier
		printf("At this point, I, the master process know that all calculation is over by my slave tasks as they notified me\n");
		messageImSending = 'f';
		for(slaveRank= 1; slaveRank <= numberOfSlaveProcess ; slaveRank++)
		{
			printf("Issuing FIM message for slave process whose rank is: %d\n",slaveRank);
			rc = MPI_Send(&messageImSending, numberOfMessageCopies, MPI_CHAR, slaveRank, TAGFIM, MPI_COMM_WORLD);
		}
			
		printf("I, the Master Process, finished issuing all the FIM messages to all my slaves so they may execute barrier \n");
		
	  	
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
		pthread_create( &thread_id[CALCULATOR], NULL, tPerformSlaveCalculatorTasks, pointerSTasksData);	
		
		//I must wait the calculator to calculate the data before I attempt to write the results to the file
		pthread_join( thread_id[CALCULATOR], NULL);
		
		//I must tell my master that I finished the calculation so that he may issue me a FIM message
		pthread_create( &thread_id[REQUESTER], NULL, tPerformSlaveCalculatorTasksOver, NULL);	
		
		//If I reached this point, that means my slave comrades all finished calculating because master issued me a FIM msg
		
		//Since I have the results, I may write it to the file
		pthread_create( &thread_id[WRITER], NULL, tPerformSlaveWriterTasks, pointerSTasksData);	
	
		//Do note that I'll not exit until master tell me a FIM msg, so I'll never return to main and cast my slave barrier
		//until that occurs according to the assignment specification
	   	pthread_exit(NULL);
	
		
}
void* tPerformSlaveRequesterTasks(void* data)
{
//	struct slaveTasksData* slaveTasksData;
	
	struct slaveTasksData * pointerSTasksData = (struct slaveTasksData *)data;
		
		//General message information
		int 	numberOfMessageCopies	=	1;
		int		messageKind				=	MPI_CHAR;
		int 	rc;
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
void* tPerformSlaveCalculatorTasks(void* data)
{
	struct slaveTasksData * pointerSTasksData = (struct slaveTasksData *)data;
	pointerSTasksData->results = pointerSTasksData->messageImReceiving;
	pointerSTasksData->results += 10;
	printf("Resultado obtido pelo processo de rank %d e: %d\n",pointerSTasksData->myRank,pointerSTasksData->results);
	pthread_exit(NULL);
	
}
void* tPerformSlaveCalculatorTasksOver()
{
		//General message information
		int 	numberOfMessageCopies	=	1;
		int		messageKind				=	MPI_CHAR;
		int 	rc;
		MPI_Status status; //This is a necessary parameter but won't be used here
		
		//Content of the message sent
		char	messageImSending 		=	'd';
		int		destinationRank			=	0;
		
		//Content of the message received
		int		sourceRank				=	0;
		char messageImReceiving;
		
		//I'll tell master the calculator was over with this TAGFIM message 
		rc = MPI_Send(&messageImSending, numberOfMessageCopies, messageKind, destinationRank, TAGFIM, MPI_COMM_WORLD);
		
		//Master must tell me the FIM message with TAGFIM so I may proceed and cast my barrier, until then I block.
		rc = MPI_Recv(&messageImReceiving, numberOfMessageCopies, MPI_CHAR, sourceRank, TAGFIM, MPI_COMM_WORLD, &status);
		
	
}
void* tPerformSlaveWriterTasks(void* data)
{
	struct slaveTasksData * pointerSTasksData = (struct slaveTasksData *)data;
	int resultado = pointerSTasksData->results;
	
	//Operacoes para salvar resultado em arquivo
	
	pthread_exit(NULL);
	
}
