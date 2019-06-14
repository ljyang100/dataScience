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
	bool isSymmetric(TreeNode* root)
	{
		return isMirror(root, root);
	
	}

	bool isMirror(TreeNode *root1, TreeNode *root2)
	{
		//The Judging whether two trees are same is ALMOST idential to the problem of juding whether a single tree is symmetric. 
		//The code structure is almost same, except with a trick as above. 
		//**** Here the recursive approach is O(n), same as the iterative method. However, the recursive method seems easier to code.
		//**** At least my iterative method is not that easy to code. Compare my way of iterative and the iterative solution from leetCode.
		if (root1 == NULL && root2 == NULL) return true; 
		if (root1 == NULL || root2 == NULL) return false; 
		
		return (root1->val == root2->val) && isMirror(root1->left, root2->right) && isMirror(root1->right, root2->left);
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
	isTrue = solution.isSymmetric(root);

	delete root;
	return 0;
}

