#include <stdio.h> 
int binarySearch(int arr[], int l, int r, int x)
{
	//**** Some similarity to quicksort: [1] It is a recursive partitioning. [2] The closing statement is an inequality, though different.
	if (r >= l)
	{
		int mid = l + (r - l) / 2;

		// If the element is present at the middle  itself 
		if (arr[mid] == x)
			return mid;

		// If element is smaller than mid, then  it can only be present in left subarray 
		if (arr[mid] > x)
			return binarySearch(arr, l, mid - 1, x);
		else
			return binarySearch(arr, mid + 1, r, x);
	}

	// We reach here when element is not present in array 
	return -1;
}

int main(void)
{
	int arr[] = { 2, 3, 4, 10, 40 };
	int n = sizeof(arr) / sizeof(arr[0]);
	int x = 10;
	int result = binarySearch(arr, 0, n - 1, x);
	(result == -1) ? printf("Element is not present in array")
		: printf("Element is present at index %d",
		result);
	return 0;
}

