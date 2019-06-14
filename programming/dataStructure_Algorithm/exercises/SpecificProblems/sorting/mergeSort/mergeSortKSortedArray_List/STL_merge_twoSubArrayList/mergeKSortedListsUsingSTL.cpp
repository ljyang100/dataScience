#include <vector>
#include <iostream>
#include <algorithm>
#include <list>

//Input:
//[
//	1->4->5,
//	1->3->4,
//	2->6
//]
//Output : 1->1->2->3->4->4->5->6



int main()
{
	std::list<int> list1, list2, list3;
	list1.push_back(1); list1.push_back(4); list1.push_back(5);
	list2.push_back(1); list2.push_back(3); list2.push_back(4);
	list3.push_back(6);

	std::vector<std::list<int>> listVector; 
	listVector.push_back(list1); listVector.push_back(list2); listVector.push_back(list3);

	for (int i = 1; i < listVector.size(); ++i) //**** The index starts from 1, and the final result is stored in the first list (the left one). There is no return value for merge() here. 
	{
		listVector[0].merge(listVector[i]);
	}
	//If I want to use the .merge() for linked list, I need use the std::list to store my lists, where the merge() method is defined. 

	return 0;
}