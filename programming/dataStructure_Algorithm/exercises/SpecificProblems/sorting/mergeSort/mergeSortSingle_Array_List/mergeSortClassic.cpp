#include<stdlib.h> 
#include<stdio.h> 

// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
void merge(int arr[], int l, int m, int r)
{
	//**** The following is the same as the algorithm for merging k-sorted arrays or linked list
	//**** Because here we have only two subarrays, we just the following structure. However, they are same merging action as those algorithm for merging k sorted list. 
	//**** Be careful, in my merge k-sorted approach, I 
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	//// create temp arrays. Merge sort need extra place. 
	//**** The following can be used with gcc but not with VC.
	//int L[n1], R[n2];

	//First way of fixing the above problem. Note I need cast the (void *) to (*int).
	//int * L = (int*) malloc(n1 * sizeof(int));
	//int * R = (int*) malloc(n2 * sizeof(int));
	
	//Second way of fixing the above problem. 
	int * L = new int(n1);
	int * R = new int(n2);

	/* Copy data to temp arrays L[] and R[] */
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1 + j];

	/* Merge the temp arrays back into arr[l..r]*/
	i = 0; // Initial index of first subarray 
	j = 0; // Initial index of second subarray 
	k = l; // Initial index of merged subarray 
	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			arr[k] = L[i];
			i++;
		}
		else
		{
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	/* Copy the remaining elements of L[], if there
	are any */
	while (i < n1)
	{
		arr[k] = L[i];
		i++;
		k++;
	}

	/* Copy the remaining elements of R[], if there
	are any */
	while (j < n2)
	{
		arr[k] = R[j];
		j++;
		k++;
	}
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r) //**** Note the return is void. 
{
	//**** Comparing to quick sort, [1] merge sort is merging recursively but not partition recursively. [2] However, the merging (combining) action is real while in quicksort combining action is trivial.
	//**** Using the recursive structure diagram to completely understand the process of merge sort. 
	//**** Also check the diagram in https://www.geeksforgeeks.org/merge-sort/ However, not when there are two elements left in a subarray, it will not be partitioned again, but just to merge. 
	//**** Be careful of this point when drawing a diagram usig boxes with 123 for each step within each bos. In the graph of Geekforgeeks, the spliting of 2-array of element is misleading. 
	//**** Also note arr is not the final output array. Everytime we call mergeSort, there will be a different arr array.
	//**** Also note either mergeSort or merg() both return void.

	//**** Draw a picture show the recursive process.
	if (l < r) //This is the returning condition, although it returns void. In tree traversal, the NULL is usually the returning condition.
	{
		// Same as (l+r)/2, but avoids overflow for large l and h 
		int m = l + (r - l) / 2; 

		// Sort first and second halves 
		//**** First two recursive calls mergeSort, and the last is the merge().
		mergeSort(arr, l, m);
		mergeSort(arr, m + 1, r);
		merge(arr, l, m, r);
	}
}

/* Driver program to test above functions */
int main()
{
	int arr[] = { 12, 11, 13, 5, 6, 7 };
	int arr_size = sizeof(arr) / sizeof(arr[0]);

	mergeSort(arr, 0, arr_size - 1);

	return 0;
}