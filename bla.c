#include <mpi.h>   
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>   
#include <pthread.h>


 struct slaveTasksData{
	int messageImReceiving; //Should be shared between the requester and the calculator
	int results;//Should be shared between the calculator and the writer
	int myRank; //Just for identification inside the thread of the current MPI_Process
	int sizeSlaveArray;
	char **line;
}sTasksData;


int main(int argc, char *argv[]){   
		//I need to start reading the functions from file, here I go..!
		FILE * fr = fopen ("RandomOperatorInput.txt", "rt");  /* open the file for reading */
		
	   	int numFuncoes;
	   	
	   	char numElemen[100];
	   

		fgets(numElemen, sizeof(numElemen), fr);
		printf("numelemen: %s\n",numElemen);
		
		numFuncoes = atoi(numElemen);
		char line2[numFuncoes][80];
		printf("numfuncoes: %d\n",numFuncoes);
		
		sTasksData.line = (char **) calloc(1,400);  ;//malloc(sizeof(char*)*numFuncoes*80);
		printf("line2: %d, e line: %d\n",sizeof(line2),sizeof(sTasksData.line));
//		exit(0);
		
		int j=0;
	 	while( (fgets(sTasksData.line[j], sizeof(numElemen), fr) != NULL) && j < numFuncoes ) 
			j++;
	
		fclose(fr);  /* close the file prior to exiting the routine */

}
