#ifndef traderNormalLong_h_
#define traderNormalLong_h_

#include "TwsApiL0.h"
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
#include "TraderEWrapperNormalLong.h"
#include "historicalDataNormal.h"
#include "commonFunctions.h"
#include "AllData.h"
#include <chrono>
//using namespace std; //**** Prefer not use this statement in the future

class TraderNormalLong:public EWrapperL0
{

private:
	AllData &a;
	CommonFunctions &m_cfs;
	EClientL0 *m_EC; //**** I cannot instantiate the EClientL0 class rather than create a pointer to it, as the class is abstract class. 
	int m_numSymbolsPassed;
	int m_numModifications, m_numLoops;

	int m_openOrderCounter;
	TraderEWrapperNormalLong &m_traderEWrapper;
	std::vector<InputStructure> m_inputStructureVectorLong;
	const bool m_open = true, m_close = true;


public:

	TraderNormalLong(CommonFunctions &cfs, AllData &a_instance, EClientL0 *EC, TraderEWrapperNormalLong &traderEWrapper)
		: m_cfs(cfs)
		, a(a_instance)
		, m_EC(EC)
		, m_traderEWrapper(traderEWrapper)
		, m_numSymbolsPassed(0)
		, m_numModifications(0)
		, m_numLoops(0)
		, m_openOrderCounter(0)
	{}

	static void __cdecl TraderNormalLongStaticEntryPoint(void * pThis)
	{
		TraderNormalLong *pTraderNormalLong = (TraderNormalLong*)pThis;
		pTraderNormalLong->TraderNormalLongEntryPoint();  
	}
	void TraderNormalLongEntryPoint();

	void manualReset();
	void cleaning();
	void createBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void createSellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void placeOrder(unsigned int &reqId, unsigned int &i, std::string action, std::string orderType, unsigned int &relevantShares, double &lmtPrice);
	void modifyBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void modifySellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void modifyOrder_sub(unsigned int &reqId, std::string action, std::string orderType, unsigned int &totalQuantity, double &lmtPrice);
	void cancelOrder(OrderId &orderId);
	void cancelExistingLongBuyOrder(unsigned int &reqId, std::string reasonToCancel);
	void cancelExistingLongSellOrder(unsigned int &reqId, std::string reasonToCancel);
	void showAllTradingConditions();
	void showSingleTradingConditions(int &reqId);

	~TraderNormalLong()
	{
		delete m_EC;
	}
};


#endif

