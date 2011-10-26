import java.io.*;
import java.util.Random;

public class RandomCreator{
    
    public RandomCreator()
    {
  
    }
    public static void main(String args[])
    {
        //Gera vetor de numeros aleatorios
         int valor;
         int qtdElementosVetor = 100000;
         Random generator = new Random();
         
         
         try{
           // Create file 
           FileWriter fstream = new FileWriter("randomInput3.txt");
           BufferedWriter out = new BufferedWriter(fstream);
           
           out.write(Integer.toString(qtdElementosVetor));
           out.write("\n");
           for(int i=0;i<qtdElementosVetor;i++)
           {
               valor = (generator.nextInt(5001+5000)-5000); //Gera um valor no intervalo de 0 a 10000 e subtrai -5000
               out.write(Integer.toString(valor));
               out.write("\n");
           }
             
           //Close the output stream
           out.close();
          }catch (Exception e){//Catch exception if any
          System.err.println("Error: " + e.getMessage());
          }
        }
}