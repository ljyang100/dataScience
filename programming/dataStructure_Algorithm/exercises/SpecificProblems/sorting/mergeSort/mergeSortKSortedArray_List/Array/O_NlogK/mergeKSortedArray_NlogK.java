//import java.util.*;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Iterator;
import java.util.PriorityQueue;

public class MergeKSortedArrays {

   public static void main(String args[]) {
      // Create an array list
      ArrayList<Integer> vec1 = new ArrayList<Integer>(), vec2 = new ArrayList<Integer>(), 
      vec3 = new ArrayList<Integer>(), resultVector = new ArrayList<Integer>();
      vec1.add(1); vec1.add(3); vec1.add(5); 
      vec2.add(2); vec2.add(4); vec2.add(6);
      vec3.add(9); vec3.add(10); 

      ArrayList<ArrayList<Integer>> vec = new ArrayList<ArrayList<Integer>>();
      resultVector = mergeKSortedArray(vec);

   }

    public static ArrayList<Integer> mergeKSortedArray(ArrayList<ArrayList<Integer>> vec){

        PriorityQueue<Iterator> heap = new PriorityQueue(vec.length, new MyComparator());
        for (int i = 0; i < vec.length; ++i)
        {
            heap.add(vec.get(i).iterator()); //vec.get(i) in Java <=> vec[i] in C++
        }

        ArrayList<Integer> resultVector = new ArrayList<Integer>();
       
        while (!heap.isEmpty()) {
            Iterator popIt = heap.poll();
            if(popIt.hasNext()) heap.push(popIt.next());
          
            resultVector.add(Integer(top));

            
           //last.next = popNode;
           //last = popNode;
        }
        return resultVector;

    }
      


   static class MyComparator implements Comparator<Integer>{
    public int compare (Integer arg0, Integer arg1){
         return arg0 - arg1;
        }
    }

}