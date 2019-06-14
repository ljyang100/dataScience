#include<iostream>
#include<vector>

//Part I: Pure recursive: time-complexity O(2^n)
// Simple and suitable for small n. 
int FibNaive(int n)
{
	if (n < 3) return 1;
	else
		return FibNaive(n - 1) + FibNaive(n - 2);
}

//Part II: Top-down dynamic programming: recursive + memoization.
//time-complexity O(n). Big improvement from O(2^n) to O(n). 
int FibTopDownDp(int n, std::vector<int> &vec)
{
	//**** If there is calculated result use it. If not, calculate and store it.
	if (vec[n] != -1) return vec[n];
	else
	{
		int result;
		if (n < 3) result = 1;
		else
			result = FibTopDownDp(n - 1, vec) + FibTopDownDp(n - 2, vec);
		
		vec[n] = result;
		return result;
	}
}

//Part III: Bottom-up dynamic programming.
//time-complexity O(n). 
int FibBottomUpDp(int n, std::vector<int> &vec)
{
	//Also need use the formerly calcuated results. 
	//However, we start from filling lower n, rather than using recursive way from tree top as in the top-down method.
	vec[1] = 1;
	vec[2] = 1;
	for (int i = 3; i < (n + 1); ++i)
		vec[i] = vec[i - 1] + vec[i - 2];
	return vec[n];
}

int main()
{
	int result;

	////Part I 
	//result = FibNaive(5);

	////Part II
	//int n = 7;
	//std::vector<int> vec(n+1, -1); //allocate n+1 elements for convenience
	//result = FibTopDownDp(n, vec);

	//Part III
	int n = 7;
	std::vector<int> vec(n+1, -1); //allocate n+1 elements for convenience
	result = FibBottomUpDp(n, vec);


	return 0;
}
