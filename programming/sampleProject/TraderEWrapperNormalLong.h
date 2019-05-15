#ifndef TraderEWrapperNormalLong_h_
#define TraderEWrapperNormalLong_h_
#include "TwsApiL0.h"
#include "sharedstructure.h"
//some of following need to be deleted if not necessary.
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
#include "AllData.h"
#include <chrono>
//using namespace std; //**** Prefer not use this statement in the future

//----------------------------------------------------------------------------
// EWrappers
//----------------------------------------------------------------------------
class TraderEWrapperNormalLong : public EWrapperL0
{

private:
	AllData &a;
	EClientL0 *m_EC;
	time_t m_heartBeat = time(0);


public:
	
	//Need delete parameters if not necessary. 
	TraderEWrapperNormalLong(AllData &a_instance)
		: a(a_instance)
	{}

	~TraderEWrapperNormalLong()
	{

	}

	virtual void error(const int id, const int errorCode, const IBString errorString);
	virtual void winError(const IBString& str, int lastError);

	void set_EC(EClientL0 *p_EC) { m_EC = p_EC; }

	virtual void nextValidId(OrderId orderId);
	virtual void orderStatus(OrderId orderId, const IBString &status, int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const IBString& whyHeld);
	void cancelOrder(OrderId &orderId);
	virtual void position(const IBString& account, const Contract& contract, int position, double avgCost);
	virtual	void positionEnd();

	//****The following are copied from former orderPositionUpdate class. 
	virtual void updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName); //**** This function use the same accountDownloadEnd() as for the updatePortfolio. 
	virtual void updatePortfolio(const Contract& contract, int realPosition, double marketPrice, double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const IBString& accountName);
	virtual void accountDownloadEnd(const IBString& accountName);
	virtual void update_traderStructureVector_by_EWrappers();
	virtual void openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState);
	virtual void openOrderEnd();
	void cancelExistingLongBuyOrder(int &reqId, std::string reasonToCancel);
	void cancelExistingLongSellOrder(int &reqId, std::string reasonToCancel);
	virtual void accountSummary(unsigned int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency);
	virtual void accountSummaryEnd(unsigned int reqId);

};

#endif