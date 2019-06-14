#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
//No <multiset> and <multimap> headers.
class FindKthLargest{
public:

	//Method I
	int findKthLargestNaive(std::vector<int> &nums, int k)
	{
		//NlogN performance, mainly from sorting
		std::sort(nums.begin(), nums.end());
		return nums[nums.size() - k];
	}

	//Method II
	int findKthLargestHeap(std::vector<int> &nums, int k)
	{
		//More general, as we can use it with other data types such as pairs in other example. 
		std::priority_queue<int, std::vector<int>, std::greater<int> > pq;
		//**** Note std::greater<int> need the header <functional>.
		//**** std::greater is for min heap. 

		for (int i = 0; i < nums.size(); ++i)
		{
			if (pq.size() < k)
			{
				pq.push(nums[i]);
			}
			else
			{
				int min_elem = pq.top(); //That is the reason we call min heap.
				if (min_elem < nums[i])
				{
					pq.pop();
					pq.push(nums[i]);
				}
			}
		}

		return pq.top();
	}

	//Method III 
	int findKthLargestSet(std::vector<int>& nums, int k)
	{
		//LogN from mset.insert(), then we have NlongN. However, I found this solution
		//is much faster than method I or II. It seems that the mset.insert is not logN operation.
		std::multiset<int> mset;
		for (int i = 0; i < nums.size(); ++i)
			mset.insert(nums[i]);

		auto it = mset.end();
		std::advance(it, -k); //*** Note advance() does not return anything. 
		return (*it);

	}
};
int main()
{

	//Find the kth largest number in the array
	std::vector<int> nums = { 1, 3, 5, 5, 2, 2, 7, 9, 11, 12 };
	int k = 2;
	FindKthLargest findKthLargest;
	int a = findKthLargest.findKthLargestNaive(nums, 2);
	int b = findKthLargest.findKthLargestHeap(nums, 2);
	int c = findKthLargest.findKthLargestSet(nums, 2);

	return 0;

}