#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <queue>



struct TreeNode {
	int val;
	TreeNode *left;
	TreeNode *right;
	TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution {
public:

	//****Idea is same. Whenevern iterative approach in trees is involved, it seems that we need the help of queue. The fundamental scheme is with the help of queue, and the general process is enqueue and dequeue.
	//****Recall some different examples using iterative queue-helped approaches: level-order traversal, symmetric tree judgement, BFS, etc., and compare their differences. 
	bool isSymmetric(TreeNode *root)
	{
		//**** Unlike my comments later in my iterative version, the iterative here is not that messy. Just my version is messy. I need learn standard way of solving this type of problem.
		//**** Also the while(!queue.empty()) is better than the while(1) + if (nodeCount == 0) break;
		std::queue<TreeNode *> queue;

		queue.push(root);
		queue.push(root);

		//****Note if using the following while loop, then only single while loop is enough. In the next example, two-folded while loop is used and it is not good. 
		while (!queue.empty()) //This is better than the while(1) + if (count==0) break;
		{
			TreeNode *q1 = queue.front(); queue.pop();
			TreeNode *q2 = queue.front(); queue.pop();

			if (q1 == NULL && q2 == NULL) continue;  //The iterative method here is similar to the recursive way. 
			if (q1 == NULL || q2 == NULL) return false; //Be familiar with such structure. Once arrived here, it already indicates that q1 = NULL and q2=NULL do not hold simultaneously.

			if (q1->val != q2->val) return false; //No need to handle the return true case. Why?

			//No need to check q1 or q2 is NULL. Once arrived here, it indicates they are not NULL.
			queue.push(q1->left);	queue.push(q2->right);
			queue.push(q1->right);	queue.push(q2->left);
			
		}

		return true;
		//Once arrived here, the tree must be symmetric. 
		//****Be familar with the structure of "once arrived here". See above. three places involve this. 

	
	}



	//****Iterative approach of mine. 
	bool isSymmetric_Mine(TreeNode* root)
	{

		//Method 1: A naive way is first invert the tree, and then compare whether it is the same tree as original. I have don't the invert and sametree methods before.

		//Method 2: Use level-order traversal:
		//See original version in another file for the general idea of enqueue and dequeue process in interative way of printing in level-order.

		if (root == NULL) return true;

		std::queue<TreeNode *> q;
		q.push(root); //First enqueue. q.size() must be 1 as root != NULL, otherwise it already return.
		int nodeCount = q.size();

		std::vector<int> tempVector; //Introduced for storing the data in each level. 
		while (1) //Pay attention to a breaking condtion below.
		{
			//The following two lines cannot be exchanged.
			nodeCount = q.size();
			if (nodeCount == 0) break;

			while (nodeCount > 0)
			{
				TreeNode *node = q.front();
				//****node pointer dynamically pointing to the front of the queue. Later always I should always refer to this pointer but not the root.
				//**** In the following enqueue code, I was once use "if (root->left != NULL) q.push(root->left)...which will give an endless loop.


				if (node != NULL)
				{
					//**** Be careful! Whenever I use node->left or node->right, I need always check whether node is NULL. If it is NULL, then node->left is not accessible.
					//**** This is particularly important when some leaves are empty. 
					std::cout << q.front()->val << " ";
					tempVector.push_back(q.front()->val);
					q.pop();
					//Enqueue process.
					if (node->left != NULL)	q.push(node->left); else q.push(NULL);
					if (node->right != NULL) q.push(node->right); else q.push(NULL);
				}
				else //q.front() == NULL. In this case, node = q.front(), and node->left is not defined. 
				{
					std::cout << "NULL" << " ";
					tempVector.push_back(1234); //**** I can store a huge number for this. 
					q.pop();
				}


				--nodeCount;

				//The following code is added by me for checking symmetric tree.
				if (nodeCount == 0)//Indicating one level is done, and check whether this level is symmetric. 
				{
					if (tempVector.size() == 1)
					{
						//std::cout << "So far it is symmetric! " << std::endl;
						tempVector.clear();
					}
					else
					{
						if (tempVector.size() % 2 != 0)
						{
							//****This branch is actually not necessary because even for no-data branch I will store a special number in it. So it will not have odd number whenever the size is > 1.
							std::cout << "The tree is not symmetric because it has odd number of nodes after the first level. " << std::endl;
							return false;
						}
						else
						{
							//**** My way of using two pointers
							int i = 0;
							int j = tempVector.size() - 1;

							while (i < j)
							{
								if (tempVector[i] == tempVector[j])
								{
									++i; //**** One is ++ and the other is --
									--j;
								}
								else
								{
									return false; //Can I do this? Yes. This can save a lot of time. 
								}

							}
							tempVector.clear(); //Do not forget this line.
						}
					}
				}
			}
			std::cout << std::endl; // This line is the original code. 
		}
		return true;
	}
};

int main() {

	//There is a better way to construct a simple binary tree. See another file for invert_mirror tree. 
	TreeNode *root;
	root = new TreeNode(1);
	root->left = new TreeNode(2);
	root->right = new TreeNode(2);
	root->left->left = new TreeNode(7); //****If this line is commented, then the root->left->left will be NULL, the default value. 
	//root->left->right = new TreeNode(3);
	//root->right->left = new TreeNode(3);
	root->right->right = new TreeNode(17);


	Solution solution;
	bool isTrue = false;
	//isTrue = solution.isSymmetric_Mine(root);
	isTrue = solution.isSymmetric(root);

	delete root;
	return 0;
}
