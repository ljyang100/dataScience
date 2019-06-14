#include<iostream>
#include<vector>
#include<algorithm> 
#include<unordered_map>
#include<map>
#include<functional>

int main()
{
	std::vector<int> vec = { 1, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 7, 1, 0, 3, -1, 0, 1, 1 };
	std::unordered_map<int, int> umap; 

	for (int i = 0; i < vec.size(); ++i) //O(N)
	{
		if (umap.find(vec[i]) != umap.end())
		{
			++umap[vec[i]];
		}
		else
		{
			umap[vec[i]] = 1;
		}
	}

	std::vector<std::pair<int, int>> pairVector;
	for (auto it = umap.begin(); it != umap.end(); ++it)  //O(K), where K is the number of distinct values in vec. 
	{
		pairVector.push_back(std::make_pair(it->second, it->first)); //std::sort use the first pair element to sort by default. So here use the frequency as the first pair element as required.
	}

	std::sort(pairVector.rbegin(), pairVector.rend()); //KlogK, where K is the number of distinct values in vec.

	return 0;
}
