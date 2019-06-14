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
	//****Beat 100% CPP submission
	int sumOfLeftLeaves(TreeNode* root) {
		if (root == NULL) return 0;
		int currentSum = 0;
		if (root->left != NULL && root->left->left == NULL && root->left->right == NULL) currentSum = root->left->val; else currentSum = 0;

		return currentSum + sumOfLeftLeaves(root->left) + sumOfLeftLeaves(root->right);

	}
};


int main() {

	int x;
	x = 1 << 6;
	int y;
	y = pow(2, 5);

	//There is a better way to construct a simple binary tree. See another file for invert_mirror tree. 
	TreeNode *root;
	root = new TreeNode(1);
	root->left = new TreeNode(2);
	root->right = new TreeNode(2);
	root->left->left = new TreeNode(7); //****If this line is commented, then the root->left->left will be NULL, the default value. 
	//root->left->right = new TreeNode(3);
	//root->right->left = new TreeNode(3);
	root->right->left = new TreeNode(17);


	Solution solution;
	int nodeCount;
	//isTrue = solution.isSymmetric_Mine(root);
	nodeCount = solution.sumOfLeftLeaves(root);

	delete root;
	return 0;
}
