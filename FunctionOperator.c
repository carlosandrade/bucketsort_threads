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

Na verdade, os tasks receivers nao se importam muito com o conteudo da mensagem FIM apesar dessa verificacao ser facil,
o que esta importando e o uso da TAGFIM. Ela tambem e necessaria para que o processo mestre ao receber os pedidos de dados
nao confunda a sinalizacao de um processo ao acabar o calculo e acabe lhe mandando outra mensagem. 
**/
 

//Libs
	#include <mpi.h>   
	#include <stdio.h>   
	#include <stdlib.h>
	#include <string.h>   
	#include <pthread.h>
//	#include <dos.h>  /* for delay */


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
int calculateFunction(char operador, char operando1[], char operando2[]);


//Metodo de threads

void* tPerformSlaveWriterTasks(void* data);
void* tPerformSlaveCalculatorTasks(void* data);
void* tPerformSlaveRequesterTasks(void* data);
void* tPerformSlaveCalculatorTasksOver();


//to use itoa which is not portable
void strreverse(char* begin, char* end);
void itoa(int value, char* str, int base);


//Necessary information among the threads
		 struct slaveTasksData{
			char result[150][80]; //Should be shared between the requester and the calculator
			int results;//Should be shared between the calculator and the writer
			int myRank; //Just for identification inside the thread of the current MPI_Process
			int sizeSlaveArray;
			char array[150][80];
		//	char **line;
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
		int 	currentDataPosition 	= 	0;

		//Content of the message sent
		int 	messageImSending 		=	9;
		int		destinationRank;
		
		//For sending FIM message
		int slaveRank;
		int numberOfSlaveProcess = numberOfSlavesMasterShouldListen;
		
		//For knowing which slave process need more functions than others if numFuncoes % numberOfSlaveProcess != 0
		int slaveNeedExtraTask = 0;
		
		
		//For file management
		FILE *fr;            /* declare the file pointer */

		printf("I am the master process and my rank is %d!\n",myRank);
		
		//I need to start reading the functions from file, here I go..!
		fr = fopen ("RandomOperatorInput.txt", "rt");  /* open the file for reading */
		
	   	int numFuncoes;
	   	
	   	char numElemen[100];
	   
		fgets(numElemen, sizeof(numElemen), fr);
		numFuncoes = atoi(numElemen);
		char line[numFuncoes][80];
		
		int j=0;
	 	while( (fgets(line[j], sizeof(numElemen), fr) != NULL) && j < numFuncoes ) 
			j++;
	
		fclose(fr);  /* close the file prior to exiting the routine */
	   
		//Alright, I, the master, am done loading all the text file. Now I need to separate the original data to my slaves.
		
		//Well, I must give each slave process the same or almost the same quantity to work on so:
		for(j=0;j<numFuncoes;j++)
			puts(line[j]);
		
		if(numberOfSlaveProcess!=0)
			sTasksData.sizeSlaveArray = numFuncoes/numberOfSlaveProcess;   
			
		printf("Aqui size salve array e: %d\n",sTasksData.sizeSlaveArray);

		//Given that the amount of functions in this assignment is always more than the number of tasks..
		if(sTasksData.sizeSlaveArray*numberOfSlaveProcess < numFuncoes)
		{ 	
			//Every task should at least have numFuncoes/numberOfSlaveProcess functions to deal with
			//If I got inside this condition that means there are still functions that are not allocated
			//to any slave process, but this amount of functions is less than the amount of tasks
			//otherwise  either numFuncoes/numberOfSlaveProcess would be higher or a perfect division! 
			
			//That being said, mathematically on the worst situation numberOfSlaves - 1 would have one extra task
			//and that would be enough so all functions are allocated
			
			//This way we guarantee that all tasks have the same amount of functions to deal with aside from the difference
			//of one
			
			//Now when I, the master, send messages, I must tell the send process to send one more function 
			//to few slave process
			
			//Now, since each slave only requests a task ONCE, a counter of amount of slaves that need extra task suffices
				slaveNeedExtraTask = numFuncoes % numberOfSlaveProcess;
				
			
		}

		//alocando o vetor de resultado (contera as posicoes onde se encontra X)  int res[n]
		//res = (int *) calloc(n,sizeof(int));  
		
		if(numberOfSlaveProcess!=0)
			printf("Sub-vector sizes: %d\n",sTasksData.sizeSlaveArray);
		
		

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

			//Send data to the slave process that requested it
			if(slaveNeedExtraTask > 0)
			{
				rc = MPI_Send(&line[currentDataPosition], (sTasksData.sizeSlaveArray+1)*80, MPI_CHAR, destinationRank, TAG, MPI_COMM_WORLD);
				slaveNeedExtraTask--;
				
				//I should also be careful not to send the same part of the data to different slave processes so..
				printf("Valor de currentDataPosition: %d\n",currentDataPosition);
				currentDataPosition += sTasksData.sizeSlaveArray+1;
			}
			else
			{
		  		rc = MPI_Send(&line[currentDataPosition], sTasksData.sizeSlaveArray*80, MPI_CHAR, destinationRank, TAG, MPI_COMM_WORLD);				
				//I should also be careful not to send the same part of the data to different slave processes so..
				printf("Valor de currentDataPosition: %d\n",currentDataPosition);
				currentDataPosition += sTasksData.sizeSlaveArray;
			}




			//if(currentDataPosition > numFuncoes)
			//	currentDataPosition = numFuncoes; //Not quite sure if this occurs, but just in case..; //Gets the next portion of the data

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
		
		pthread_create( &thread_id[REQUESTER], NULL, tPerformSlaveRequesterTasks, NULL);	
		
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
	
	int a=0,b=0;
	
		//Since I'm a slave, I should request my master for some data to work on.
		rc = MPI_Send(&messageImSending, numberOfMessageCopies, messageKind, destinationRank, TAG, MPI_COMM_WORLD);
		
		//Let me initialize the bidimensional array so that it will be easier to find where are the elements I want
		int i,j;
		for(i=0; i < 150 ; i++)
		{
			for(j=0;j<80;j++)
			{
				sTasksData.array[i][j] = '\n';
			}
		}
		
		//I shall slack until master send what I requested. I'm blocked until I receive the data.
		rc = MPI_Recv(&sTasksData.array, 2000, MPI_CHAR, sourceRank, TAG, MPI_COMM_WORLD, &status);
		
		
		//It is of interest to me, slave, to know how many funcions master gave me. Let me use his package count to figure it out.
		MPI_Status Stat;
		int count;
		rc = MPI_Get_count(&status, MPI_CHAR, &count);
		
		//With this, all the threads of this proces 
		sTasksData.sizeSlaveArray = count/80;  //Because I know the sender always does count = 80 * something this is okay
		printf("I am the task %d, and the amount of functions I need to deal with are: %d\n",sTasksData.myRank,sTasksData.sizeSlaveArray);
//		printf("valor de subarray de rank : %d\n",sTasksData.myRank)			;
		
/*
		//Just a print to see whats going on
		for(a=0;a < sTasksData.sizeSlaveArray ;a++)
		{ 
			for(b=0;(b<5) && (sTasksData.array[a][b] != '\n') ;b++)
			{ 	
				printf("%c",sTasksData.array[a][b]);
			}
			printf("\n");
		}						
*/
													
		
		//printf("Testando algo aqui2: %s\n",subarray[6]);													
//		printf("Testando algo aqui: %s\n",sTasksData.messageImReceiving[0]);
		
		//I should be a slave task and I should have received my data properly. 
//		for(i=0;i<2;i++)
//			printf("Azure %s\n",pointerSTasksData->messageImReceiving[i]);
			//printf("I'm the slave task %d. The data I received is %s\n",pointerSTasksData->myRank,pointerSTasksData->messageImReceiving[i]);
			
		
		pthread_exit(NULL);
	   
}
void* tPerformSlaveCalculatorTasks(void* data)
{	
	int a=0,b=0,i=0,res; //used for the method
	char operand1[17],operand2[17],cRes[17],operator;
	
	
	int k,j; //used for initialization and feeding the result bidimensional array from the structure
	
	//initialization of the bidimensional array result
	for(k=0; k < 150 ; k++)
	{
		for(j=0;j<80;j++)
		{
			sTasksData.result[i][j] = ' ';
		}
	}
	
	for(k=0;k<17;k++)
		operand1[k] = ' ';
	for(k=0;k<17;k++)
		operand2[k] = ' ';
	for(k=0;k<17;k++)
		cRes[k] = ' ';
	

	
	for(a=0;a < sTasksData.sizeSlaveArray ;a++)
	{ 
		while(sTasksData.array[a][b] == ' ')	b++;
		
		//Operator goes first
		operator = sTasksData.array[a][b];
		//printf("VALOR RECEBIDO EM OPERATOR: %c\n",operator); isso ta ok
		b++;
		while(sTasksData.array[a][b] == ' ')	b++;
		while(sTasksData.array[a][b] != ' ')
		{
			operand1[i]=sTasksData.array[a][b];
			b++;
			i++;
			
		}
	
		//printf("source: %s\n",sTasksData.array[a]);
	//	printf("op1: %s\n",operand1);
		//exit(0);
		
		i=0;
		while(sTasksData.array[a][b] == ' ') b++;
		
			//	printf("op1: %s\n",operand1);
		while((sTasksData.array[a][b] != ' ') && (sTasksData.array[a][b] != '\n') && (sTasksData.array[a][b] != EOF))
		{
			operand2[i]=sTasksData.array[a][b];
			b++;
			i++;
		}
	//			printf("op1: %s\n",operand1);
	//	printf("op2: %s\n",operand2);
//		exit(0);
		//printf("Sejam dados: %c %c %c.\n",sTasksData.array[a][0],sTasksData.array[a][2],sTasksData.array[a][4]);
//		printf("passando para calculateFunction: %c, %s, %s\n",operator,operand1,operand2);
		res = calculateFunction(operator,operand1,operand2);
		//printf("res em int %d\n",res);
		itoa(res,cRes,10); //conversao para decimal
		//printf("res: %s\n",cRes);
		
		//Results are added to the result structure	
		j=0;
		sTasksData.result[a][j] = operator;
		j++;
		sTasksData.result[a][j] = ' ';
		j++;
		for(k=0;(k<17) && (j<80) && operand1[k]!= ' ';k++,j++)
		{
			sTasksData.result[a][j] = operand1[k];	
		}
		sTasksData.result[a][j] = ' ';
		j++;
		for(k=0;(k<17) && (j<80) && operand2[k] != ' ';k++,j++)
		{
			sTasksData.result[a][j] = operand2[k];	
		}
		sTasksData.result[a][j] = ' ';
		j++;
		for(k=0;(k<17) && (j<80) && (cRes[k]!= ' ' || EOF);k++,j++)
		{
					printf("value = %s, j= %d\n",sTasksData.result[a],j);
			sTasksData.result[a][j] = cRes[k];	
		}
		puts(sTasksData.result[a]);
		sTasksData.result[a][j] = '\n';
		
		//	b=0;i=0;
	//	}
		
		
		//for(k=0;k<17;k++)
			//printf("op1 %c",operand1[k]);
		//printf("op1: %s\n",operand1);
	
		//Just to see if all went ok
//		printf("Linha de resultado: %s\n",sTasksData.result[a]);
		/*
		printf("Valor da linha %d do arquivo resposta: ",a);
		for(k=0;k<80;k++)	
			printf("%c",sTasksData.result[a][k]);
		printf("\n");
		b=0; i=0;

		*/
		b=0;i=0;
	}
		
		
//	int i=0,j=0;
//	for(i=0;i<)
//	while(sTasksData.array[i][j])
//	sTasksData.array
	//struct slaveTasksData * pointerSTasksData = (struct slaveTasksData *)data;
	//pointerSTasksData->results = pointerSTasksData->messageImReceiving;
	//pointerSTasksData->results += 10;
	//printf("Resultado obtido pelo processo de rank %d e: %d\n",pointerSTasksData->myRank,pointerSTasksData->results);
	//pthread_exit(NULL);
	
}
int calculateFunction(char operador, char operando1[], char operando2[])
{ int op1,op2,res;
 	op1 = atoi(operando1);
 	op2 = atoi(operando2);

	switch(operador)
	{
		case '+':
			res = op1 + op2;
		break;
		case '-':
			res = op1 - op2;
		break;
		case '*':
			res = op1 * op2;
		break;
		case '/':
			if(op2!=0)
				res = op1 / op2;
			else
				res = 0;
		break;
		case '%':
			res = op1 % op2;
		break;
			default:
			printf("Operador '%c' nao suportado, erro na funcao especificacada \n op1: %s, op2: %s, slave %d\n",operador,operando1,operando2,sTasksData.myRank);
			exit(0);
	}
	return res;
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
	int i=0;

	for(i=0;i<sTasksData.sizeSlaveArray;i++)
		printf("Linha de resultado no writer: %s\n",sTasksData.result[i]);			
		
	//Operacoes para salvar resultado em arquivo
	
	FILE *p;
	
	char slave[6] = "slave\0";
	char nomeArq[80];

	//Define file name as slave<slaveRank>.txt
	sprintf(nomeArq,"%s%d.txt",slave,sTasksData.myRank);

	if (!(p = fopen(nomeArq,"w")))  /* Caso ocorra algum erro na abertura do arquivo..*/ 
  	{                           /* o programa aborta automaticamente */
  		printf("Erro! Impossivel abrir o arquivo!\n");
  		exit(1);
  	}
	/* Se nao houve erro, imprime no arquivo, fecha ...*/
	for(i=0;i<sTasksData.sizeSlaveArray;i++)
		fprintf(p,"%s\n",sTasksData.result[i]);
	fclose(p);
/*
	// abre novamente para a leitura 
	p = fopen(str,"r");
	while (!feof(p))
 	{
  		fscanf(p,"%c",&c);
  		printf("%c",c);
  	} 
	fclose(p);
	
*/	
	pthread_exit(NULL);
	
}



/**
	
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C"
	
 * with slight modification to optimize for specific architecture:
	
 */
	
void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}
	
void itoa(int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	char* wstr=str;
	
	int sign;
	
	div_t res;
	

	
	// Validate base
	
	if (base<2 || base>35){ *wstr='\0'; return; }
	

	
	// Take care of sign
	
	if ((sign=value) < 0) value = -value;
	

	
	// Conversion. Number is reversed.
	
	do {
	
		res = div(value,base);
	
		*wstr++ = num[res.rem];
	
	}while(value=res.quot);
	
	if(sign<0) *wstr++='-';
	
	*wstr='\0';
	

	
	// Reverse string
	
	strreverse(str,wstr-1);
	
}