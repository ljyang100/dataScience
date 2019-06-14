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

//**** SameTree. 
class SameTree{
public:
	
	bool sameTree(Node *root1, Node *root2) //Imagin why I need a return type of Node*. 
	{
		if (root1 == NULL && root2 == NULL) return true;

		//***** Be careful! The following looks like wrong because root1= NULL || root2 = NULL may include the case of both root1 and root2 are NULL. 
        //***** However, consider the first sentence.  If the thread arrived here, then it already indicates that both root1 and root2 are NULL. Otherwise, 
        //***** it already returned. 
        //***** So the seemingly wrong code, actually save one line of code if we write in more complete way.  
        //***** Whenever return is used many times, think about that we may save some logical structure. 
		if (root1 == NULL || root2 == NULL) return false;
		return (root1->value == root2->value) && sameTree(root1->left, root2->left) && sameTree(root1->right, root2->right);
		//****For tree problems, always think about recursive for standard implementation. However, it might be better use dynamic programming.

        //**** Also study why the algorithm above is O(n). Because each node is visited only once. 
        //**** Finally note that all the final node has left and right as NULL. So eventually the  first if sentence above will be used. 
	}
};

int main() {

    //There is a better way to construct a simple binary tree. See another file for invert_mirror tree. 
	Node *root1, *root2, *leftChild, *rightChild;
	root1 = new Node(5); leftChild = new Node(7); rightChild = new Node(3); root1->left = leftChild; root1->right = rightChild;
	root2 = new Node(5); leftChild = new Node(5); rightChild = new Node(3); root2->left = leftChild; root2->right = rightChild;

	SameTree sameTree;
	bool isTrue = false;
	isTrue = sameTree.sameTree(root1, root2);

	delete root1,root2;
	delete leftChild;
	delete rightChild;
}