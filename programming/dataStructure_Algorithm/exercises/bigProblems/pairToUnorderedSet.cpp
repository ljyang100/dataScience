#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>


int main()
{
	//**** Note ****
	//**** Important. Although in standard we cannot insert pair to unordered_set, we can do so for std::multiset. See example in find K pairs of smallest sum. 
	std::unordered_set<std::pair<int, int>> hello;
	//There is no standard way to insert pair to unordered_set.
	//Check "insert pair into unordered_set" for the ways of providing specialize the template to do so. 
	//**** Seems not necessary, because this is what exactly does by un_ordered_map.
	return 0;
}
