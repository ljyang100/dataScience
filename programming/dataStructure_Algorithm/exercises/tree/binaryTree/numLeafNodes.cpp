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
	int numLeafNodes(TreeNode* root) {

		if (root==NULL) return 0;
		int count;
		if (root->left == NULL && root->right == NULL) count = 1; else count = 0;
		return count + numLeafNodes(root->left) + numLeafNodes(root->right);

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
	nodeCount = solution.numLeafNodes(root);

	delete root;
	return 0;
}
