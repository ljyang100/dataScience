#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
using namespace std;
class Node{
public:
	int value;
	Node* left;
	Node* right;
	Node(int x) :value(x), left(NULL), right(NULL){}
};

//**** maxDepth. 
class MaxDepth{
public:
	
	int maxDepth(Node *root) //Imagin why I need a return type of Node*. 
	{
		////The following is for clear understanding.
		//if (root == NULL ) return 0; //**** Note people take the first level as 0 or 1 by convention. If 0, then here return -1. 
		//int l = maxDepth(root->left);
		//return l+1;


        //****The whole recursive procedure, like the binary tree traversal, can also be intuitively visualized by writing 123 (the steps below) in each node.
        //****Then after finishing one step, put an underscore under the specific number. 
		if (root == NULL) return 0; //**** Note people take the first level as 0 or 1 by convention. If 0, then here return -1. 
		int l = maxDepth(root->left);
		int r = maxDepth(root->right);
		return max(l, r) + 1;
		//****Normally the returned value is not useful such as in cases of binary tree inverting, tree traversal, etc.
        //**** However, here the return the value is useful. Be familiar with these ideas. 

	}
};

int main() {
	Node *root, *leftChild, *rightChild;
	root = new Node(5); leftChild = new Node(7); rightChild = new Node(3); root->left = leftChild; root->right = rightChild;
	Node *left = new Node(11), *right = new Node(12);
	leftChild->left = left; leftChild->right = right;

	MaxDepth maxDepth;
	int mDepth; 
	mDepth = maxDepth.maxDepth(root);

	delete root;
	delete leftChild;
	delete rightChild;
}