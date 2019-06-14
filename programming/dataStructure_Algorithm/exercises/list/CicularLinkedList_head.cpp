#include <stdio.h> //**** Note these are two most common headers.
#include <stdlib.h>
#include <iostream> //**** for std::cout, etc. 

struct Node
{
	int data;
	Node *next;
};

class CircularList
{
private:
	Node *head; 
public:

	//*****
	//***** When coding list stuff, be careful that I need at aleast three nodes to figure out general formula, unless I don't have three nodes yet. 
	CircularList() :head(NULL){}
	void addNodeFromEnd(int data);
	void deleteNodeFromEnd();
	void deleteNodeWithValue(int data);
	void deleteNthNode(int position);
	void addNodeFromStart(int data);
	void displayList();

	~CircularList(){}
};

void CircularList::addNodeFromStart(int data)
{
	//See the AddNodeFromEnd. Even I add from Start, I still need traverse to the last node in order let the last node pointing to the head node.
}

void CircularList::addNodeFromEnd(int data)
{
	Node * newNode = new Node;
	newNode->data = data;
	newNode->next = NULL;

	Node * last = new Node;
	if (head == NULL)
	{
		head = newNode;
		head->next = head;
		last = head;
	}
	else
	{
		//When figuring out the formula, use at least three nodes unless I cannot. Otherwise it will be easy errors.
		//See other file using last as the parameter for circular linked list, where I don't need traverse to add node, either from start or end.
		last = head;
		while (last->next != head) last = last->next;
		last->next = newNode;
		newNode->next = head;
	}
}

void CircularList::deleteNodeFromEnd()
{
	Node * preLast = new Node;
	Node * temp = head;

	preLast = head;
	while (preLast->next != head) preLast = preLast->next;
	preLast->next = head->next;
	head = preLast;

	delete temp;
	//May need handle the boundary problem. For example, if there are five elements, and I delete 5 times.

}

void CircularList::deleteNodeWithValue(int data)
{
	Node * previous = new Node;
	Node * current = new Node;

	current = head;
	do
	{
		previous = current;
		current = current->next;
		if (current->data == data) break;
	} while (current != head);

	previous->next = current->next;
	delete current;
}
void CircularList::deleteNthNode(int position)
{
	
	//delete;
}

void CircularList::displayList()
{
	Node * temp = new Node;
	temp = head;
	if (temp != NULL)
	{
		//****when printing out either list or circular list, we use (temp != head/NULL), but not use (temp->next !=head/NULL)
		//****In the circular list case, it is better to use do while structure. First temp = head need print out, and next temp=head does not print. So using do while is perfect match.
		do
		{ 
			std::cout << temp->data << ","; 
			temp = temp->next;
		} 
		while (temp != head); //**** after a do while structure, there is a ; in the end of while statement. This is very different from the while() {} case. 
	}
}

int main(void)
{
	CircularList *cirList = new CircularList;
	cirList->addNodeFromEnd(1);
	cirList->addNodeFromEnd(2);
	cirList->addNodeFromEnd(3);
	cirList->addNodeFromEnd(4);
	cirList->addNodeFromEnd(5);
	cirList->displayList(); std::cout << std::endl;
	cirList->deleteNodeFromEnd();
	cirList->displayList(); std::cout << std::endl;
	cirList->deleteNodeFromEnd();
	cirList->displayList(); std::cout << std::endl;
	cirList->deleteNodeFromEnd();
	cirList->displayList(); std::cout << std::endl;
	cirList->deleteNodeFromEnd();
	cirList->displayList(); std::cout << std::endl;
	cirList->addNodeFromStart(2);
	cirList->addNodeFromStart(3);
	cirList->addNodeFromStart(4);
	cirList->addNodeFromStart(5);
	cirList->displayList(); std::cout << std::endl;
	cirList->deleteNodeWithValue(3);
	cirList->displayList(); std::cout << std::endl;
}