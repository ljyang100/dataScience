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

//**** Inverting a tree. 
class InvertTree{
public:
	
	Node *invertTreeNode(Node *root) //Imagin why I need a return type of Node*. 
	{
		if (root == NULL) return NULL;

        //**** See a better example in another file.
        //**** Exchange the order for the following part I and part II is better to understand. 
        //In the recursive tree, plot 123 to understand fast tract the recursive procedure. See another example in aother file. 

        //Part I
		Node *temp; 
		temp = root->left;
		root->left = root->right;
		root->right = temp;
		
        //Part II
        invertTreeNode(root->left); //**** each recursive branch will push nestly to the stack until return
		invertTreeNode(root->right);//**** Imagine the process of recursive process. 
		return root; //**** This return root seems useless. I changed it to return NULL, it still works. Right?
	}
};

int main() {
	Node* root = new Node(5);
	Node* leftChild = new Node(10);
	Node* rightChild = new Node(3);
	root->left = leftChild;
	root->right = rightChild;

	cout << "root:left:right are " << root->value << ":" << root->left->value << ":" << root->right->value << endl;

	InvertTree invert;
	invert.invertTreeNode(root);
	cout << "root:left:right are " << root->value << ":" << root->left->value << ":" << root->right->value << endl;

	delete root;
	delete leftChild;
	delete rightChild;
}