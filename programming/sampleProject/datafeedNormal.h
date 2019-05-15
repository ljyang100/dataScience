#ifndef datafeedNormal_h_
#define datafeedNormal_h_

#include <time.h>
#include<math.h>   
#include<assert.h> 
#include<iostream> 
#include<fstream>  
#include<sstream>  
#include <vector> 
#include <deque> //I found if I only include <queue> still fine, even I use deque only. May be because <queue> uses <deque>
#include "sharedstructure.h"
#include "CriticalSection.h"
#include "AllData.h"
#include <chrono>
//using namespace std; //**** Prefer not use this statement in the future


class DatafeedNormal : public EWrapperL0
{

private:
	AllData &a;
	EClientL0 *m_EC; 
	bool m_connectionRestored_and_dataMaintained, m_connectionRestored_and_dataLost;
	time_t m_nextHeartbeat1; //m_initime is shared by many functions, and thus cannot be changed once set.
	time_t m_NextHeartbeat;
	int m_counter300, m_counter322;
	//**** The following shares the same critical section.
	std::vector < int > m_submittedReqIdVector;
	bool m_dataRequestsProcessed; 
	std::vector < int > m_reqIdVector;
	Critical_Section m_csDataRequestAndReqId;
	time_t m_nextHeartBeat;
	std::vector<std::string> m_noMarketDataSymbolVector;
public:

	DatafeedNormal(AllData &a_instance)
		: a(a_instance)
		, m_connectionRestored_and_dataMaintained(false)
	, m_connectionRestored_and_dataLost(false)
	, m_NextHeartbeat(time(0))
	, m_nextHeartbeat1(0)
	, m_counter300(0)
	, m_counter322(0)
	, m_dataRequestsProcessed(false)
	, m_reqIdVector (*new(std::vector<int>))
	, m_csDataRequestAndReqId(*new(Critical_Section))
	, m_submittedReqIdVector(*new(std::vector<int>))
	, m_nextHeartBeat(time(0))
	, m_noMarketDataSymbolVector(*new(std::vector<std::string>))
	{}

	~DatafeedNormal()
	{
		delete m_EC;
	}


	static void __cdecl DatafeedStaticEntryPoint(void * pThis)
	{
		DatafeedNormal * pDatafeed = (DatafeedNormal*)pThis;
		pDatafeed->DatafeedEntryPoint();        // call the true entry-point-function
	}
	void DatafeedEntryPoint();

	//The following functions are always necessary, otherwise error message sent from server will not be shown.
	virtual void error( const int id, const int errorCode, const IBString errorString );
	virtual void winError( const IBString& str, int lastError );
	virtual void tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute);
	virtual void tickSize ( TickerId tickerId, TickType field, int size );
	void set_EC(EClientL0 *p_EC) { m_EC = p_EC; }
};

#endif