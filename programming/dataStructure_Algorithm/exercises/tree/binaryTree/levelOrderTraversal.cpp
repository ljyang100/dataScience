#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <queue>


class Node
{
public:
	int data;
	Node *left;
	Node *right;

	Node(int x) : data(x), left(NULL), right(NULL) {}
};

class LevelOrder
{
public:

	// Iterative method to do level order traversal line by line
	void printLevelOrder(Node *root)
	{
		// Check the link https://www.youtube.com/watch?v=NjdOhYKjFrU for the general idea. The key point is enqueue and dequeue. However the following implementation is a little different in the case of print level by level.
		// Here while(nodeCount>0) is used, while in the link, it use two continuous NULLs as the termination condition. The implementation below seems better and intuitive. Just adding a sentence std::cout << std::endl, then it will print level by level. 
		//**** The diagram in the video for enqueue and dequeue (not deque) is very infomrmative. 

		// Base Case
		if (root == NULL)  return;

		// Create an empty queue for level order tarversal
		std::queue<Node *> q; //**** A queue of pointer.

		// Enqueue Root and initialize height
		q.push(root); //q us a queue of pointers. But q itself is not a pointer. So q. but not q->

		while (1) // always true and break at nodeCount = 0;
		{
			// nodeCount (queue size) indicates number of nodes
			// at current lelvel.
			int nodeCount = q.size(); //**** As going to deep levels, the nodeCount might become bigger. 

			//**** Important breaking condition. First, nodeCount or q.size will be 1. Then q.size() will become bigger and bigger. Howver,when reaching the bottom of tree, no left or right node pushed into the queue.
			//**** However, the dequeue process is still going on. Eventually it the q will become empty and q.size()= nodeCount = 0. Thus finally will satisfy the breaking condition. 
			if (nodeCount == 0)  
				break;

			// Dequeue all nodes of current level and Enqueue all
			// nodes of next level
			while (nodeCount > 0)
			{
				Node *node = q.front(); //****Important: set a node pointer dynamically pointing to the front of the queue. Then later, always refers to this pointer but not the root. 
				std::cout << node->data << " ";
				q.pop();
				if (node->left != NULL)
					q.push(node->left);
				if (node->right != NULL)
					q.push(node->right);
				nodeCount--;
			}
			std::cout << std::endl; //
		}
	}
};


// Driver program to test above functions
int main()
{
	// Let us create binary tree shown above
	Node *root = new Node(1);
	root->left = new Node(2);
	root->right = new Node(3);
	root->left->left = new Node(4);
	root->left->right = new Node(5);
	root->right->right = new Node(6);
	root->right->right->left = new Node(7);
	root->right->right->right= new Node(8);

	LevelOrder levelOrder;
	levelOrder.printLevelOrder(root);


	return 0;
}
