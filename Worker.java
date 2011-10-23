import java.lang.Thread;

public class Worker extends Thread{
    Bucket bucket;
    public Worker(Bucket bucket)
    {
        this.bucket = bucket;
    }
    public void run()
    {
        ordenaBucket();
    }
    private void ordenaBucket()
    {
     insertionSort(bucket);   
    }
    private void insertionSort(ArrayList<Integer> a)
    {
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
