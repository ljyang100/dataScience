#ifndef symbolSelection_h_
#define symbolSelection_h_

#include <time.h>
#include<math.h>   
#include<assert.h> 
#include<iostream> 
#include<fstream>  
#include<sstream>  
#include <vector> 
#include <deque>
#include "sharedstructure.h"
#include "AllData.h"
#include <algorithm> //for std::sort, std::unique

class Symbols
{

private:
	std::vector<double> ATR_H, ATR_L, ATR_C;
	double *ATR_output;
	AllData &a;
	std::vector<InputStructure> &m_inputStructureVector; //**** For symbol selection
	std::vector<std::string> &m_exchangeNameVector;
public:
	Symbols(AllData &a_instance, std::vector<InputStructure> &inputStructureVector, std::vector<std::string> &exchangeNameVector)
		:a(a_instance)
		,ATR_H(*new(std::vector<double>))
		,ATR_L(*new(std::vector<double>))
		,ATR_C(*new(std::vector<double>))
		,ATR_output(new(double))
		, m_inputStructureVector(inputStructureVector)
		, m_exchangeNameVector(exchangeNameVector)
	{}

	void setParameters();
	void historyVector();
	void indexVector();
	void findSymbols(std::string exchangeName);
	void combineSymbols();

	~Symbols()
	{
		delete ATR_output;
	}
};

#endif

