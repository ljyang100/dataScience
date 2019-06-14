#include<iostream>
#include<vector>
#include<algorithm> 
#include<unordered_set>
#include<functional>


class LongestSubsequence{
public:

	//I: Naive solution--O(NlogN)
	// If an array is already ordered, then the following approach can provide an O(n) complexity. 
	// In other words, for ordered array, there is no need to use Hash-based method to achieve O(n) complexity approach. 
	int findLongestSubseqence_Naive(std::vector<int> &vec)
	{
		std::sort(vec.begin(), vec.end()); //O(NlogN)
		int maxLength = 0;
		for (int i = 0; i < vec.size(); ++i) //O(N).
		{
			int currentLength = 1;
			int j = i + 1;
			while (j < vec.size() && vec[j] - vec[j - 1] == 1)
			{
				++currentLength;
				++j;
			}

			//Important line. Otherwise O(n) becomes O(n^2)
			//This way, the seeminly nested loop actually loop alternatively only for N times. So it is O(N).
			i = j; 

			maxLength = std::max(maxLength, currentLength);
		}

		return maxLength;
	}

	//From GeeksforGeeks
	int findLongestConseqSubseq(int arr[], int n)
	{
		std::unordered_set<int> S;
		int ans = 0;

		// Hash all the array elements
		for (int i = 0; i < n; i++)
			S.insert(arr[i]);
		//**** The algorithm will not give longest sequence like {1,2,2,3,3,3,4,5,6} because here we have to use unordered_set. If we use set, then the above for loop will have already O(NlogN) complexity.
		//**** I tried use unordered_multiset and still I cannot obtain the above results. 
		//**** So the O(N) complexity for longest consecutive subsequence is only for distinct values, right? 

		// check each possible sequence from the start
		// then update optimal length
		for (int i = 0; i < n; i++)
		{
			// if current element is the starting
			// element of a sequence
			if (S.find(arr[i] - 1) == S.end())
			//**** Best condition I have ever met for a particular subsequence problem. If the whole array is strictly consecutively sequenced, then only the smallest number satisfy this condition.
			//**** If the not two numbers in the array is not consecutive, then this if condition for every elment is met, but the nested while loop will only run twice for each element. So it is still O(N) solution. 
			//**** Unlike the i-reset in other example of finding longest consecutive ones, the condition here is very different can crutial. 
			{
				// Then check for next elements in the
				// sequence
				int j = arr[i];
				while (S.find(j) != S.end())
					++j; //***Same final result if we use j++

				// update  optimal length if this length
				// is more
				ans = std::max(ans, j - arr[i]);
			}
		}
		return ans;
		//At first look, time complexity looks more than O(n).If we take a closer look, we can notice that it is O(n) under the assumption that hash insert and search take O(1) time.The function S.
		//find() inside the while loop is called at most twice for every element.For example, consider the case when all array elements are consecutive.In this case, the outer find is called for every element,
		//but we go inside the if condition only for the smallest element.Once we are inside the if condition, we call find() one more time for every other element.
	}
};

// Driver program
int main()
{
	int arr[] =  { 1, 9, 3, 10, 4, 20, 2, 6, 5,5,5,5 };
	int n = sizeof arr / sizeof arr[0];
	LongestSubsequence longestSubsequence;
	std::cout << "Length of the Longest contiguous subsequence is "
		<< longestSubsequence.findLongestConseqSubseq(arr, n) << std::endl;

	std::vector<int> vec = { 1, 9, 3, 10, 4, 20, 2, 6, 5 };
	int result;
	result = longestSubsequence.findLongestSubseqence_Naive(vec);
	

	return 0;
}
