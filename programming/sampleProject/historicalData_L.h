#ifndef historicalDataL_h_
#define historicalDataL_h_

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
#include "commonFunctions.h"
#include "AllData.h"
#include <chrono>



//using namespace std; //**** Prefer not use this statement in the future


class HistoricalDataL: public EWrapperL0
{

private:
	EClientL0 *m_EC; 
	std::vector<bool> m_historicalDataEndVector; 
	Critical_Section m_csHistoricalDataEndVector;
	std::vector<std::string> m_liquidStock;
	char   m_buf[80];
	std::string m_durationString, m_barSize;  
	std::vector<double> ATR_H, ATR_L, ATR_C;
	double *ATR_output;
	CommonFunctions m_cfs;
	AllData &a;

	std::vector<std::string> tempVector;
	
public:
	HistoricalDataL(CommonFunctions &cfs, AllData &a_instance)
		: a(a_instance)
		, m_liquidStock(*new(std::vector<std::string>))
		, ATR_H(*new(std::vector<double>))
		, ATR_L(*new(std::vector<double>))
		, ATR_C(*new(std::vector<double>))
		, ATR_output(new(double))
		, m_cfs(cfs)
	{}

	~HistoricalDataL()
	{
		delete m_EC;
	}

	static void __cdecl HistoricalDataStaticEntryPoint(void * pThis)
	{
		HistoricalDataL *pHis = ( HistoricalDataL *)pThis;  
		pHis->downloadHistoricalData_Multiple_L();       
	}
	
	void connectToTWS();
	void downloadHistoricalData_Common();
	void downloadHistoricalData_Single(); 
	void downloadHistoricalData_Multiple_L(); //Note Done must be passed by reference.
	void historicalData( TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps );
	void historicalData_Single(  TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps  ); //Single symbol download. Passed all by reference.
	void historicalData_M_notToday_L( TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps ); //Multiple symbol download. Passed all by reference.
	void error( const int id, const int errorCode, const IBString errorString );
	void scannerParameters(const IBString &xml);
	void scannerData(int reqId, int rank, const ContractDetails &contractDetails,const IBString &distance, const IBString &benchmark, const IBString &projection, const IBString &legsStr);
	void scannerDataEnd(int reqId);


};

#endif