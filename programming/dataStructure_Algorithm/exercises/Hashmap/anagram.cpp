#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
using namespace std;

int main() {
	
	std::string s = "anagram", t = "anagrm";
	bool isAnagram = false;
	if (s.size() != t.size()) isAnagram = false;
	else
	{
		//s.size() == t.size()
		if (s.size() == 0) isAnagram = true;
		else
		{
			////**** Part I: O(n^2) algorithm
			//std::vector<bool> matched(s.size(), false);
			//for (int i = 0; i < s.size(); ++i)
			//{
			//	for (int j = 0; j < t.size(); ++j)
			//	{
			//		if (s[i] == t[j] && matched[i] == false)
			//		{
			//			matched[i] = true;
			//			break; //This break is very critical. Otherwise, if there are same letters, it will set all of them to be true.
			//		}
			//	}
			//}

			//bool temp = true;
			//for (int i = 0; i < s.size(); ++i)
			//	if (matched[i] == false)
			//	{
			//		temp = false;
			//		break;
			//	}
			//isAnagram = temp;

			////**** Part II O(NlogN) algorithm.
			//std::sort(s.begin(), s.end());
			//std::sort(t.begin(), t.end());
			//isAnagram = (s == t);

			////**** Part III O(N) algorithm. Hashmap.
			//std::unordered_map<char, int> hMap;
			//for (int i = 0; i < s.size(); ++i)
			//	hMap[s[i]]++;
			//for (int i = 0; i < t.size(); ++i)
			//	hMap[t[i]]--;

			//bool temp = true;
			//for (auto elem : hMap)
			//	if (elem.second != 0)
			//	{
			//		temp = false;
			//		break;
			//	}
			//isAnagram = temp;

			//**** Part IV O(N) algorithm. Using vector to simulate Hashmap. In this case, we don't use Hash function, then 
			//**** This method is sort of like my using orderId to simulate hash keys. 
			std::vector<int> vMap(128,0); //**** If for only letters, it can be with smaller size. 
			for (int i = 0; i < s.size(); ++i)
				vMap[s[i]]++;
			for (int i = 0; i < t.size(); ++i)
				vMap[t[i]]--;

			bool temp = true;
			for (int i = 0; i < 128; ++i) // 0=<ASCII<128. Also note how to pronounce ASCII.
				if (vMap[i] != 0)
				{
					temp = false;
					break;
				}
			isAnagram = temp;
		}
	}

	return 0;
}