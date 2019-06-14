#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
using namespace std;

int main() {
	
	//Find duplicate values that separated with an upperbound distance. 
	//That is. a[i] = a[j], and |i-j| <= K, e.g. 2
	int K = 2;
	std::vector<int> vec = { 1, 2, 3, 4, 1, 6, 7, 1, 5, 1 };
	bool pairFound = false;
	int pairIndex_i, pairIndex_j;
	////First version of brute force algorithm
	//for (int i = 0; i < vec.size(); ++i)
	//{
	//	int j = i - 1;
	//	while (j >= 0)
	//	{
	//		if (vec[i] == vec[j])
	//			if (abs(i - j) <= K) //This is not good. Where is the problem? See the second version of brute force algorithm. 
	//			{
	//				pairFound = true;
	//				pairIndex_i = i;
	//				pairIndex_j = j;
	//				break;
	//			}
	//		j = j - 1;
	//	}
	//	if (pairFound) break;
	//}

	////Second version of brute-force algorithm. 
	//for (int i = 0; i < vec.size(); ++i)
	//{
	//	int j = i - 1;
	//	while (j >= 0)
	//	{
	//		if (abs(i - j) > K) break; 
	//		//**** This sentence is important! if this j is already satisfying abs(i-j)>K, then next j will also satisfy, so break to save computing. 
	//		//**** For such with fixed > or < relation, we can save some complexity. 
	//		if (vec[i] == vec[j])
	//		{
	//			pairFound = true;
	//			pairIndex_i = i;
	//			pairIndex_j = j;
	//			break;
	//		}
	//		j = j - 1;
	//	}
	//	if (pairFound) break;
	//}

	//Hashing-map way of finding duplicates with certain condition
	std::unordered_map<int, int> hmap;

	for (int i = 0; i < vec.size(); ++i)
	{
		if (hmap.find(vec[i]) != hmap.end()) 
		{
			//The value for key vec[i] is found in the map.
			if (i - hmap[vec[i]] > K) hmap[vec[i]] = i;
			else
			{
				pairFound = true;
				pairIndex_i = i;
				pairIndex_j = hmap[vec[i]];
				break;
			}

		}
		else
		{
			hmap[vec[i]] = i;
		}
	}

	return 0;
}