#ifndef historicalDataNormal_h_
#define historicalDataNormal_h_

#include <time.h>
#include<math.h>   
#include<assert.h> 
#include<iostream> 
#include<fstream>  
#include<sstream>  
#include <vector> 
#include <deque>
#include "sharedstructure.h"
#include "CriticalSection.h"


// For TA-lib
#include "ta_common.h"
#include "ta_defs.h"
#include "ta_func.h"
#include "ta_global.h"
#include "ta_memory.h"
// End of Ta-lib

#include "AllData.h"
#include "commonFunctions.h"
#include <chrono>



class HistoricalDataNormal : public EWrapperL0
{

private:
	EClientL0 *m_EC; 
	std::vector<bool> m_historicalDataEndVector; 
	Critical_Section m_csHistoricalDataEndVector;
	std::vector<std::string> m_liquidStock;
	char   m_buf[80];
	std::string m_durationString, m_barSize;  

	CommonFunctions &m_cfs;
	AllData &a;

	unsigned int m_numSmallBarsOfDay;

public:
	
	HistoricalDataNormal(CommonFunctions &cfs, AllData &a_instance)
		: a(a_instance)
		, m_cfs(cfs)
		, m_liquidStock(*new(std::vector<std::string>))
	{}

	~HistoricalDataNormal()
	{
		delete m_EC;
	}

	static void __cdecl HistoricalDataStaticEntryPoint(void * pThis)
	{
		HistoricalDataNormal *pHis = (HistoricalDataNormal *)pThis;
		pHis->downloadHistoricalData_multiple_duringTrading();
	}
	virtual void downloadHistoricalData_multiple_duringTrading();

	virtual void connectToTWS();
	virtual void downloadHistoricalData_Common();
	virtual void downloadHistoricalData_Single(); 
	virtual void downloadHistoricalData_multiple_beforeTrading(); //Note Done must be passed by reference.
	virtual void historicalData( TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps );
	virtual void historicalData_Single(  TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps  ); //Single symbol download. Passed all by reference.
	virtual void historicalData_multiple_beforeTrading( TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps ); //Multiple symbol download. Passed all by reference.
	virtual void historicalData_multiple_duringTrading(TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps); //Multiple symbol download. Passed all by reference.
	virtual void error( const int id, const int errorCode, const IBString errorString );
	virtual void winError( const IBString& str, int lastError );
};

#endif