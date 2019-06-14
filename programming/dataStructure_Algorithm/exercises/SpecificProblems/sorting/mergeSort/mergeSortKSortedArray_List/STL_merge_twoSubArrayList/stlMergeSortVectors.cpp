#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

// comparator function to reverse merge sort
struct greaters {
	bool operator()(const long& a, const long& b) const
	{
		return a > b;
	}
};

int main()
{
	// initializing 1st container
	vector<int> arr1 = { 1, 4, 6, 3, 2 };

	// initializing 2nd container
	vector<int> arr2 = { 6, 2, 5, 7, 1 };

	vector<int> arr3 = { 1, 6, 4, 5, 3, 7 };

	// declaring resultant container
	vector<int> arr4;

	// sorting initial containers
	// in descending order
	std::sort(arr1.rbegin(), arr1.rend());
	std::sort(arr2.rbegin(), arr2.rend());

	// using merge() to merge the initial containers
	// returns descended merged container
	std::merge(arr1.begin(), arr1.end(), arr2.begin(), arr2.end(), arr4.begin(), greaters()); // without the greaters(), then the default output is ascending order.

	// printing the resultant merged container
	cout << "The container after reverse merging initial containers is : ";

	for (int i = 0; i < arr4.size(); i++)
		cout << arr4[i] << " ";

	return 0;
}
