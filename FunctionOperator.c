//Autor: Filipe Areias NÈvola   
//Ano: 2009   
//Programa: Usando Scatter, vamos exemplicar por meio de uma busca de posiÁıes do elemento X   
//Licensa: VocÍ pode usar e alterar, mas deve manter o Autor   
//Principal   
  
#include <mpi.h>   
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>   
#define GROUP MPI_COMM_WORLD   
#define Init(x,y) MPI_Init(x,y)   
#define Finalize() MPI_Finalize()   
#define Size(x) MPI_Comm_size(GROUP,x)   
#define Rank(x) MPI_Comm_rank(GROUP,x)   
#define Send(x, tam, tipo, destino, flag) MPI_Send(x, tam, tipo, destino, flag, GROUP)   
#define Recv(onde, tam, tipo, origem, flag) MPI_Recv(onde, tam, tipo, origem, flag, GROUP, &status)   
#define Bcast(m, x, tipo, emissor) MPI_Bcast(m, x, tipo, emissor, GROUP)   
  
#define Scatter(x,tam,tipo,subx,tamrec,tipo2,enviador) MPI_Scatter(x,tam,tipo,subx,tamrec,tipo2,enviador,GROUP)   
#define Gather(x,tam,tipo,subx,tamrec,tipo2,enviador) MPI_Gather(x,tam,tipo,subx,tamrec,tipo2,enviador,GROUP)   
  
#define INT MPI_INT   
#define ROOT_RANK 0   
  
int main(int argc, char *argv[]){   
/**declaracoes**/  
int vetor[]={1,2,3,4,5,6,7,8,9,10,11,2,2,14,15,16,17,18,19,2,2,2,23,24,25,26,2,28,2,30,31,32};   
int rank,p,i;   
int x = 2; //elemento que desejamos saber suas posicoes   
int n = 32; //numero total de elementos no vetor   
int n_p; //numero de elementos por proc   
//ponteiros de inteiros que serao alocados   
int *vpos,*res,*subvetor;   
  
/**inicializacao**/  
Init(&argc, &argv);   
Size(&p);   
Rank(&rank);   
  
  
/**processamento**/  
//o proc 0 (zero) faz a conta para saber   
//quantos elementos cada parte ter·   
if(rank==ROOT_RANK){   
//guardando em n_p o tamanho dos subvetores   
n_p=n/p;   
  
//corrigindo caso tenha sido arredondado para baixo   
if(n_p*p < n) n_p++;   
  
//alocando o vetor de resultado (conter· as posiÁıes onde se encontra X)   
res = (int *) calloc(n,sizeof(int));   
}   
  
//manda o tamanho de cada sub-vetor para todos os procs atravÈs do proc 0   
Bcast(&n_p,1,INT,0);   
  
//todos os procs alocam seus subvetores   
subvetor = (int *) calloc(n_p,sizeof(int));   
  
//o proc 0 divide o vetor em subvetores de tamanho n/p   
Scatter(vetor,n_p,INT,subvetor,n_p,INT,0);   
 
//aloca o vetor que guardar· as respostas de cada subconjunto   
vpos = (int *) calloc(n_p,sizeof(int));   
//indÌce atual do vetor de respostas   
int indice=0;   
  
//passa por todas as posiÁıes do subvetor procurando por X   
for(i=0;i<n_p;++i){   
if( x == subvetor[i] ){ //achou X   
//guarda posicaÁ„o de X no vetor de respostas   
//para ter a posiÁ„o original desse elemento no vetor basta fazer   
//i È a posiÁ„o no subvetor + (o n˙mero do proc * o tamanho)   
vpos[indice++] = i+(rank*n_p);   
}   
}   
  
//junta todos os subvetores de resposta no vetor principal de respostas   
Gather(vpos,indice,INT,res,indice,INT,0);   
  
/**saÌda de dados**/  
//o proc 0 imprime a resposta final   
if(rank==ROOT_RANK){   
//mostra todas as posiÁıes que tem X   
for(i=0;i<n;++i)   
printf("\nRes[%d]=%d",i,res[i]);   
}   
  
/**finalizaÁ„o**/  
Finalize();   
return (0);   
  
}