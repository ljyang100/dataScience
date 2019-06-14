#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional> //for std::greater<int>. 

using namespace std;
//No <multiset> and <multimap> headers.

class SubArraySum{

public:
	int subArraySumBrute(std::vector<int> &nums, int &k)
	{
		int ans = 0;
		//****If I want to calculate the all possible sum from index i to j of an array, then the following code is not right. sum(j)-sum(i) = nums(i+1) +nums(i+2) + ...+ nums(j). So if i = 0, then will miss the sum from nums(0) to 
		//****all other num(j). 
		//std::vector<int> sums(nums.size(),0);
		//int tempSum = 0;
		//for (int i = 0; i < nums.size(); ++i)
		//{
		//	tempSum = tempSum + nums[i];
		//	sums[i] = tempSum;
		//}

		//**** To include all possible sums from nums[0], we allocate an extra 0 in the sums array.
		std::vector<int> sums(nums.size() + 1, 0);
		for (int i = 1; i <= nums.size(); ++i) //**** i <= but not i < 
			sums[i] = sums[i - 1] + nums[i - 1];


		for (int i = 0; i < (sums.size() - 1); ++i) // sums.size = nums.size + 1
		{
			for (int j = i + 1; j < sums.size(); ++j)
			{

				if (sums[j] - sums[i] == k)
				{
					std::cout << i << ":" << j - 1 << std::endl; //**** Note i and j are the indices in the original nums array.
					ans += 1;
				}
			}
		}

		return ans;
	}

	
	int subArraySumHash(std::vector<int> &nums, int &k)
	{
		//{ 10, 2, -2, -20, 10, -2, -6, -2, -7, -3 }
		if (nums.empty()) return 0;
		std::unordered_map<int, int> umap;
		umap.insert({ 0, 1 });
		int currentSum = 0;
		int ans = 0;
		for (int i = 0; i< nums.size(); ++i)
		{
			currentSum = currentSum + nums[i];
			umap[currentSum] = umap[currentSum] + 1;
			//**** Note currentSum can increase and then decrease. So we may have multiple same currentSum value obtain with sum of different number of array indices. 

			ans = ans + umap[currentSum - k];
			//Most important points: 
			//[1] I need insert {0,1} to the map because when currentSum = k, then it is obviously a solution.
			//[2] Check the twosum Hash problem. The method here is actually exactly the same idea to the two-sum case with duplicate numbers there. 


			//If I use the following code as from the two-sum or three-sum case, then it will miss a lot.
			//As already mentiond above, the following code is only suitable for distinct case. If there are duplicate cases, then we should not just use ++ans or ans=ans+1, but should use ans = ans+umap[currentSum-k].
			//if (umap.find(currentSum - k) != umap.end()) ++ans;
		}
		return ans;
	}

};

// Driver program to test above function
int main()
{
	int arr[] = { 10, 2, -2, -20, 10 };
	std::vector<int> nums = { 10, 2, -2, -20, 10, -2, -6, -2, -7, -3 }; //0123, 1234, 34, 567, 89, 456789, 01234567;

	int n = sizeof(arr) / sizeof(arr[0]);
	int sum = -10;

	SubArraySum subArraySum;
	subArraySum.subArraySumBrute(nums, sum);
	subArraySum.subArraySumHash(nums, sum);

	//std::unordered_map<int, int> hello;
	//int a = hello[1];
	//hello[1];
	//The above two statements will all set hello[1] = 0;

	return 0;
}