// C program to convert a binary tree
// to its mirror
#include<stdio.h>
#include<stdlib.h>

/* A binary tree node has data, pointer
to left child and a pointer to right child */
struct Node
{
	int data;
	struct Node* left;
	struct Node* right;
};

/* Helper function that allocates a new node with the
given data and NULL left and right pointers. */
struct Node* newNode(int data)

{
	struct Node* node = (struct Node*)
		malloc(sizeof(struct Node));
	node->data = data;
	node->left = NULL;
	node->right = NULL;

	return(node);
}


/* Change a tree so that the roles of the  left and
right pointers are swapped at every node.
*/
void mirror(struct Node* node)
{
	if (node == NULL)
		return;
	else
	{
		/* do the subtrees */
		mirror(node->left);
		mirror(node->right);
        //Unlike another example, we don't need return a Node pointer to invert the true. 
        //Also note that the two parts of code can change order but still give the same results. 

        //**** If the swap code is after mirror(), then it is better to add a condition (if both ->left and ->right are NULL, then no need to swap. 
        //**** If the swap code is before mirror(), then not necessary?
		struct Node* temp;
		/* swap the pointers in this node */
		temp = node->left;
		node->left = node->right;
		node->right = temp;

        //Write 123 in each node to understand intuitively the above procedure. 1. mirror(node->left). 2. mirror(node->right). 3. swap. 



	}
}

/* Helper function to print Inorder traversal.*/
void inOrder(struct Node* node)
{
	if (node == NULL)
		return; //**** Note return nothing = return + ;

    //InOrder: left-root-right
    //**** Need connect the recursive picture with the binary tree in an intuitive way, and thus analyze them very quickly
	inOrder(node->left);
	printf("%d ", node->data);
	inOrder(node->right);

 //   //Pre_order: root-left-right
	//printf("%d ", node->data);
	//inOrder(node->left);
	//inOrder(node->right);

 //   //Post-order: left-right-post
	//inOrder(node->left);
	//inOrder(node->right);
	//printf("%d ", node->data);
}

//**** The following function is my version. I let it return a Node pointer.
//**** If finally as recursive to the end, node = NULL, then I return NULL. but return + ; 
//**** In my opinion, return NULL or return node both fine, anyway the returned value will not be used in his example. 
//**** Just as in the main(), return 0; The returned value 0 is actually not used. 
Node *InOrder(struct Node* node)
{
	if (node == NULL)
		return NULL; //**** Note return nothing = return + ;

	//InOrder: left-root-right
	inOrder(node->left);
	printf("%d ", node->data);
	inOrder(node->right);

	//   //Pre_order: root-left-right
	//printf("%d ", node->data);
	//inOrder(node->left);
	//inOrder(node->right);

	//   //Post-order: left-right-post
	//inOrder(node->left);
	//inOrder(node->right);
	//printf("%d ", node->data);
	return node;
}


/* Driver program to test mirror() */
int main()
{

    //****The following way to construct a binary tree is better than the other method in other file. 
    //****The key is root->left->left = nweNode(1); root->left->right = newNode(7)....
	struct Node *root = newNode(1);
	root->left = newNode(2);
	root->right = newNode(3);
	root->left->left = newNode(4);
	root->left->right = newNode(5);

	/* Print inorder traversal of the input tree */
	printf("Inorder traversal of the constructed"
		" tree is \n");
	inOrder(root); //InOrder(root); // also works

	/* Convert tree to its mirror */
	mirror(root);

	/* Print inorder traversal of the mirror tree */
	printf("\nInorder traversal of the mirror tree"
		" is \n");
	inOrder(root); //InOrder also works

	return 0;
}

//Inorder traversal of the constructed tree is
//4 2 5 1 3
//Inorder traversal of the mirror tree is
//3 1 5 2 4
