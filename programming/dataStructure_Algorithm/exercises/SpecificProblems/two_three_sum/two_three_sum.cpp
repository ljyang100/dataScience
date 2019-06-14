#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class TwoSum{
	//Find the indices i, j of two array elements satisfying array[i] + array[j] = fixed number k.

public:

	//Method 1: brute-force O(n^2) approach. Can be implemented with nested loop.
	//**** Note implemented.


	//Method 2: From the C++ interview course. 
	//The method has a O(N) complexity iff the array is ordered. Otherwise, ordering it will take O(NlogN) time.
	//Also this method can not easily generalized to multiple pairs that satisfy the condition. Also, it must have one soluiton. Otherwise, I need further modify the code. So it is not a good way. 
	//However, it is easy to track the original index of the elements with pair structure. 
	std::pair<int, int> twoSum(std::vector<int> &nums, int &k)
	{
		std::vector<std::pair<int, int>> numsPair; //This is to check which array index an element originally has.
		for (int I = 0; I < nums.size(); ++I)
		{
			numsPair.push_back(std::make_pair(nums[I], I)); //Consider that when sorting the nums, it will use the first pair element as the default. 
		}
		std::sort(numsPair.begin(), numsPair.end());

		int i = 0, j = numsPair.size() - 1;

		while (i < j)
		{
			if (numsPair[i].first + numsPair[j].first == k)
				return std::make_pair(numsPair[i].second, numsPair[j].second);
			else
			{
				if (numsPair[i].first + numsPair[j].first < k)
				{
					++i;
				}
				else if (numsPair[i].first + numsPair[j].first > k)
				{
					--j;
				}
			}
		}
	}

	//method 3: Two-pass hash table.
	//**** In fact, this method is rediculously not optimal. For such a problem, I should never try two passes. This will cause extra problem. So NEVER DO TWO passes for Hash table. Otherwise it will
	//**** lead to duplicate answers. See the optimal one later. 
	std::unordered_map<int, int> twoSumTwoPassHash(std::vector<int> &nums, int &k)
	{
		//One pass Hash table
		std::unordered_map<int, int> umap, umap1, umap2;
		for (int i = 0; i < nums.size(); ++i)
		{
			umap.insert(std::make_pair(nums[i], i));
			//***The value i in the key-value has not been used. So the method in this function can actually be done by an unordered_set.
			//***However, if we want to keep track of the index, or we are explicitly required to calcuate the indices that satisfy the conditions, then using std::ordered_map is a better choice. See the optimal hash table later.
		}

		//Two pass Hash table
		for (int i = 0; i < nums.size(); ++i)
		{
			if (umap.find(k - nums[i]) != umap.end() && (k - nums[i]) != nums[i]) //Cannot use the same element twice. 
			{
				umap1.insert(std::make_pair(nums[i], k - nums[i])); //Here saved to an unordered map for the purpose of eliminating similar pairs such as (3,7) and (7,3). 
				//tempVector.push_back(std::make_pair(nums[i], k - nums[i]));
			}
		}

		//Three pass hash table: for eliminating the equivalent pair of (2, 3) and (3, 2) etc. 
		for (auto it = umap1.begin(); it != umap1.end(); ++it)
		{
			if (umap2.empty())
				umap2.insert(std::make_pair(it->first, it->second));
			else
			{
				if (umap2.find(it->second) != umap2.end()) //not Found
					continue;
				else 
					umap2.insert(std::make_pair(it->first, it->second));
			}
		}

		return umap2;
		//**** The index can actually obtained directly by modifying umap1.insert(std::make_pair(nums[i], k - nums[i])); to umap1.insert(std::make_pair(i, umap[k - nums[i]])); See the next example. 
	}

	//method 4: Optimal hash table.
	void twoSumOnePassHash(std::vector<int> &nums, std::unordered_map<int, int> &umapElem, std::unordered_map<int,int> &umapIndex, int &k)
	{
		//One pass Hash table
		std::unordered_map<int, int> umap, umap1, umap2;
		for (int i = 0; i < nums.size(); ++i)
		{
			//**** Note the following single pass save not just a pass, but directly obtain the distinct resulting pairs.
			umap.insert(std::make_pair(nums[i], i));
			if (umap.find(k - nums[i]) != umap.end() && (k - nums[i]) != nums[i]) //Cannot use the same element twice. 
			{
				umapElem.insert(std::make_pair(nums[i], k - nums[i])); //elements 
				umapIndex.insert(std::make_pair(i, umap[k - nums[i]])); //element indices. Note the way of retrieving index. 
			}
		}

	}

};

struct Triplet{
	int i;
	int j;
	int k;
};

class ThreeSum{
public:
	void threeSum(std::vector<int> &nums, std::vector<Triplet> &tripletElem, std::vector<Triplet> &tripletIndex, int &k)
	{
		std::unordered_map<int, int> umap, umap1, umap2;
		Triplet triplet;
		for (int i = 0; i < nums.size(); ++i)
		{
			umap.insert(std::make_pair(nums[i], i)); //**** If I initialize umap separately in an ealier loop, then the following code will find equivalent triplets such as (1,4,9), (1,9,4), etc. 
			for (int j = i+1; j < nums.size(); ++j)
			{
				//**** Note the following single pass save not just a pass, but directly obtain the distinct resulting pairs.
				if (umap.find(k - nums[i] - nums[j]) != umap.end() && (k - nums[i] - nums[j]) != nums[i] && (k - nums[i] - nums[j]) != nums[j]) //Cannot use the same element twice. 
				{
					triplet.i = nums[i]; triplet.j = nums[j]; triplet.k = k - nums[i] - nums[j]; tripletElem.push_back(triplet);
					triplet.i = i; triplet.j = j; triplet.k = umap[k - nums[i] - nums[j]]; tripletIndex.push_back(triplet);
				}
			}
		}

	}
};

int main()
{
	std::vector<int> vec1 = { 4, 3, 1, 5, 9 }, vec2 = { 3, 1, 5, 9, 8, 7, 6, 4 };
	//**** How about if I have duplicate elements? Eliminate them first.
	//**** I tried the multi-map to handle the duplicate case. However, the [] is not defined. 


	int k = 10;

	TwoSum twoSum;
	std::pair<int, int> result1 = twoSum.twoSum(vec1, k);
	std::unordered_map<int, int> result2 = twoSum.twoSumTwoPassHash(vec2, k);
	std::unordered_map<int, int> umapElements, umapIndex;
	twoSum.twoSumOnePassHash(vec2, umapElements, umapIndex, k);


	std::vector<int> vec3 = { 3, 4, 8, 5, 9, 1, 13, 17, 15};
	int k3 = 15;
	ThreeSum threeSum;
	std::vector<Triplet> tripletElem, tripletIndex; 
	threeSum.threeSum(vec3, tripletElem, tripletIndex, k3);
	
	return 0;
}
