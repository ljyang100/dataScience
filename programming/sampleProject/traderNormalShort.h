#ifndef traderNormalShort_h_
#define traderNormalShort_h_

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
#include "TraderEWrapperNormalShort.h"
#include "historicalDataNormal.h"
#include "commonFunctions.h"
#include "AllData.h"
#include <chrono>

class TraderNormalShort :public EWrapperL0
{

private:

	AllData &a;
	CommonFunctions &m_cfs;
	EClientL0 *m_EC;
	int m_numSymbolsPassed;
	int m_numModifications, m_numLoops;

	int m_openOrderCounter;
	TraderEWrapperNormalShort &m_traderEWrapper;
	std::vector<InputStructure> m_inputStructureVectorShort;



public:

	TraderNormalShort(CommonFunctions &cfs, AllData &a_instance, EClientL0 *EC, TraderEWrapperNormalShort &traderEWrapper)
		: m_cfs(cfs)
		, a(a_instance)
		, m_EC(EC)
		, m_traderEWrapper(traderEWrapper)
		, m_numSymbolsPassed(0)
		, m_openOrderCounter(0)

	{}

	static void __cdecl TraderNormalShortStaticEntryPoint(void * pThis)
	{
		TraderNormalShort * pTraderNormalShort = (TraderNormalShort*)pThis;
		pTraderNormalShort->TraderNormalShortEntryPoint();        // call the true entry-point-function
	}
	void TraderNormalShortEntryPoint();

	void createBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void createSellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void placeOrder(unsigned int &reqId, unsigned int &i, std::string action, std::string orderType, unsigned int &relevantShares, double &lmtPrice);
	void modifyBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void modifySellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure);
	void modifyOrder_sub(unsigned int &reqId, std::string action, std::string orderType, unsigned int &totalQuantity, double &lmtPrice);
	void cancelOrder(OrderId &orderId);
	void cancelExistingShortBuyOrder(unsigned int &reqId, std::string reasonToCancel);
	void cancelExistingShortSellOrder(unsigned int &reqId, std::string reasonToCancel);

	~TraderNormalShort()
	{
		delete m_EC;
	}
};

#endif

