//Like Merge Sort, QuickSort is a Divide and Conquer algorithm.
//Here the merge sort is not the way of merging k-sorted array/list, but refer to the way of merge with recursion for a single array.
//However, quicksort is in-place while merge sort need extra space (linked list does not, right?)
//Here I mainly should master the idea of divide and conquer in sorting, and then apply them in other places.
// The following code is from : https://www.geeksforgeeks.org/quick-sort/
// Take a look at a Youtube video for quick sort, and then study the code. There are many ways of doing partitioning. The code below corresponds to the following two links.
// https://www.youtube.com/watch?v=MZaf_9IZCrc https://www.youtube.com/watch?v=PgBzjlCcFvc&t=49s

/* C implementation QuickSort */
#include<stdio.h> 

// A utility function to swap two elements 
void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

//This function takes last element of each subarray as pivot
//**** For quicksort, it is mainly about partitioning, which is actually find the pivot and then return it. 
int partition(int arr[], int low, int high) 
{
	int pivot = arr[high]; // pivot is always the last element here.  
	int i = (low - 1);  // **** Index of smaller element, setting to -1 in the beginning. But later i will be other values for different 

	//**** Iterate from first to the last but 1 because we use the last as pivot. See the video before. 
	//**** Note high does not correspond to the size of array as usual, but the index of the final element. 
	for (int j = low; j < high; j++) 
	{
		if (arr[j] <= pivot) //****Similar to merge sort: if it is min (for ascending sort), using the value and then move pointer. 
		{
			i++;    
			swap(&arr[i], &arr[j]); //In the iteration swap i and j elements if condition met. 
		}
	}

	//**** Using pointer to swap is more efficient. 
	//*****Check https://www.youtube.com/watch?v=MZaf_9IZCrc for why we need swap this and to move the pivot in the right position
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}


void quickSort(int arr[], int low, int high)
{
	//Recursively do the three steps: [1] partioning (find the pivot, the first time pivot is last). [2] quicksort left. [3] quicksort right. Using this to memorize the idea. 
	//Note the second step and third step actually still partitioning. So quicksort is just recursively partioning, while merge sort is recursively merging. 
	//Note quick sort is also a divide and conquer algorithm, same as the mergesort for a single array.
	//However,the difference is that quicksort recursively does the partioning, while merge sort recursively does the merging. 

	//****Key Idea in estimating complexity: 
	
	if (low < high) //**** Note this ending condition. 
	{
		/* pi is partitioning index, arr[p] is now
		at right place */
		int pivot = partition(arr, low, high);

		quickSort(arr, low, pivot - 1); //Note the pivot - 1 and pivot + 1
		quickSort(arr, pivot + 1, high);
	}
}

// Driver program to test above functions 
int main()
{
	//**** Some keypoints: [1] recursive on partition, quicksort left and quicksort right (in fact they are still partitioning). [2] start of i and when increase it; start of j and end of j (last but one).
	//**** [3] The closing condition low < high. [4] Details of partitioning part: (a) set i, j initial values. (b) ++i and swap i and j value when condition met. (c) swap i+1 and high element (d) return i+1 the pivot index. 
	//**** IMPORTANT. The key is to partion and let the small values to the left of pivot, and big values to the right to pivot. Memorize this with a picture from the video. 
	//**** In the above implementation, i is to keep the last index of numbers that are smaller than the pivot. 
	int arr[] = { 10, 7, 8, 9, 9,9, 1, 5 };
	int n = sizeof(arr) / sizeof(arr[0]);
	quickSort(arr, 0, n - 1); //**** Note n-1 is the index for the last element. 
	return 0;
}