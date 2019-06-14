#include<iostream>
#include<queue> //For priority_queue

using namespace std;

class Node {
public:
	int data;
	Node *next;

	Node(int x) :data(x), next(NULL) {}
};

class Compare {
public:
	bool operator()(struct Node* a, struct Node* b)
	{
		return a->data > b->data; //default is normally < sign. So the > sign definition is necessary. 
		//**** Here > sign will give a min heap. I understand like this. 
		//**** When generating a sorted array and default > sign, then the initial point is with biggest value. 
		//**** For queue, we have FIFO and the initial point is the place where we add new data.
		//**** Thus the top is always minimum value if we have a > sign defined here. 
		//**** This also explained why the default < sign will give a maximum heap in the priority_queue later. 
	}
};

// function to merge k sorted linked lists
struct Node* mergeKSortedLists(std::vector<Node*> *lists)
{
	//**** Method I: with dummny node
	//**** Part I Initialize qp with first element of all k lists, and initialize head and last as NULL.  
	Node* head = new Node(0);
	Node* last = head; 
	//**** It cannot be last = new Node(0) as it will point to a different data = 0 node in memory. 
	//**** Unlike both head and last point to null as in method 2, here we use a dummy node and both head and last point to a real node. 
	priority_queue<Node*, vector<Node*>, Compare> pq;
	if (lists == NULL) return NULL;
	for (int i = 0; i < lists->size(); i++)
		pq.push((*lists)[i]);

	//**** Note this is different find the kth largest element where we need compare the current value with the top one.
	//**** We need to update the pq with the top k element. Here we need all the elements and thus don't have such comparison. Never get confused again.
	while (!pq.empty()) {
		//**** Part II Pop one for use and push in another. 
		struct Node* top = pq.top();
		pq.pop();
		if (top->next != NULL)
			pq.push(top->next);

		//****With the dummy node, we can only use the following two lines without checking whether head = null in each loop. Without dummy node as below, then we need more code. 
		last->next = top;
		last = top;
	}

	return head->next; 
	//****With dummy node, I need return head->next, rather than head.


	////**** Method II: without dummy node; 
	////**** Part IV. return head but not head->next. Not confuse with Java or Python. Why we need return ->next there.
	////**** Visualization for memorization: Three lists with q1, q2, q3 pointed; three-element pq; merged list pointed by head and last. When understanding the scheme, moving q1, q2, q3 in the lists.
	////**** In the pq, use q1 = 1, q2= 2, q3 = 9 etc, although it should be q3.data = 9...; draw multiple pq when q1, q2, q3 changes.....
	//return head;
	////**** Part I Initialize qp with first element of all k lists, and initialize head and last as NULL.  
	//Node* head = NULL, *last = NULL;
	//priority_queue<Node*, vector<Node*>, Compare> pq;
	//if (lists == NULL) return NULL;
	//for (int i = 0; i < lists->size(); i++)
	//	pq.push((*lists)[i]);

	////**** Note this is different find the kth largest element where we need compare the current value with the top one.
	////**** We need to update the pq with the top k element. Here we need all the elements and thus don't have such comparison. Never get confused again.
	//while (!pq.empty()) {
	//	//**** Part II Pop one for use and push in another. 
	//	struct Node* top = pq.top();
	//	pq.pop();
	//	if (top->next != NULL)
	//		pq.push(top->next);

	//	//**** Part III Set both head and last to first element, and then move last to eash additional element. 
	//	if (head == NULL) {
	//		head = top;
	//		last = top;
	//	}
	//	else {
	//		//**** Note we cannot change the order of the following two sentence. 
	//		last->next = top;
	//		last = top;
	//	}
	//}

	////**** Part IV. return head but not head->next. Not confuse with Java or Python. Why we need return ->next there.
	////**** Visualization for memorization: Three lists with q1, q2, q3 pointed; three-element pq; merged list pointed by head and last. When understanding the scheme, moving q1, q2, q3 in the lists.
	////**** In the pq, use q1 = 1, q2= 2, q3 = 9 etc, although it should be q3.data = 9...; draw multiple pq when q1, q2, q3 changes.....
	//return head;
}


// Driver program to test above
int main()
{
	std::vector<Node*> *lists = new std::vector<Node*>;
	Node *list1, *list2, *list3;

	// creating k = 3 sorted lists
	list1 = new Node(1);
	list1->next = new Node(3);
	list1->next->next = new Node(5);
	list1->next->next->next = new Node(7);

	list2 = new Node(2);
	list2->next = new Node(4);
	list2->next->next = new Node(6);
	list2->next->next->next = new Node(8);

	list3 = new Node(0);
	list3->next = new Node(9);
	list3->next->next = new Node(10);
	list3->next->next->next = new Node(11);

	lists->push_back(list1);
	lists->push_back(list2);
	lists->push_back(list3);

	// merge the k sorted lists
	struct Node* head = mergeKSortedLists(lists);


	return 0;
}
