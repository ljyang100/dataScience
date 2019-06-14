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
	Node *last; 
public:
	CircularList() :last(NULL){}
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
	Node * newNode = new Node;
	newNode->data = data;
	newNode->next = NULL;

	if (last == NULL)
	{
		last = newNode;
		last->next = last;
	}
	else
	{
		newNode->next = last->next; //Note it is not newNode->next = last. If I use two nodes structure.
		last->next = newNode;
	}
}

void CircularList::addNodeFromEnd(int data)
{
	Node * newNode = new Node;
	newNode->data = data;
	newNode->next = NULL;

	if (last == NULL)
	{
		last = newNode;
		last->next = last;
	}
	else
	{
		//Unlike linked list, we don't have traverse when adding in the end. 
		newNode->next = last->next;
		last->next = newNode;
		last = newNode; //**** don't forget this one.
	}
}

void CircularList::deleteNodeFromEnd()
{
	Node * preLast = new Node;
	Node * temp = last;

	preLast = last;
	while (preLast->next != last) preLast = preLast->next;
	preLast->next = last->next;
	last = preLast;

	delete temp;
	//Still need handle the boundary problem. For example, if there are five elements, and I delete 5 times.

}

void CircularList::deleteNodeWithValue(int data)
{
	Node * previous = new Node;
	Node * current = new Node;

	current = last;
	do
	{
		previous = current;
		current = current->next;
		if (current->data == data) break;
	} while (current != last);

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
	temp = last;
	if (temp != NULL)
	{
		//****when printing out either list or circular list, we use (temp != Last/NULL), but not use (temp->next !=last/NULL)
		//****In the circular list case, it is better to use do while structure. First temp = last need print out, and next temp=last does not print. So using do while is perfect match.
		do
		{ 
			std::cout << temp->data << ","; 
			temp = temp->next;
		} 
		while (temp != last); //**** after a do while structure, there is a ; in the end of while statement. This is very different from the while() {} case. 
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