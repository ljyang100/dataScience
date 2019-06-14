#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <stack>
//Recursive algorithm O(n^2)
class ValidParentheses{
public:
	bool isValid(std::string s)
	{
		if (s.size() == 0) return true;
		if (s.find("()") != std::string::npos) //Similar to if( it->find(x) != it->end() )
		{
			int index = s.find("()");
			s.replace(index, 2, "");
			return isValid(s);
		}
		else if (s.find("{}") != std::string::npos)
		{
			int index = s.find("{}");
			s.replace(index, 2, "");
			return isValid(s);
		}
		else if (s.find("[]") != std::string::npos)
		{
			int index = s.find("[]");
			s.replace(index, 2, "");
			return isValid(s);
		}
		else
		{
			return false;
		}
	}
};

//Stack algorithm: O(n)
class ValidParenthesesStack{

public:

	bool isValid(std::string s)
	{
		int n = s.size();
		if (n == 0) return true;
		if (n == 1) return false;
		
		std::stack<char> stack;
		if (s[0] == ')' || s[0] == '}' || s[0] == ']') return false;
		else
		{
			stack.push(s[0]);
		}

		for (int i = 1; i < n; ++i)
		{
			if (s[i] == ')' || s[i] == '}' || s[i] == ']')
			{
				if (stack.empty()) return false;
				else
				{
					if (stack.top() == '(' && s[i] == ')' || stack.top() == '[' && s[i] == ']' || stack.top() == '{' && s[i] == '}')
						stack.pop();
					else
						return false;
				}
			}
			else
			{
				stack.push(s[i]);
			}
		}

		if (stack.empty()) return true; else return false;
	}
};

int main() {
	
	std::string s = "(((){})[[])";

	////Part I: recursive O(n^2) approach.
	//ValidParentheses validParentheses;
	bool isValid = false;
	//isValid = validParentheses.isValid(s);

	//Part II: Stack. O(n) 
	ValidParenthesesStack validParenthesesStack;
	isValid = validParenthesesStack.isValid(s);

	return 0;
}