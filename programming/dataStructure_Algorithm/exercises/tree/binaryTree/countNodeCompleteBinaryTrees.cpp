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

	//Count COMPLETE tree nodes; What is complete tree

	//****Method 1: A very natural way is to traverse with level-order and tree and print out the non-null results.
	//****The following method exceed the time limit in LeetCode. But for general binary tree, the following approach is efficient, right?
	int countNodes_I(TreeNode* root) 
	{
		if (root == NULL) return 0; //Why is this necessary.

		std::queue<TreeNode*> queue;

		queue.push(root);

		int nodeCount = 0;
		while (!queue.empty()) //Unlike some version where two while loops are used, here only one while-loop is necessary if using !queue.empty().
		{
			TreeNode *q = queue.front();
			++nodeCount;
			queue.pop();
			if (q->left != NULL) queue.push(q->left);
			if (q->right != NULL) queue.push(q->right);
		}

		return nodeCount;

	}

	//****Mehtod 2: A recursive solution by myself. Write 1234 on each node to intuitively understand the recursive procedure. 
	//****Still exceed the time limit. 
	int countNodes_II(TreeNode* root)
	{
		if (root == NULL) return 0; //Why is this necessary.
		return 1 + countNodes_II(root->left) + countNodes_II(root->right);
	}

	//****Method 3: from web. 45.2%.  However, if I modify the return pow(2,hLeft)-1 to return (1<< (hLeft)) - 1; Then it will become 80%. 
	//****So counting the complete tree, two tricks are very important. One is use the formaula for a perfect tree 2^N. The other trick is use shifting operator instead of pow(2,N).
	//****Furthermore, in method4, there might be some more optimization in calculating the height of a tree. 
	int countNodes_III(TreeNode* root) {
		int hLeft = 0, hRight = 0;
		TreeNode *pLeft = root, *pRight = root; 
		//**** Even there is only one root, in binary tree related problem, we often assign root to two pointers. Another example is for determining a symmetric tree. 

		//*****
		while (pLeft) {
			++hLeft;
			pLeft = pLeft->left;
		}
		while (pRight) {
			++hRight;
			pRight = pRight->right;
		}
		
		if (hLeft == hRight) return pow(2, hLeft) - 1; 
		//**** I think this is only good for complete binary tree. Otherwise, even if h_Left = h_Right, we cannot have this conclution. For example, a "mid-hollow" tree cannot use 2^k-1. 
		//**** For complete tree, this is the critical part for saving time. In a big tree, three might be a lot of perfect tree from the perspective of many nodes. Under these conditions, the number of nodes can be calculated
		//**** with a single expression 2^k -1. It is this point that save a lot of time where a tree complete, or even there are a lot of complete part. 
		//**** However, if a tree is not complete at all, or even sparse, then the algorithm should be similar to a O(N) algorithm from pure recursion. 

		return countNodes_III(root->left) + countNodes_III(root->right) + 1; 
	}

	//***** Method 4. 95.6%. This is much better and it seems to be general for any binary trees?
	//***** Figure out why this is much faster the earlier one, and much faster than my two versions.
	enum Side { LEFT, RIGHT };
	int getHeight(TreeNode* root, Side side){
		int h = 0;
		if (side == LEFT){
			while (root->left){
				h++;
				root = root->left;
			}
		}
		else {
			while (root->right){
				h++;
				root = root->right;
			}
		}

		return h;
	}
	int countNodes(TreeNode* root, int L = -1, int R = -1) {
		if (!root) return 0;
		L = L == -1 ? getHeight(root, LEFT) : L;
		R = R == -1 ? getHeight(root, RIGHT) : R;
		if (L == R){
			// this is a complete binary tree
			return (1 << (L + 1)) - 1;
		}
		else {
			return 1 + countNodes(root->left, L - 1, -1) + countNodes(root->right, -1, R - 1);
		}
	}
};

int main() {

	int x = -1;

	x = x >> 3;
	x = (2 << 5) - 1;
	x = pow(2,6) - 1;

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
	int nodeCount;
	//isTrue = solution.isSymmetric_Mine(root);
	nodeCount = solution.countNodes_III(root);

	delete root;
	return 0;
}
