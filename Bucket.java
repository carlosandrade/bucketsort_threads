import java.util.ArrayList;


public class Bucket{
    public ArrayList<Integer> bucket;
   // public int posicaoAtualNoBucket = 0;
    
    public Bucket()
    {
        bucket = new ArrayList<Integer>();
    }
    public void add(int elemento)
    {
        bucket.add(elemento);
        
        /*
        if(posicaoAtualNoBucket < bucket.length)
        {
            bucket[posicaoAtualNoBucket] = elemento;
            posicaoAtualNoBucket++;
        }
        else
            System.out.println("Oops..algo deu errado, nao cabe nesse bucket mais nenhum elemento");
            
        */
    }
    public String toString(){
        String string = "";
        for(int i=0;i<bucket.size();i++)
            string = string+" "+bucket.get(i);
        return string;
    }
}