
#include <stdio.h> //**** Note these are two most common headers.
#include <stdlib.h>
#include <iostream> //**** for std::cout, etc. 
#include <vector>
#include <algorithm>


int binarySearch(std::vector<std::string> &string, int low, int high, std::string searchString)
{
	if (high >= low)
	{
		int mid = low + (high - low) / 2;
		if (string[mid] == searchString)
			return mid;
		if (string[mid] > searchString)
			return binarySearch(string, low, mid - 1, searchString);
		if (string[mid] < searchString)
			return binarySearch(string, mid + 1, high, searchString);
	}

	return -1; //indicating the element is not in the vector. 
}

int main(void)
{
	std::vector<std::string> string = { "at", "", "", "", "ball", "", "", "car", "dad", "", "" };
	//std::vector<std::string> string = { "at", "au", "av", "aw", "ball", "bb", "bc", "car", "dad", "db", "dc" };
	std::sort(string.begin(), string.end()); //**** Note I cannot search an unordered array using binary searching method.
	int index;
	index = binarySearch(string, 0, string.size() - 1, "ball");

	if (index == -1)
		std::cout << "The element ball is not in the string array." << std::endl;
	else
		std::cout << "The found index for element ball is in the " << index << "-th position" << std::endl;

	return 0;
}