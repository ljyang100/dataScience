#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include <functional> //for std::greater<int>. 
//No <multiset> and <multimap> headers.

class MyComparison{
private:
	bool m_minHeap;  
public:
	MyComparison(const bool &minHeap = false)
	{
		m_minHeap = minHeap;
	}
	bool operator()(const std::pair<int, int> &lhs, const std::pair<int, int> &rhs) const
	{
		if (m_minHeap) return (lhs.first + lhs.second <= rhs.first + rhs.second); //**** The sign here is not necessarily right.
		else return (lhs.first + lhs.second >= rhs.first + rhs.second);
	}
};

class NaiveSolution{
public:

	//NlogN solution
	//Note the return type is vector.
	std::vector<std::pair<int, int>> kSmallestPairs(std::vector<int> &num1, std::vector<int> &num2, int k)
	{
		int m = num1.size();
		int n = num2.size();
		if (m*n == 0)
			return std::vector<std::pair<int,int>>();
			//**** This is to return an empty vector;
		
		std::vector<std::pair<int, int>> allPairs;
		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				allPairs.push_back(std::make_pair(num1[i], num2[j]));
			}
		}

		std::sort(allPairs.begin(), allPairs.end(), MyComparison()); //****I cannot just use MyComparison without (). This is different from that of the priority_queue case. 
		//****The final parameter can be class name or function name + ().

		return std::vector<std::pair<int, int>>(allPairs.begin(), allPairs.begin() + std::min(k, (int)allPairs.size()));
		//**** If don't know where is the min or max and do not want self-define, use the std::min/max.

	}

};

//Nlogk complexity. 
class HeapSolution{
public:
	std::vector<std::pair<int, int>> kSmallestPairs(std::vector<int> &nums1, std::vector<int> &nums2, int k)
	{
		//MyComparison myComparison(false);
		//std::priority_queue < std::pair<int, int>, std::vector<std::pair<int, int>>, MyComparison> pq; //**** Here I cannot add () after MyComparison. 
		std::priority_queue < std::pair<int, int>> pq;
		//**** Never add a () after MyComparison.
		int m = nums1.size();
		int n = nums2.size();
		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				if (pq.size() < k)
					pq.push(std::make_pair(nums1[i], nums2[j]));
				else
				{
					auto top_pair = pq.top();
					if (top_pair.first + top_pair.second < nums1[i] + nums2[j])
					{
						pq.pop();
						pq.push(std::make_pair(nums1[i], nums2[j]));
					}
				}

			}
		}

		if (pq.size() == 0) return std::vector<std::pair<int, int>>(); //() indicating an empty vector.

		//The following code show how to return a vector from other data structure.
		std::vector<std::pair<int, int>> returnedVector(pq.size());
		int i = pq.size() - 1;
		while (!pq.empty())
		{
			returnedVector[i] = pq.top();
			pq.pop();
			--i;
		}
		return returnedVector;
	}
};

//Nlogk complexity. 
class SetSolution{
public:
	std::vector<std::pair<int, int>> kSmallestPairs(std::vector<int> &nums1, std::vector<int> &nums2, int k)
	{
		//MyComparison myComparison(false);
		//std::priority_queue < std::pair<int, int>, std::vector<std::pair<int, int>>, MyComparison> pq; //**** Here I cannot add () after MyComparison. 
		std::multiset <  std::pair< int, std::pair<int,int> > > pair;
		//**** Never add a () after MyComparison.
		int m = nums1.size();
		int n = nums2.size();
		for (int i = 0; i < m; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				pair.insert(  std::make_pair(  nums1[i] + nums2[j], std::make_pair(nums1[i], nums2[j]) ) );
			}
		}
		
		std::vector<std::pair<int, int>> returnedVector;
		auto it = pair.begin();
		int k_index = 0;
		while (k_index < k)
		{
			returnedVector.push_back(it->second);
			++it;
			++k_index;
		}

		return returnedVector;
	}
};

int main()
{
	std::vector<int> nums1 = { 1, 3, 5, 2 };
	std::vector<int> nums2 = { 1, 4, 9 };
	int k = 5;
	//NaiveSolution naiveSolution;
	std::vector<std::pair<int, int>> result;
	//result = naiveSolution.kSmallestPairs(nums1, nums2, k);

	////Heap Solution
	////If I will find a smallest subset, then I need maximum heap, which is the default case if I implement heap by using priority_queue.
	//HeapSolution heapSoluiton;
	//result = heapSoluiton.kSmallestPairs(nums1, nums2, k);


	////Heap examples:
	////For the heap, the top() is zero element. So if it is like pq = (1, 3, 7, 10..), then it is a min heap. 
	////Note the default heap generate by priority queue is max heap. 
	//std::priority_queue<int> PQ_max;
	//for (int i = 0; i < 10; ++i)
	//	PQ_max.push(i);
	//int a = PQ_max.top();

	//std::priority_queue<int,std::vector<int>, std::greater<int>> PQ_min;
	//for (int i = 0; i < 10; ++i)
	//	PQ_min.push(i);
	//int b = PQ_min.top();

	//Set solution
	SetSolution setSolution;
	result = setSolution.kSmallestPairs(nums1, nums2, k);
	

	return 0;

}