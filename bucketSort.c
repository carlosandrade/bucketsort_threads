#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define ROOT_RANK 0   


void splitElements(char ** unsorted);
int decidePosicaoDoBucket(int elementoAtual,int numFuncoes);
void separaElementosDoVetorNosBuckets(int *vetor,int bucket[300][16], int numProcessos, int posicao[], int numFuncoes);
void performMasterTasks(int numberOfSlavesMasterShouldListen, int numberOfSlavesMasterShouldWaitForCalculation, int myRank, int rc);
void performSlaveTasks(int myRank, int rc);

int main(int argc, char *argv[]) 
{
	int numberOfSlavesMasterShouldListen, numberOfMPIProcesses, numberOfSlavesMasterShouldWaitForCalculation;
	int myRank,rc;
	
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
	
	
	return 0;	



	
	
		

}

void performMasterTasks(int numberOfSlavesMasterShouldListen, int numberOfSlavesMasterShouldWaitForCalculation, int myRank, int rc)
{
		//For file management
		FILE *fr;            /* declare the file pointer */
		
		//to use on for
		int i;



//		printf("I am the master process and my rank is %d!\n",myRank);
		
		//I need to start reading the functions from file, here I go..!
		
		
		if(!(fr=fopen("randomInput4.txt", "rt")))  /* open the file for reading */
		{
			printf("Erro! Impossivel abrir o arquivo!\n");
		  	exit(1);	
		}
	   	int numFuncoes,numProcessos=10;
	   	char numElemen[100];

		fgets(numElemen, sizeof(numElemen), fr);
		numFuncoes = atoi(numElemen);
		char line[numFuncoes][80];
		int unsort[numFuncoes];
		int bucket[300][16];
		int j=0;
	 	while( (fgets(line[j], sizeof(numElemen), fr) != NULL) && j < numFuncoes ) j++;
	
		fclose(fr);  /* close the file prior to exiting the routine */


//		for(i=0;i<numFuncoes;i++)
//			printf("Posicoes no vetor: %s\n",line[i]);
		for(i=0;i<numFuncoes;i++)
			unsort[i] = atoi(line[i]);
//		for(i=0;i<numFuncoes;i++)
//			printf("%d\n",unsort[i]);
		
		//para saber o num de elementos de cada bucket
		int numElemenBucket[numProcessos];
		separaElementosDoVetorNosBuckets(unsort,bucket,numProcessos,numElemenBucket,numFuncoes);

	
	for(i=0;i<numProcessos;i++)
	{
		printf("size: %d, bucket[%d] = ",numElemenBucket[i],i);
		for(j=0;j<numElemenBucket[i];j++)
			printf("%d|",bucket[i][j]);
		printf("\n");
	}
	
		
}

void performSlaveTasks(int myRank, int rc)
{

}


void separaElementosDoVetorNosBuckets(int *vetor,int bucket[300][16], int numProcessos, int posicao[16],int numFuncoes)
{int i,j;
	int posicaoBucket;
	
	for(i=0;i<numProcessos;i++)
		posicao[i] = 0;
	
    for(i = 0; i<numFuncoes;i++)
	{
		posicaoBucket = decidePosicaoDoBucket(vetor[i],numProcessos);
		bucket[posicaoBucket][posicao[posicaoBucket]] = vetor[i];
		posicao[posicaoBucket]++;
	}
	//for(i=0;i<numFuncoes;i++)
	//	printf("%d\n",posicao[i]);
/*		
	for(i=0;i<numProcessos;i++)
	{
		printf("size: %d, bucket[%d] = ",posicao[i],i);
		for(j=0;j<posicao[i];j++)
			printf("%d|",bucket[i][j]);
		printf("\n");
	}
*/

}
int decidePosicaoDoBucket(int elementoAtual,int numProcessos)
{
	int i=0;
	    if(elementoAtual == 0)
	        return numProcessos/2;

	    int elementoAtualEmModulo = elementoAtual;

	    if(elementoAtualEmModulo<0)
	        elementoAtualEmModulo = elementoAtualEmModulo*-1;

	    for(;i<(numProcessos/2);i++)
	    {
	        if(elementoAtualEmModulo < ((5000/((float)numProcessos/2))*(i+1)))
	        {
	            if(elementoAtual < 0)
	                return((numProcessos/2) - (i+1));
	            else
	                return((numProcessos/2) + i);
	        }
	    }
	    if(elementoAtual < 0)
	        return (numProcessos/2 - i); 
	    else
	        return((numProcessos/2) + i-1); //So ocorre quando elemento igual a 5000	
}