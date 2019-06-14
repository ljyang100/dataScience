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

               ListNode[] lists = new ListNode[] {list1, list2,list3};
        MyFirstJava merge = new MyFirstJava();
      
              ListNode result2 = merge.mergeKlists_O_NK(lists);
        while(result2.next != null){
            System.out.println(result2.next.val);
            result2 = result2.next;
        }
     		
    }

     private ListNode findMinAndMove(ListNode[] lists){
        
        //**** Similar to the priority_queue case: Find the min/max value and move pointer. 
        //**** In that case, we automatically pop up the minimum, and then move pointer if next node is not null. 
        //**** Here the complexity to find min is O(k) while in priority queue case it is O(logK)
        int min_value = Integer.MAX_VALUE;
        int ret_i = -1;
        for(int i = 0; i < lists.length; i++){ 
            //lists.size() in C++ vs. lists.length in Java
            if(lists[i] == null) {
                //NULL in C++ vs null in Java.
                continue;
            }

            if(lists[i].val < min_value){
                min_value = lists[i].val; //**** This value is useful. Check the structure. 
                ret_i = i;
            }
        }
        //Using the above to find minimum within k numbers, we have O(k) complexity. If we use heap, then we have O(logk) complexity.

        ListNode ret_node = null;
        if(ret_i != -1) {
            ret_node = lists[ret_i];
            lists[ret_i] = lists[ret_i].next;
        }
        return ret_node;
    }

    public ListNode mergeKlists_O_NK(ListNode[] lists){
        if(lists == null || lists.length == 0){
            return null;
        }

        ListNode head = new ListNode(0); //See other places for why I use this dummy node.
        ListNode last = head;
        ListNode temp = null;
        //From above, it is same as the method of using priority_queue. 

        // //temp is for checking whether all list nodes are traversed.
        // //For first time I found that the do-while loop is useful. However, I replace it with while loop for convenience. 
        // do {
        //     temp = findMinAndMove(lists);

        //     //Next two lines are same as priority_queue case. The difference is that there temp here is automatically
        //     //obtained by poping the priority_queue. However, here we need a function to find this minimum. 
        //     last.next = temp;
        //     last = temp;
        // } while (temp!=null);

        temp = findMinAndMove(lists);
        while (temp != null) {
            //Next two lines are same as priority_queue case. The difference is that there temp here is automatically
            //obtained by poping the priority_queue. However, here we need a function to find this minimum. 
            last.next = temp;
            last = temp;
            temp = findMinAndMove(lists);
        } 

        return head.next; //If using dummy node, then return head.next instead of head. 
    }

   
}
