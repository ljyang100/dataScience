#ifndef commonfunctions_h_
#define commonfunctions_h_

#include <time.h>
#include<math.h>   
#include<assert.h> 
#include<iostream> 
#include<fstream>  
#include<sstream>  
#include <vector> 
#include "sharedstructure.h"
#include "CriticalSection.h"

// For TA-lib
#include "ta_common.h"
#include "ta_defs.h"
#include "ta_func.h"
#include "ta_global.h"
#include "ta_memory.h"
// End of Ta-lib

#include "commonFunctions.h"
#include "allData.h"
#include <chrono>
//using namespace std; //**** Prefer not use this statement in the future



class CommonFunctions
{
private:
	AllData &a;
	std::vector<double> ATR_H, ATR_L, ATR_C;
	LineStructure m_lineStructure;
	double *ATR_output;
	std::vector<LineStructure> m_lineVector;


public:

	CommonFunctions(AllData &a_instance)
		: a(a_instance)
		, ATR_H(*new(std::vector<double>))
		, ATR_L(*new(std::vector<double>))
		, ATR_C(*new(std::vector<double>))
		, ATR_output(new(double))
	{}

	~CommonFunctions()
	{

	}

	bool commission(unsigned int &reqId, unsigned int &index, unsigned int &relevantShares, double &realBiasUsed);
	double commissionValue(unsigned int &reqId, unsigned int &index, unsigned int &relevantShares);

	bool commonCode(unsigned int &reqId, unsigned int &index, CurrentDataStructure &currentDataStructure, std::string &algoString, std::string openOrClose);
	void lineFittingTested(unsigned int &i, unsigned int &reqId);
	void lineFittingLocal(unsigned int &i, unsigned int &reqId, std::string minOrMax);
	unsigned int generalSharesLong(unsigned int &i, unsigned int &reqId, std::string String, double &currentPrice);
	unsigned int generalSharesShort(unsigned int &i, unsigned int &reqId, std::string String, double &currentPrice);
	void initialization();
	void calculateBaseVector();
	void calculateBaseVector(unsigned int &reqId);
	void calculateBaseVector_symbol();
	void tradingIndicators(unsigned int &i, unsigned int &reqId);
	void ATR_calculation(unsigned int &i, unsigned int &reqId);
	void volatilityCalculation(unsigned int &i, unsigned int &reqId);
	void misceParameters(unsigned int &i, unsigned int &reqId);
	void misceParameters_Symbol(unsigned int &i, unsigned int &reqId);
	void tradeLines(unsigned int &i, unsigned int &reqId);


};


#endif

