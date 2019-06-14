#include<iostream>
#include<vector>
#include<algorithm> //**** for using std::max.


class FindMaximumConsecutiveOnes{
public:
	//Naive O(n^2) solution 
	int findMaximumConsecutiveOnes(std::vector<int> &nums)
	{
		int maxLength = 0;
		int r;
		for (int i = 0; i < nums.size(); ++i)
		{
			if (nums[i] == 1) //Only consider this case. 
			{
				int currentLength = 1;
				int r = i + 1;
				while (r < nums.size() && nums[r] == 1)
				{
					++r;
					++currentLength;
				}
				maxLength = std::max(maxLength, currentLength);
			}

		}

		return maxLength;
	}

	//Naive O(n^2) solution but more intuitive.
	int findMaximumConsecutiveOnesIntuitive(std::vector<int> &nums)
	{
		//This way is more natural and readable.
		int maxLength = 0; //Must be a variable to record the maximum. 
		for (int i = 0; i < nums.size(); ++i)
		{
			if (nums[i] == 1)
			{
				int currentLength = 1;
				for (int j = i + 1; j < nums.size(); ++j)
				{
					if (nums[j] == 1)
						++currentLength;
					else
						break;
				}

				maxLength = std::max(maxLength, currentLength);
			}
		}

		return maxLength;
	}

	//Optimal O(n) solution-I, with only a tiny modification. Although almost identical, I'd better use the next version for optimal
	int findMaximumConsecutiveOnesOptimal(std::vector<int> &nums)
	{
		//This way is more natural and readable.
		int maxLength = 0; //Must be a variable to record the maximum. 
		int J = 0;
		for (int i = 0; i < nums.size(); ++i)
		{
			if (nums[i] == 1)
			{
				int currentLength = 1;
				for (int j = i + 1; j < nums.size(); ++j)
				{
					J = j;
					if (nums[j] == 1)
						++currentLength;
					else
						break;
				}

				i = J; //****This single line modifies the original O(n^2) to O(n) complexity, although formally it is still with nested for loops.
				maxLength = std::max(maxLength, currentLength);
			}
		}

		return maxLength;
	}

	//Optimal O(n) solution-II. The version I should use in the future. 
	int findMaximumConsecutiveOnesOptimalFinal(std::vector<int> &nums)
	{
		int maxLength = 0;
		int r;
		for (int i = 0; i < nums.size(); ++i)
		{
			if (nums[i] == 1) //Only consider this case. 
			{
				int currentLength = 1;
				int r = i + 1;
				while (r < nums.size() && nums[r] == 1)
				{
					++r;
					++currentLength;
				}

				i = r;
				maxLength = std::max(maxLength, currentLength);
			}
		}
		return maxLength;
	}
};

int main()
{
	int result, result1, result2, result3;
	std::vector<int> nums = { 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0 };
	FindMaximumConsecutiveOnes findMaximumConsecutiveOnes;
	result = findMaximumConsecutiveOnes.findMaximumConsecutiveOnes(nums);
	result1 = findMaximumConsecutiveOnes.findMaximumConsecutiveOnesIntuitive(nums);
	result2 = findMaximumConsecutiveOnes.findMaximumConsecutiveOnesOptimal(nums);
	result3 = findMaximumConsecutiveOnes.findMaximumConsecutiveOnesOptimalFinal(nums);

	return 0;
}