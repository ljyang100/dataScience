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

std::vector<int> mergeKSortedArrays(std::vector<std::vector<int>> &vec) //don't forget pass by reference.
{
	std::priority_queue<std::vector<int>::iterator, std::vector<std::vector<int>::iterator>, Compare> pq;
	for (int i = 0; i < vec.size(); i++)
		pq.push(vec[i].begin());

	std::vector<int> resultVector;
	while (!pq.empty()) {
		std::vector<int>::iterator top = pq.top();
		pq.pop();

		auto next = std::next(top); //Here std::next does not advance top iterator.  
		if ((*next) != INT_MIN) //
			pq.push(next);

		if ((*top) != INT_MIN) resultVector.push_back((*top));
	}
	return resultVector;
}

// Driver program to test above
int main()
{
	//Even with STL containers, we can always use pointer-like iterator. 
	std::vector<std::vector<int>> vec;
	std::vector<int> vec1 = { 1, 3, 5, INT_MIN }, vec2 = { 2, 6, 8, INT_MIN }, vec3 = { 0, 9, 10, INT_MIN }, vec4 = { 4, 7, INT_MIN };
	//**** because I cannot if (top.next != NULL), or even cannot use vec.end(). I put some indicators in the end of array.
	vec.push_back(vec1); vec.push_back(vec2); vec.push_back(vec3); vec.push_back(vec4);

	//merge the k sorted vectors.
	std::vector<int> result = mergeKSortedArrays(vec);

	return 0;
}
