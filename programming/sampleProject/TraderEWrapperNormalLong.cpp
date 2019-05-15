#include "TraderEWrapperNormalLong.h"
void TraderEWrapperNormalLong::error(const int id, const int errorCode, const IBString errorString)
{
	if (errorCode == 1100) //Connection between IB and TWS has been lost.  
	{
		printf("I have set the .traderClientActive = false in TraderEWrapper. \n");
		//**** If internet is suddently disconnected, then orderUpdatedTime may not be updated but some of them may have small .gapOfTwoUpdates. Once internet restored, these small gapOfTwoUpdates still smaller than the required, and thus the app will set many orders to default.
		//**** To prevent this from happening, set the two of the elements below whenever internet is disconnected. 
		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
		{
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_traderVector.longBuy[reqId].orderUpdatedTime = time(0) * 2;	//This is its default.
				a.m_traderVector.longSell[reqId].orderUpdatedTime = time(0) * 2;	//This is its default.
				a.m_csTraderVector.LeaveCriticalSection();
			}
		}
	}
	else if (errorCode == 1101) //Connectivity between IB and TWS (for this client) is restored, but data lost (and thus need request again and account data subscription). Here the request related to this client is only reqMktData(). 
	{
		std::chrono::high_resolution_clock::time_point now; int counter = 0; bool done = false;
		while (1)
		{
			bool b = true;
			try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)
				{
					m_EC->reqPositions();
					now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
					a.m_timeDeque.push_back(now);
					done = true;
				}
				a.m_csTimeDeque.LeaveCriticalSection();
			}

			if (done == true)
			{
				printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("TWS and IB connection restored for this client, but market data subscription maintained and thus I don't have re-submit requests.\n");
				printf("***Check whether the market real-time data is still on.\n");

				break;
			}
			else { Sleep(100); counter = counter + 1; }
			if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
		} //end of while(1)

		//***** Note I don't need wait for the positionEnd = true, as in the trader.cpp.
	} //end of errorCode = 1101
	else if (errorCode == 1102)
	{
		printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("TWS and IB connection restored for this client, but market data subscription maintained and thus I don't have re-submit requests.\n");
		printf("***Check whether the market real-time data is still on.\n");
	}
	else if (errorCode == 110) //minimum price variation not conformed. 
	{
		//****For some specific error like 161 here and 201 below, etc. I may need trigger the openOrder and updatePortfolio updates. 
		bool break1 = false;
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++){
			break1 = false;
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)	{
				if (a.m_traderVector.longBuy[i].orderId == id){
					a.m_traderVector.longBuy[i].minPriceVariation = 0.05; //***** Note this might not right. In the future I need change this to a different value. 
					OrderId orderId = id; OrderId &referenceOrderId = orderId; cancelOrder(referenceOrderId);
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf;
					std::cout << "At " << tempString << ":" << " Check whether it is right to change longBuy minimun price variation to 0.05 for symbol " << a.m_newtxtVector[i] << std::endl;
					break1 = true;
				}
				else if (a.m_traderVector.longSell[i].orderId == id){
					a.m_traderVector.longSell[i].minPriceVariation = 0.05;
					OrderId orderId = id; OrderId &referenceOrderId = orderId; cancelOrder(referenceOrderId);
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf;
					std::cout << "At " << tempString << ":" << " Check whether it is right to change longSell minimun price variation to 0.05 for symbol " << a.m_newtxtVector[i] << std::endl;
					break1 = true;
				}
				a.m_csTraderVector.LeaveCriticalSection();
			} //end of try-catch.	

			if (break1 == true) break;
		} //i loop
	}
	else if (errorCode == 161) //meaning "Cancel attempted when order is not in a cancellable state", and this means the cancel attempt was not successful. In this case, the id above should be orderId. 
	{
		//****For some specific error like 161 here and 201 below, etc. I may need trigger the openOrder and updatePortfolio updates. 
		bool break1 = false;
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			break1 = false;
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_traderVector.longBuy[i].orderId == id)
				{
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; 
					std::cout << "At " << tempString << ":" << " I am handling an unsuccessful cancelled longBuy order for symbol " << a.m_newtxtVector[i] << std::endl;

					break1 = true;
				}
				else if (a.m_traderVector.longSell[i].orderId == id)
				{
					printf("I am handling an unsuccessful cancelled longSell order for symbol %s. \n", a.m_newtxtVector[i].c_str()); 
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					std::cout << "At " << tempString << ":" << " I am handling an unsuccessful cancelled longSell order for symbol " << a.m_newtxtVector[i] << std::endl;
					break1 = true;
				}
				a.m_csTraderVector.LeaveCriticalSection();
			} //end of try-catch.	

			if (break1 == true) break;
		} //i loop
	}
	else if (errorCode == 201) //handling rejected order.
	{
		int reqId = -1;
		bool break1 = false;//****** ****** never delete this because I cannot break within a critical section. 
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_traderVector.longBuy[i].orderId == id)
				{
					reqId = i; 
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper %s: An rejected order for symbol %s has been cancelled due to %s and noBuyLong is set to be true.  \n", tempString.c_str(), a.m_newtxtVector[reqId].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true; //****** ****** never delete this because I cannot break within a critical section. 
				}
				else if (a.m_traderVector.longSell[i].orderId == id)
				{
					reqId = i;
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper %s: An rejected order for symbol %s has been cancelled due to %s and noSellLong is set to be true.  \n", tempString.c_str(), a.m_newtxtVector[reqId].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true;//****** ****** never delete this because I cannot break within a critical section. 
				}

				a.m_csTraderVector.LeaveCriticalSection();
			} //end of try-catch.	

			if (break1) break;//****** ****** never delete this because I cannot break within a critical section. 
		}
	}
	else if (errorCode == 202) //cancelled orders. 
	{
		//Because I will not update position-related elements of traderStructureVector, I don't need separate the treatment of cancellation of an order with and without remaining positions. Thus the order-cancelling is only need to handle in one place, as below. 
		bool break1 = false; //****** ****** never delete this because I cannot break within a critical section. 
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_traderVector.longBuy[i].orderId == id)
				{
					int totalCancellations;
					a.m_traderVector.longBuy[i].totalCancellationsBuy = a.m_traderVector.longBuy[i].totalCancellationsBuy + 1;
					totalCancellations = a.m_traderVector.longBuy[i].totalCancellationsBuy;
					int totalPosition = a.m_traderVector.longBuy[i].totalPosition;
					double averageCost = a.m_traderVector.longBuy[i].averageCost;
					double stopLossLong = a.m_traderVector.longBuy[i].stopLossLong;
					std::string symbol = a.m_newtxtVector[i];
					a.m_traderVector.longBuy[i] = a.m_traderStructureDefault; 
					a.m_traderVector.longBuy[i].symbol = symbol;
					a.m_traderVector.longBuy[i].averageCost = averageCost;
					a.m_traderVector.longBuy[i].totalPosition = totalPosition;
					a.m_traderVector.longBuy[i].totalCancellationsBuy = totalCancellations;
					a.m_traderVector.longBuy[i].stopLossLong = stopLossLong;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper:long %s: An long buy order for symbol %s has been cancelled due to %s.  \n", tempString.c_str(), a.m_newtxtVector[i].c_str(), (const char*)errorString); //Beep(350,200);

					
					break1 = true; //****** ****** never delete this because I cannot break within a critical section. 
				}
				else if (a.m_traderVector.longSell[i].orderId == id)
				{
					std::string symbol = a.m_newtxtVector[i];
					a.m_traderVector.longSell[i].totalCancellationsSell = a.m_traderVector.longSell[i].totalCancellationsSell + 1;
					int totalCancellations = a.m_traderVector.longSell[i].totalCancellationsSell;
					a.m_traderVector.longSell[i] = a.m_traderStructureDefault; 
					a.m_traderVector.longSell[i].symbol = symbol;				//**** Thus .soldSinceLastDefault is also set to be zero. This prepares the app to sell next if position available. 
					a.m_traderVector.longSell[i].totalCancellationsSell = totalCancellations;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper:long %s: An long sell order for symbol %s has been cancelled due to %s.  \n", tempString.c_str(), a.m_newtxtVector[i].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true; //****** ****** never delete this because I cannot break within a critical section. 
				}

				a.m_csTraderVector.LeaveCriticalSection();

			} //end of try-catch.	

			if (break1) break;//****** ****** never delete this because I cannot break within a critical section.
		}
	}
	else if (errorCode == 2100)
	{
		//API client has been unsubscribed from account data. This is due to repetitive subscritio and cancellation of reqUpdateAccount...
	}
	else
	{
		if (id >= 0 && id < (const int)a.m_newtxtVector.size()) 		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
		{
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("TraderEWrapper %s: errorcode %d: %s  MIGHT (the id might not be symbol id) for symbol %s.\n", tempString.c_str(), errorCode, (const char*)errorString, a.m_newtxtVector[id].c_str());
		}
		else
		{
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("TraderEWrapper %s: errorcode %d: %s.\n", tempString.c_str(), errorCode, (const char*)errorString);
		}
	}
}

void TraderEWrapperNormalLong::winError(const IBString& str, int lastError)
{
	printf("WinError: %d = %s\n", lastError, (const char*)str);
}

void TraderEWrapperNormalLong::nextValidId(OrderId orderId)
{
	bool b = true;
	try{ a.m_csOrderIdReadyLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_orderIdLong = orderId; //m_csOrderIdReady protects both m_orderIdReady and m_orderId.
		//**** I combine the orderIdLong/Short to a single orderId. Can we keep different set of orderIds in different clients (within the same account)? 
		if (a.m_orderIdLong > 0) (a.m_orderIdReadyLong) = true;
		a.m_csOrderIdReadyLong.LeaveCriticalSection();
	}

}

void TraderEWrapperNormalLong::orderStatus(OrderId orderId, const IBString &status, int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const IBString& whyHeld)
{
	//**** IMPORTANT: Due to repetitive feed back,I did not add sentences like: a.m_traderVector.longBuy[reqId].buyingShares = a.m_traderVector.longBuy[reqId].currentTotalBuy - filled; Otherwise, it will subtract again and again and give wrong results.
	//**** Also note that "Filled" is not always triggered for marketable orders (From IB website). So I need other condition to obtain that status. 
	bool b = true; //**** .buy/sellingShares is in both m_VectorS and m_traderStructureVector.So whenever we have such an element, it must be simultaneously in the two corresponding critical sections.Otherwise, we cannot guarantee that we modify it in a safe way.Normally don't share element between two critical sections, unless in special case. 
	try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		//**** I need search with a better way, not like below. Do that later.
		//**** If use unorderedMap, then I need a criticalSection-protected unordered map as the map need change all the time as orders coming in all the time.
		int reqId = -1;
		bool break1 = false, buying = false, selling = false;
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++){
			if (a.m_traderVector.longBuy[i].orderId == orderId){  //Be careful in the EWrapper main body, [i].orderId might be changed. This may cause problem when a same-orderStatus is updated many times. 
				reqId = i;	buying = true;
				break1 = true;
			}
			else if (a.m_traderVector.longSell[i].orderId == orderId){
				reqId = i;	selling = true;
				break1 = true;
			}
			if (break1) break;
		}

		if (reqId >= 0 && reqId < a.m_newtxtVector.size()){
			if (buying){
				if (filled != 0){
					a.m_traderVector.longBuy[reqId].filledBuy = filled;
					a.m_traderVector.longBuy[reqId].lastFilledLongTime = time(0);
					if (remaining == 0)	{
						//****From IB website, we know that market order (may include buy at ASK or Sell at BID) cannot always trigger status = "Filled", thus I just use the remaining = 0 replace the "Filled" status.
						//****No! even remaining = 0 will not be triggerred, at least at the time of testing. 
					} // else if (remaining == 0)
				}// filled ! = 0.  
			} //end of if(buying)
			else if (selling){
				if (filled != 0){
					a.m_traderVector.longSell[reqId].filledSell = filled;
					a.m_traderVector.longSell[reqId].filledPrice = avgFillPrice; 

						//***** IMPORTANT: If the "Filled" status also have problems, we can also move those stuff here. 
				}// Filled ! = 0.  
			} // if (selling)
		}

		a.m_csTraderVector.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalLong::update_traderStructureVector_by_EWrappers()
{
	////**** Part I execution details
	time_t initialTime = time(0), finalTime = time(0);
	std::chrono::high_resolution_clock::time_point now;
	
	//**** Part I updatePortfolio
	/////////////////////////////////////////////////
	bool b = true;
	try{ a.m_csUpdatePorfolioUpdateDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_updatePorfolioUpdateDoneLong = false;
		a.m_csUpdatePorfolioUpdateDoneLong.LeaveCriticalSection();
	}

	bool done = false;
	int counter = 0;
	while (1)
	{
		b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				bool subscribe = true;
				m_EC->reqAccountUpdates(subscribe, a.m_accountCode);

				done = true;
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);

				initialTime = time(0);
			}

			a.m_csTimeDeque.LeaveCriticalSection();
		}

		if (done) break; else { Sleep(100); counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	}

	done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csUpdatePorfolioUpdateDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_updatePorfolioUpdateDoneLong) done = true; // I cannot break here because otherwise I will be always in the critical section. 
			a.m_csUpdatePorfolioUpdateDoneLong.LeaveCriticalSection();
		}

		finalTime = time(0);
		if ((finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime)
		{
			bool b = true;
			try{ a.m_csUpdatePorfolioUpdateDoneLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_updatePorfolioUpdateDoneLong = true;

				done = true;
				a.m_csUpdatePorfolioUpdateDoneLong.LeaveCriticalSection();
			}

			printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Waiting too long for downloading account position. Set flag = true and break out. \n"); 
			printf("If application is just started, this might cause problems.\n");
			break;
		} //end of if ( (finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime) 

		if (done) break; else Sleep(100);
	}

	//******* The following is for open order handling. Note I need keep position first and then open order second. This is because in the openOrder() I will use totalPosition to fix other stuff. If then the totalPosition is not available, then it is not good.  
	//**** Part II openOrder
	b = true;
	try{ a.m_csOpenOrderUpdateDoneLong.EnterCriticalSection(); }catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_openOrderUpdateDoneLong = false;
		a.m_csOpenOrderUpdateDoneLong.LeaveCriticalSection();
	}

	counter = 0;
	done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				//**** Letting openOrder() be in a different client as orderStatus before is really a bad idea. For example, in the openOrder() client I cannot cancel bad order because it does not recognize the orderId generated from different client. 
				// bool autoBind = true; m_EC->reqAutoOpenOrders(autoBind); //This can only use clientId = 0;
				//m_EC->reqAllOpenOrders();	
				m_EC->reqOpenOrders(); //****For now (only one client submit orders), the reqOpenOrders() and reqAllOpenOrders() give the same results. 
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);

				done = true;
				initialTime = time(0);
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}

		if (done == true) break; else { Sleep(100); counter = counter + 1; }
		if (counter > (a.m_traderParameters.maxTimeToWaitSendingMessage)) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	}

	done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csOpenOrderUpdateDoneLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_openOrderUpdateDoneLong == true)
			{
				done = true;  //I cannot break here even temp is set to be true. Otherwise I may be out of while loop but still within critical section. And thus other code in different thread may need wait forever. 
			}
			a.m_csOpenOrderUpdateDoneLong.LeaveCriticalSection();
		}

		//Exception handling for long waiting of the end of open order download.
		finalTime = time(0);
		if ((finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime)
		{
			bool b = true;
			try{ a.m_csOpenOrderUpdateDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_openOrderUpdateDoneLong = true;

				//**** If after waiting time, still no End signal, then flag is set as end = true. Then many symbols might not be updated with the information of open order. This is OK because: First, open order does not need update so frequently.
				//**** Second, I have to break out otherwise the application will be stopped here and all other places cannot work. I need make sure each important and necessary function is working.
				done = true;
				a.m_csOpenOrderUpdateDoneLong.LeaveCriticalSection();
			}

			for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
			{
				bool b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_traderVector.longBuy[reqId].orderUpdatedTime = time(0) * 2;
					a.m_traderVector.longSell[reqId].orderUpdatedTime = time(0) * 2;

					a.m_csTraderVector.LeaveCriticalSection();
				}
			}

			printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Waiting too long for downloading open order. Set flag = true and break out. \n"); 
			break;
		} //end of if ( (finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime) 


		if (done == true) break; //This sentence must be out of the critical section. 
		else Sleep(100); //**** Sleep here is very important. Actually Even Sleep(1) is OK. But if I Sleep within the try-catch above, then it will be a pure waste of time, because it does not give openOrderEnd() any chances to run into the same critical section. In such a structure. Sleep must just within while(1), but not within its inner loops.
	}
}

void TraderEWrapperNormalLong::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState)
{
	TraderStructure traderStructure; //used in bubble sorting below. 
	bool b = true;
	try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		int reqId = -1; 
		//**** Note the critical section of newtxtMap is always within the critical section of traderVector.
		auto it = a.m_newtxtMap.find(contract.symbol);
		if (it != a.m_newtxtMap.end()) reqId = it->second;
		

		//***** all the following, either "BUY" or "SELL" case, is only for the cases where the symbol corresponding to the orderId is in the symbol list considered.
		if (order.action == "BUY") //***** This indicates that the reqId value is from the branch starting with if (a.m_traderVector.longBuy[i].orderId == orderId) 
		{
			if (reqId >= 0 && reqId < a.m_newtxtVector.size()) // meaning the symbol in the order is in my recorded vector.
			{
				if (a.m_traderVector.longBuy[reqId].orderId == orderId) //***** In the symbol list and is already recorded in the trader Vector
				{
					if (a.m_traderVector.longBuy[reqId].opening) a.m_traderVector.longBuy[reqId].modifiedBuy = true;

					if (orderState.status != "Filled")
					{
						a.m_traderVector.longBuy[reqId].modifiedBuy = true;
						a.m_traderVector.longBuy[reqId].opening = true;
						// a.m_traderVector.longBuy[reqId].closing = false; //**** Buy and Sell are independent and may exist simultaneously. Thus do not set .closing stuff when there is opening (buy) action. 
						a.m_traderVector.longBuy[reqId].currentTotalBuy = order.totalQuantity;
						a.m_traderVector.longBuy[reqId].buyingShares = a.m_traderVector.longBuy[reqId].currentTotalBuy - a.m_traderVector.longBuy[reqId].filledBuy; // .filledBuy is already in the m_csVectorS critical section.
						a.m_traderVector.longBuy[reqId].myBid = order.lmtPrice;
						a.m_traderVector.longBuy[reqId].action = order.action;
						a.m_traderVector.longBuy[reqId].symbol = contract.symbol;
						a.m_traderVector.longBuy[reqId].orderType = order.orderType;
						a.m_traderVector.longBuy[reqId].orderId = orderId;
						a.m_traderVector.longBuy[reqId].orderUpdatedTime = time(0);
					}
					else if (orderState.status == "Filled")
					{
						int totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
						double averageCost = a.m_traderVector.longBuy[reqId].averageCost; //**** Here I just leave the averageCost untouched because I don't know its value. 
						std::string symbol = a.m_newtxtVector[reqId];
						double stopLossLong = a.m_traderVector.longBuy[reqId].stopLossLong;
						a.m_traderVector.longBuy[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.longBuy[reqId].symbol = symbol;
						a.m_traderVector.longBuy[reqId].averageCost = averageCost;
						a.m_traderVector.longBuy[reqId].totalPosition = totalPosition;
						a.m_traderVector.longBuy[reqId].lastFilledLongTime = time(0);
						a.m_traderVector.longBuy[reqId].stopLossLong = stopLossLong;

		
						a.m_traderVector.longBuy[reqId].lastFilledLongTime = time(0); //**** This is a special one, used to prevent jerking. 
					}
				}
				else //***** In the symbol list and but not recorded in the trader Vector
				{
					//**** I cancel the orderId before, which is a big mistake! After I cancel the orderId, and then store it in the trade Vector below, which makes no sense at all. 
					if (orderState.status != "Filled")
					{
						int totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
						double averageCost = a.m_traderVector.longBuy[reqId].averageCost;
						int filledBuy = a.m_traderVector.longBuy[reqId].filledBuy;
						double stopLossLong = a.m_traderVector.longBuy[reqId].stopLossLong;

						traderStructure = a.m_traderStructureDefault;
						traderStructure.totalPosition = totalPosition;
						traderStructure.averageCost = averageCost;
						traderStructure.buyingShares = order.totalQuantity - filledBuy;
						traderStructure.orderId = orderId;
						traderStructure.opening = true;
						// traderStructure.closing = false; //**** Buy and Sell are independent and may exist simultaneously. Thus do not set .closing stuff when there is opening (buy) action. 
						traderStructure.myBid = order.lmtPrice;
						traderStructure.action = order.action;
						traderStructure.symbol = contract.symbol;
						traderStructure.orderType = order.orderType;
						traderStructure.orderUpdatedTime = time(0);
						traderStructure.currentTotalBuy = order.totalQuantity;
						traderStructure.stopLossLong = stopLossLong;

						a.m_traderVector.longBuy[reqId] = traderStructure;
						//**** modifiedBuy/Sell do not need set to be true explicitly because its default is true.
					}
					else if (orderState.status == "Filled")
					{

						int totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
						double averageCost = a.m_traderVector.longBuy[reqId].averageCost;
						std::string symbol = a.m_newtxtVector[reqId];
						double stopLossLong = a.m_traderVector.longBuy[reqId].stopLossLong;


						a.m_traderVector.longBuy[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.longBuy[reqId].symbol = symbol;
						a.m_traderVector.longBuy[reqId].averageCost = averageCost;
						a.m_traderVector.longBuy[reqId].totalPosition = totalPosition;
						a.m_traderVector.longBuy[reqId].lastFilledLongTime = time(0);
						a.m_traderVector.longBuy[reqId].stopLossLong = stopLossLong;

						a.m_traderVector.longBuy[reqId].lastFilledLongTime = time(0); //**** This is a special one, used to prevent jerking. 
					}
				}
			} // end of if (reqId >= 0 && reqId < a.m_newtxtVector.size())
		}

		if (order.action == "SELL")
		{
			if (reqId >= 0 && reqId < a.m_newtxtVector.size()) // the contract is in the symbol list considered.
			{
				if (orderId == a.m_traderVector.longSell[reqId].orderId) //***** In the symbol list and is already recorded in the trader Vector
				{
					if (a.m_traderVector.longSell[reqId].closing) a.m_traderVector.longSell[reqId].modifiedSell = true;
					if (orderState.status != "Filled")
					{
						//a.m_traderVector.longSell[reqId].opening = false; //**** Buy and Sell are independent may exist simultaneously. Thus do not set .opening = false, as it may trigger a second buy. 
						a.m_traderVector.longSell[reqId].closing = true;
						a.m_traderVector.longSell[reqId].currentTotalSell = order.totalQuantity;
						a.m_traderVector.longSell[reqId].sellingShares = a.m_traderVector.longSell[reqId].currentTotalSell - a.m_traderVector.longSell[reqId].filledSell;
						a.m_traderVector.longSell[reqId].myAsk = order.lmtPrice;
						a.m_traderVector.longSell[reqId].action = order.action;
						a.m_traderVector.longSell[reqId].symbol = contract.symbol;
						a.m_traderVector.longSell[reqId].orderType = order.orderType;
						a.m_traderVector.longSell[reqId].orderId = orderId;
						a.m_traderVector.longSell[reqId].orderUpdatedTime = time(0);
					}
					else if (orderState.status == "Filled") //**** This might be the second or multiple cases of setting fullyFilledSell = true. This is OK because system will responds only once. 
					{
						a.m_traderVector.longSell[reqId].modifiedSell = true;
						std::string symbol = a.m_newtxtVector[reqId];
						a.m_traderVector.longSell[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.longSell[reqId].symbol = symbol;

						a.m_traderVector.longBuy[reqId].stopLossLong = 0;

						if (a.m_traderVector.longBuy[reqId].totalPosition == 0)
							a.m_traderVector.longBuy[reqId].algoString = "EMPTY";
						//**** No deletion: m_algoStringVector shares the critical section of traderVector. Thus it must always be within that critical section. 

					}
				}
				else //***** In the symbol list and but not recorded in the trader Vector
				{
					if (orderState.status != "Filled")
					{
						int filledSell = a.m_traderVector.longSell[reqId].filledSell;
						traderStructure = a.m_traderStructureDefault;
						traderStructure.sellingShares = order.totalQuantity - filledSell;
						traderStructure.orderId = orderId;
						// traderStructure.opening = false;
						traderStructure.closing = true;
						traderStructure.myAsk = order.lmtPrice;
						traderStructure.action = order.action;
						traderStructure.symbol = contract.symbol;
						traderStructure.orderType = order.orderType;
						traderStructure.orderUpdatedTime = time(0);
						traderStructure.currentTotalSell = order.totalQuantity;
						a.m_traderVector.longSell[reqId] = traderStructure;
					}
					else if (orderState.status == "Filled") //**** This might be the second or multiple cases of setting fullyFilledSell = true. This is OK because system will responds only once. 
					{
						std::string symbol = a.m_newtxtVector[reqId];
						a.m_traderVector.longSell[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.longSell[reqId].symbol = symbol;
						
						a.m_traderVector.longBuy[reqId].stopLossLong = 0;

						if (a.m_traderVector.longBuy[reqId].totalPosition == 0) 
							a.m_traderVector.longBuy[reqId].algoString = "EMPTY";						//**** No deletion: m_algoStringVector shares the critical section of traderVector. Thus it must always be within that critical section. 

					}
				}
			}
		}
		a.m_csTraderVector.LeaveCriticalSection();
	} // end of try{ a.m_csTraderVector.EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
}

void TraderEWrapperNormalLong::openOrderEnd()
{
	//**** Part III: open order does not exist, but records may exist or not. Anyway set to default is OK.
	//**** Seem hard to find another way to handle the this part of problem. The key reason is that we have to wait the all open orders are downloaded and then we can decide whether they are consistent with the records in the memory vector. 
	bool b = true;
	try{ a.m_csOpenOrderUpdateDoneLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_openOrderUpdateDoneLong = true;
		a.m_csOpenOrderUpdateDoneLong.LeaveCriticalSection();
	}

}

void TraderEWrapperNormalLong::updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName) // After m_EC->reqAccountUpdates(subscribe, m_accountCode), two EWrapper will receive: updateAccountValue and updatePortfolio
{
	int a;
	a = 1;
}

void TraderEWrapperNormalLong::updatePortfolio(const Contract& contract, int realPosition, double marketPrice, double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const IBString& accountName)
{
	if (realPosition >= 0) // It seems IB feed back all long and short positions to all clients. So I need this condition. 
	{
		int i_value = -1;
		bool b = true;
		try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			//**** This also need the protection of traderVector critical section as it might change during trading (in the future)
			auto it = a.m_newtxtMap.find(contract.symbol); 
			if (it != a.m_newtxtMap.end()) i_value = it->second;

			if (i_value >= 0 && i_value < a.m_newtxtVector.size())
			{
				a.m_traderVector.longBuy[i_value].totalPosition = realPosition; //**** Even a short position is closed, it is fine to set longBuy.totalPosition = 0. 
				a.m_traderVector.longBuy[i_value].averageCost = averageCost;
				a.m_traderVector.longBuy[i_value].positionUpdatedTime = time(0);
			}
			a.m_csTraderVector.LeaveCriticalSection();
		}
	}
}//updatePortfolio function end.

void TraderEWrapperNormalLong::accountDownloadEnd(const IBString& accountName)
{
	std::chrono::high_resolution_clock::time_point now;
	bool b = true;
	try{ a.m_csUpdatePorfolioUpdateDoneLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_updatePorfolioUpdateDoneLong = true;
		a.m_csUpdatePorfolioUpdateDoneLong.LeaveCriticalSection();
	}

	bool done = false;
	int counter = 0;
	while (1)
	{
		b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				bool subscribe = false;
				m_EC->reqAccountUpdates(subscribe, a.m_accountCode); //**** requesting with subscribe = false means to cancel the request. Without this, this function can subscribe once and get data every three minutes.

				done = true;
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
			}

			a.m_csTimeDeque.LeaveCriticalSection();
		}

		if (done) break; else { Sleep(100); counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	}
}

void TraderEWrapperNormalLong::position(const IBString& account, const Contract& contract, int position, double avgCost)
{
	//**** Note the = sign below is necessary when a position is closed. 
	if (position >= 0) // It seems IB feed back all long and short positions to all clients. So I need this condition
	{
		int i_value = -1;
		bool b = true;
		try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			//**** This also need the protection of traderVector critical section as it might change during trading (in the future)
			auto it = a.m_newtxtMap.find(contract.symbol); 
			if (it != a.m_newtxtMap.end()) i_value = it->second;
			if (i_value >= 0 && i_value < a.m_newtxtVector.size())
			{
				a.m_traderVector.longBuy[i_value].totalPosition = position;
				a.m_traderVector.longBuy[i_value].averageCost = avgCost;
				a.m_traderVector.longBuy[i_value].positionUpdatedTime = time(0);
			}
			a.m_csTraderVector.LeaveCriticalSection();
		}
	}
}

void TraderEWrapperNormalLong::positionEnd()
{
	bool b = true;
	try{ a.m_csPositionUpdateDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_positionUpdateDoneLong = true;
		a.m_csPositionUpdateDoneLong.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalLong::accountSummary(unsigned int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency)
{
	int a;
	a = 1;
}

void TraderEWrapperNormalLong::accountSummaryEnd(unsigned int reqId)
{
	bool b = true;
	try{ a.m_csAccountSummaryDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_accountSummaryDoneLong = true;
		a.m_csAccountSummaryDoneLong.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalLong::cancelOrder(OrderId &orderId)
{
	std::chrono::high_resolution_clock::time_point now;
	int counter = 0;
	bool done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				m_EC->cancelOrder(orderId);

				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);

				done = true;
			}

			a.m_csTimeDeque.LeaveCriticalSection();
		}

		if (done == true) break; else { Sleep(100); counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	}

}

void TraderEWrapperNormalLong::cancelExistingLongBuyOrder(int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.longBuy[reqId].cancelling)
	{
		if (a.m_traderVector.longBuy[reqId].orderId > 0 && a.m_traderVector.longBuy[reqId].opening == true)
		{
			cancelOrder(a.m_traderVector.longBuy[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.longBuy[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("At %s, A longBuy order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

void TraderEWrapperNormalLong::cancelExistingLongSellOrder(int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.longSell[reqId].cancelling)
	{
		if (a.m_traderVector.longSell[reqId].orderId > 0 && a.m_traderVector.longSell[reqId].closing == true)
		{
			cancelOrder(a.m_traderVector.longSell[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.longSell[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("At %s, A longSell order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

