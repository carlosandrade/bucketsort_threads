import java.util.ArrayList;
import java.util.Random;

public class BucketSort{
    
    
    public static void main(String args[])
    {int qtdElementosVetor = 100;
     int qtdThreads = 10;
     int qtdElementosPorBucket = qtdElementosVetor/qtdThreads;
     Random generator = new Random();
     
        //Operacao para carregar numeros em um vetor e obter quantidade de elementos do vetor
            //int[] vetor = new int[qtdElementosVetor];
        //int[] vetor = new int[] {1,3,4,6,4,2,9,1,2,9};
        
        int[] vetor = new int[qtdElementosVetor];
        for(int i=0;i<qtdElementosVetor;i++)
            vetor[i] = generator.nextInt(5001+5000)-5000; //Gera um valor no intervalo de 0 a 10000 e subtrai -5000
        
        
        //Cria o vetor de buckets
        Bucket[] buckets = new Bucket[qtdThreads];  
        
        //Inicializa o vetor de buckets
        for(int i=0;i<qtdThreads;i++)
            buckets[i] = new Bucket(qtdElementosPorBucket);
            
        //Criterio de separacao dos elementos por bucket de modo que os elementos em um bucket seja heterogeneo
        //e possuam a mesma quantidade
        separaElementosDoVetorNosBuckets(vetor,buckets);
        
        //Sort dos elementos de cada bucket por insertion sort
        for(int i=0;i<qtdThreads;i++)
            insertionSort(buckets[i].bucket);
        /*    
        //Concatenacao dos buckets em um vetor
        for(int i=0;i<qtdElementosVetor;i = i + buckets[i].bucket.length)
        {
            for(int j=0; j<buckets[i].bucket.length;j++)
                vetor[i] = buckets[i].bucket[j];
        }
        */
        
        //Imprime os buckets em ordem crescente
        for(int i=0; i < buckets.length;i++)
            System.out.println("Bucket "+i+": "+buckets[i]);
        
        /*
        //Impressao dos valores depois do sort
        for(int i=0; i<qtdElementosVetor;i++)
            System.out.print(vetor[i] + " ");
        System.out.println();
        */
    }
    private static void separaElementosDoVetorNosBuckets(int[] vetor, Bucket[]buckets)
    {
        for(int i = 0; i<vetor.length;i++)
            buckets[decidePosicaoDoBucket(vetor[i],buckets)].add(vetor[i]);
    }
    /**
        Este metodo dinamicamente atribui a cada bucket uma faixa de intervalo baseado
        na quantidade de elementos de cada bucket. 
        
        Ex: se vetor.size = 20 elementos, buckets.size = 4 buckets, as faixas de intervalo
        sao de bucket(1) = 1 a 5, bucket(2) = de 6 a 10, bucket(3) de 11 a 15 e bucket(4) 16 a 20.
    
        O criterio de alocao para numeros negativos e feito observando o modulo do valor. 
            Uma vez que a faixa de valores permitidas e simetrica (-5000 a +5000) a distribuicao
            dos elementos por bucket em teoria nao deve ser afetada.
            
        Se o elemento atual for 0 ele coloca imediatamente no primeiro bucket.
            
        Lembrar que o bucket sort CONCATENA os buckets ao final, logo os elementos de cada bucket
        devem necessariamente obedecer a uma faixa de intervalos. A heterogeneidade dos elementos
        de cada bucket deve obedecer, portanto, a esse intervalo.
    */
    private static int decidePosicaoDoBucket(int elementoAtual, Bucket[]buckets)
    {int i=0;
        if(elementoAtual == 0)
            return buckets.length/2;
        
            
        int elementoAtualEmModulo = elementoAtual;
        
        if(elementoAtualEmModulo<0)
            elementoAtualEmModulo = elementoAtualEmModulo*-1;
            
        for(;i<(buckets.length/2);i++)
        {
            if(elementoAtualEmModulo < ((5000/((float)buckets.length/2))*(i+1)))
            {
                if(elementoAtual < 0)
                    return((buckets.length/2) - (i+1));
                else
                    return((buckets.length/2) + i);
            }
        }
        
        //Se saiu do loop entao e um dos valores limites    
        
        //System.out.println("Ouch, valor e: "+elementoAtual);
        
        if(elementoAtual < 0)
            return (buckets.length/2 - i); 
        else
            return((buckets.length/2) + i);
            
       /*     
        if(elementoAtual < 0)
        {
            elementoAtual = elementoAtual*-1;
            for(int i=1;i<buckets.length;i++)
            {
                if(elementoAtual <= ((5000/buckets.length)*i))
                {
                    if(elementoAtual)
                }
                    return (i-1); //-1 pois o vetor comeca a partir de 0.
            }
            return (i-1);
        }
        
            
        for(int i =buckets.length/2; i<buckets.length;i++)
        {
            if(elementoAtual <= ((5000/buckets.length)*i))
                return (i-1); //-1 pois o vetor comeca a partir de 0. 
        }
        return (buckets.length - 1); 
        
    }
    */
    }
    public static void insertionSort(ArrayList<Integer> a) {
        for (int i=1; i < a.size(); i++) {
            /* Insert a[i] into the sorted sublist */
	        int v = a.get(i);
	        int j;
	        for (j = i - 1; j >= 0; j--) {
	            if (a.get(j) <= v) break;
	            a.set(j+1,a.get(j));
	        }
	        a.set(j+1,v);
        }
    }
}