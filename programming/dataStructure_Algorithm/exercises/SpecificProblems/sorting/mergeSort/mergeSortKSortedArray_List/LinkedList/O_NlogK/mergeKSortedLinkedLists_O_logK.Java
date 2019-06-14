
import java.util.PriorityQueue;
import java.util.ArrayList;
import java.util.List;
import java.util.Comparator;
public class MyFirstJava {
    //The following class can be put into outside can declared as "class ListNode...." (cannot be "public ListNode..."). 
    //However, we put it here as a static nested class is for convenience. So nested static class behaves like top-level outer class.
    //See the relevant nodes. 

    private static class ListNode {
        int val;
        ListNode next;
        ListNode (int x) {val = x;} 
        //Similar to C/C++ except don't have to initialize next.
    }

    public static void main(String[] args)
    {
      
        ListNode list1 = new ListNode(1);
        list1.next = new ListNode(3);
        list1.next.next = new ListNode(5);
        list1.next.next.next = new ListNode(7);

        ListNode list2 = new ListNode(2);
        list2.next = new ListNode(4);
        list2.next.next = new ListNode(6);
        list2.next.next.next = new ListNode(8);

        ListNode list3 = new ListNode(0);
        list3.next = new ListNode(9);
        list3.next.next = new ListNode(10);

        MyFirstJava merge = new MyFirstJava();  //Access with reference. 
      
        ListNode result1 = merge.mergeKlists_PriorityQueue(lists);
        while(result1.next != null){
            System.out.println(result1.next.val);
            result1 = result1.next;
        }
    }

    static class MyComparator implements Comparator<ListNode>{
        public int compare (ListNode arg0, ListNode arg1){
             return arg0.val - arg1.val;
         }
    }
 
    //https://www.youtube.com/watch?v=Uz4fTr34270&t=224s. If not familar the picture, check the video.
    //Find the smallest node in all of linked list's head/pointer
    public ListNode mergeKlists_PriorityQueue (ListNode[] lists){
        //  //The following is almost one-to-one mappting to C++. See visualization scheme in c++ code for understanding. 
        // if(lists == null || lists.length == 0){
        //      return null;
        //  }
              
        //  ListNode head = null; 
        //  ListNode last = null;
        //  PriorityQueue<ListNode> heap = new PriorityQueue(lists.length, new MyComparator());
        //  //C++: priority_queue<ListNode*, vector<ListNode*>, compNode> pq;
 
        //  //Insert the head of all lists
        //  for (ListNode listHead:lists){
        //      if(listHead != null){
        //          heap.add(listHead);
        //      }
        //  }
 
        // while(!heap.isEmpty()){ //C++ while(!heap.empty())
        //      ListNode popNode = heap.poll();
        //      if(popNode.next != null){
        //         heap.add(popNode.next); //C++ heap->push(popNode->next); Note push_back.
        //     }
        //      //c++ topp() vs java poll(). Very different (see C++ code later). /Note poll() first retrieve the node value, and then pop out the node.
             
        //      if(head == null)
        //      {
        //          head = popNode;
        //          last = popNode;
        //      }
        //      else
        //      {
        //         last.next = popNode;
        //         last = popNode;
        //      }
            
        //  }

        // return head;

        //A Better waye
        ListNode head = new ListNode(0); //**** Is this the so-called dummy node? Here head.next but not head is the real head of linked list. 
        ListNode last = head;
        //****Unlike the first method where head and last both point to null, here we define a res or dummy node. Both res and last point to this dummy node.
        //****This way, we don't have check whether head = null every loop in the while statement.
        PriorityQueue<ListNode> heap = new PriorityQueue(lists.length, new MyComparator());
       
        //Insert the head of all lists
        for (ListNode listHead:lists){
            if(listHead != null){
                heap.add(listHead); //C++ heap->push(popNode->next); Note push_back.
            }
        }

       while(!heap.isEmpty()){ //C++ while(!heap.empty())
            ListNode popNode = heap.poll();
            if(popNode.next != null){
               heap.add(popNode.next); 
           }
 
           last.next = popNode;
           last = popNode;
           
        }

       return head.next;
       //****If we use dummy node head, then we should return head.next, but not head.  
       //****Also be familiar with how the head and last are declared. It is different from that both points to null. 
    }
}
