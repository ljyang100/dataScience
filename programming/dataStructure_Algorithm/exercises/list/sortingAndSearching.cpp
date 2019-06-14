#include <stdio.h> //**** Note these are two most common headers.
#include <stdlib.h>
#include <iostream> //**** for std::cout, etc. 
#include <vector>
#include <algorithm>

void bubbleSorting(int array[], int arraySize) //**** Be very careful, not lost the [] after array.
{
	for (int i = 0; i < (arraySize - 1); ++i)
	{
		bool swapped = false;
		for (int j = 0; j < (arraySize - i - 1); ++j)
		{
			if (array[j] > array[j + 1])
			{
				int temp;
				temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
				swapped = true;
			}
		}
		if (swapped == false) break; //**** If the inner loop has no swapping, then it means that the array has already been ordered.
	}
}

int partition(int array[], int low, int high)
{
	int temp;
	int pivot = array[high]; 
	int i = -1;
	for (int j = low; j < high; ++j)
	{
		if (array[j] >= pivot) //This is for descending, so I use >= sign
		{
			i = i + 1;
			temp = array[j];
			array[j] = array[i];
			array[i] = temp;
		}
	}

	temp = array[i + 1];
	array[i + 1] = array[high];
	array[high] = temp;

	return (i + 1);
}

void quickSort(int array[], int low, int high)
{
	if (low < high) //Even for desceding, here is still low < high?
	{
		int pivot = partition(array, low, high);
		quickSort(array, low, pivot - 1);
		quickSort(array, pivot + 1, high);

	}


}


int main(void)
{
	int array[] = { 3, 7, 8, 10, 2, 5, 8 };
	int arraySize = sizeof(array) / sizeof(array[0]);
	//bubbleSorting(array, arraySize); //ascending. tested.
	quickSort(array, 0, arraySize - 1);

	for (int i = 0; i < arraySize; ++i)
		std::cout << array[i] << "," ;

	return 0;
}