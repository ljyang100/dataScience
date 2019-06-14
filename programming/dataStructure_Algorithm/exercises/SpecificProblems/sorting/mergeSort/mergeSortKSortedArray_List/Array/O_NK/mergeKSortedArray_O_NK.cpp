#include<iostream>
#include<queue> //For priority_queue
#include<vector>

class Compare {
public:
	bool operator()(std::vector<int>::iterator a, std::vector<int>::iterator b)
	{
		return (*a) > (*b); //default is normally < sign. So the > sign definition is necessary. 
	}
};


int findMinAndMove(std::vector<std::vector<int>::iterator> &it_vec) //****Here must I must pass by reference. In Java, it also did this? 
{
	int top = INT_MAX; //**** If no NULL default to return or to compare for a pointer/interator, set up a special default value for its contents. 
	int ret_i = -1;
	int min = INT_MAX; //Use MAX when finding minimum value. 

	for (int i = 0; i < it_vec.size(); ++i)
	{
		if (*(it_vec[i]) < min)
		{
			min = *(it_vec[i]);
			ret_i = i;
		}
	}

	if (ret_i != -1)
	{
		top = *(it_vec[ret_i]);
		it_vec[ret_i] = std::next(it_vec[ret_i]);
	}

	return top;
	//**** There is not "standard" default for iterator, but we can set a default value for the contents of the iterator. 
	//**** It can return INT_MAX, but it is fine. 
}

std::vector<int> mergeKSortedArrays_NK(std::vector<std::vector<int>> &vec)
{
	if (vec.size() == 0) return{ INT_MIN, INT_MAX };
	std::vector<int> resultVector;

	std::vector<std::vector<int>::iterator> it_vec;
	for (int i = 0; i < vec.size(); ++i)
	{
		it_vec.push_back(vec[i].begin());
	}

	int top = findMinAndMove(it_vec);
	while (top != INT_MAX) //As long as there are non INT_Max in it_vec, findMinAndMove will return a non INT_MAX value. 
	{
		resultVector.push_back(top);
		top = findMinAndMove(it_vec);
	}
	return resultVector;
}

// Driver program to test above
int main()
{
	//Even with STL containers, we can always use pointer-like iterator. 
	std::vector<int> vec1 = { 1, 3, 5, INT_MAX }, vec2 = { 2, 6, 8, INT_MAX }, vec3 = { 0, 9, 10, INT_MAX }, vec4 = { 4, 7, INT_MAX };
	//**** because I cannot if (top.next != NULL), or even cannot use vec.end(). I put some indicators in the end of array.

	std::vector<std::vector<int>> vec;
	vec.push_back(vec1); vec.push_back(vec2); vec.push_back(vec3); vec.push_back(vec4);


	//merge the k sorted vectors.
	std::vector<int> result = mergeKSortedArrays_NK(vec);

	return 0;
}
