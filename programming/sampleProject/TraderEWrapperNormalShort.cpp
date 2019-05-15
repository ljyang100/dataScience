
#include "TraderEWrapperNormalShort.h"

void TraderEWrapperNormalShort::error(const int id, const int errorCode, const IBString errorString)
{
	if (errorCode == 1100) //Connection between IB and TWS has been lost.  
	{
		//**** If internet is suddently disconnected, then orderUpdatedTime may not be updated but some of them may have small .gapOfTwoUpdates. Once internet restored, these small gapOfTwoUpdates still smaller than the required, and thus the app will set many orders to default.
		//**** To prevent this from happening, set the two of the elements below whenever internet is disconnected. 
		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
		{
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_traderVector.shortSell[reqId].orderUpdatedTime = time(0) * 2;	//This is its default.
				a.m_traderVector.shortBuy[reqId].orderUpdatedTime = time(0) * 2;	//This is its default.
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
				if (a.m_traderVector.shortBuy[i].orderId == id){
					a.m_traderVector.shortBuy[i].minPriceVariation = 0.05; //***** Note this might not right. In the future I need change this to a different value. 
					OrderId orderId = id; OrderId &referenceOrderId = orderId; cancelOrder(referenceOrderId);
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf;
					std::cout << "At " << tempString << ":" << " Check whether it is right to change shortBuy minimun price variation to 0.05 for symbol " << a.m_newtxtVector[i] << std::endl;
					break1 = true;
				}
				else if (a.m_traderVector.shortSell[i].orderId == id){
					a.m_traderVector.shortSell[i].minPriceVariation = 0.05;
					OrderId orderId = id; OrderId &referenceOrderId = orderId; cancelOrder(referenceOrderId);
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf;
					std::cout << "At " << tempString << ":" << " Check whether it is right to change shortSell minimun price variation to 0.05 for symbol " << a.m_newtxtVector[i] << std::endl;
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
		int reqId = -1;
		bool break1 = false;
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			break1 = false;
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_traderVector.shortSell[i].orderId == id )
				{
					reqId = i; //reqId is always OK for the vector range because i is within the correct range. 
					printf("I am handling an unsuccessful cancelled shortSell order for symbol %s. \n", a.m_newtxtVector[reqId].c_str()); 
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					std::cout << "At " << tempString << ":" << " I am handling an unsuccessful cancelled shortSell order for symbol " << a.m_newtxtVector[i] << std::endl;

					break1 = true;
				}
				else if (a.m_traderVector.shortBuy[i].orderId == id)
				{
					reqId = i;
					printf("I am handling an unsuccessful cancelled shortBuy order for symbol %s. \n", a.m_newtxtVector[reqId].c_str());
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					std::cout << "At " << tempString << ":" << " I am handling an unsuccessful cancelled shortBuy order for symbol " << a.m_newtxtVector[i] << std::endl;
					break1 = true;
				}

				a.m_csTraderVector.LeaveCriticalSection();
			} //end of try-catch.	

			if (break1 == true) break;
		} //reqId loop
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
				if (a.m_traderVector.shortBuy[i].orderId == id)
				{
					reqId = i;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper %s: An rejected order for symbol %s has been cancelled due to %s and noBuyShort is set to be true.  \n", tempString.c_str(), a.m_newtxtVector[reqId].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true;//****** ****** never delete this because I cannot break within a critical section.
				}
				else if (a.m_traderVector.shortSell[i].orderId == id)
				{
					reqId = i;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper %s: An rejected order for symbol %s has been cancelled due to %s and noSellShort is set to be true.  \n", tempString.c_str(), a.m_newtxtVector[reqId].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true;//****** ****** never delete this because I cannot break within a critical section.
				}
				a.m_csTraderVector.LeaveCriticalSection();
			} //end of try-catch.	
			if (break1) break; //****** ****** never delete this because I cannot break within a critical section.
		}
	}
	else if (errorCode == 202) //cancelled orders. 
	{
		//Because I will not update position-related elements of traderStructureVector, I don't need separate the treatment of cancellation of an order with and without remaining positions. Thus the order-cancelling is only need to handle in one place, as below. 
		bool break1 = false;//****** ****** never delete this because I cannot break within a critical section.
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			bool b = true;
			try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_traderVector.shortSell[i].orderId == id)
				{
					int totalCancellations;
					a.m_traderVector.shortSell[i].totalCancellationsSell = a.m_traderVector.shortSell[i].totalCancellationsSell + 1;
					totalCancellations = a.m_traderVector.shortSell[i].totalCancellationsSell;
					int totalPosition = a.m_traderVector.shortSell[i].totalPosition;
					double averageCost = a.m_traderVector.shortSell[i].averageCost;
					double stopLossShort = a.m_traderVector.shortSell[i].stopLossShort;
					std::string symbol = a.m_newtxtVector[i];
					a.m_traderVector.shortSell[i] = a.m_traderStructureDefault; 
					a.m_traderVector.shortSell[i].symbol = symbol;
					a.m_traderVector.shortSell[i].averageCost = averageCost;
					a.m_traderVector.shortSell[i].totalPosition = totalPosition;
					a.m_traderVector.shortSell[i].totalCancellationsSell = totalCancellations;
					a.m_traderVector.shortSell[i].stopLossShort = stopLossShort;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper:short %s: An short sell order for symbol %s has been cancelled due to %s.  \n", tempString.c_str(), a.m_newtxtVector[i].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true;//****** ****** never delete this because I cannot break within a critical section.
				}
				else if (a.m_traderVector.shortBuy[i].orderId == id)
				{
					std::string symbol = a.m_newtxtVector[i];
					int totalCancellations;
					a.m_traderVector.shortBuy[i].totalCancellationsBuy = a.m_traderVector.shortBuy[i].totalCancellationsBuy + 1;
					totalCancellations = a.m_traderVector.shortBuy[i].totalCancellationsBuy;
					a.m_traderVector.shortBuy[i] = a.m_traderStructureDefault; 
					a.m_traderVector.shortBuy[i].symbol = symbol;				//**** Thus .soldSinceLastDefault is also set to be zero. This prepares the app to sell next if position available. 
					a.m_traderVector.shortBuy[i].totalCancellationsBuy = totalCancellations;

					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					printf("TraderEWrapper:short %s: An short buy order for symbol %s has been cancelled due to %s.  \n", tempString.c_str(), a.m_newtxtVector[i].c_str(), (const char*)errorString); //Beep(350,200);

					break1 = true;//****** ****** never delete this because I cannot break within a critical section.
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
			int reqId = id; //**** Just for a unification of vector index for a.m_newtxtVector[reqId].
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("TraderEWrapper %s: errorcode %d: %s  MIGHT (the id might not be symbol id) for symbol %s.\n", tempString.c_str(), errorCode, (const char*)errorString, a.m_newtxtVector[reqId].c_str());
		}
		else
		{
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			printf("TraderEWrapper %s: errorcode %d: %s.\n", tempString.c_str(), errorCode, (const char*)errorString);
		}
	}
}

void TraderEWrapperNormalShort::winError(const IBString& str, int lastError)
{
	printf("WinError: %d = %s\n", lastError, (const char*)str);
}

void TraderEWrapperNormalShort::nextValidId(OrderId orderId)
{
	bool b = true;
	try{ a.m_csOrderIdReadyShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_orderIdShort = orderId; //m_csOrderIdReady protects both m_orderIdReady and m_orderId.
		//**** m_orderId is the same pointer in trader, start, EWrapper. So I don't need pass this value to trader. 
		if (a.m_orderIdShort > 0) (a.m_orderIdReadyShort) = true;
		a.m_csOrderIdReadyShort.LeaveCriticalSection();
	}

}

void TraderEWrapperNormalShort::orderStatus(OrderId orderId, const IBString &status, int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice, int clientId, const IBString& whyHeld)
{
	// **** IMPORTANT: Here I should only get non-accumulative quantities because the same signal may come several times. For example, I can obtain the variable "int filled ". It is OK it arrives several times.
	// **** Also I cannot rely on the fully filled status "Filled" because Market orders executions will not always trigger a Filled status. I have found that even non-market order will not trigger a FILLED status.
	bool b = true; 
	try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b){
		int reqId = -1;
		bool break1 = false, buying = false, selling = false;
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++){
			if (a.m_traderVector.shortSell[i].orderId == orderId){
				reqId = i;	selling = true;
				break1 = true;
			}
			else if (a.m_traderVector.shortBuy[i].orderId == orderId){
				reqId = i;	buying = true;
				break1 = true;
			}

			if (break1) break;
		}


		if (reqId >= 0 && reqId < a.m_newtxtVector.size()){
			if (selling){
				if (filled != 0){
					a.m_traderVector.shortSell[reqId].filledSell = filled;
					a.m_traderVector.shortSell[reqId].lastFilledShortTime = time(0);
				}// filled ! = 0.  
			} //end of if(selling)
			else if (buying){
				if (filled != 0){
					a.m_traderVector.shortBuy[reqId].filledBuy = filled;
				}// Filled ! = 0.  
			} // if (buying)
		}
		a.m_csTraderVector.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalShort::update_traderStructureVector_by_EWrappers()
{
	time_t initialTime = time(0), finalTime = time(0);
	std::chrono::high_resolution_clock::time_point now;

	bool b = true;
	try{ a.m_csUpdatePorfolioUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_updatePorfolioUpdateDoneShort = false;
		a.m_csUpdatePorfolioUpdateDoneShort.LeaveCriticalSection();
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
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	}

	done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csUpdatePorfolioUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_updatePorfolioUpdateDoneShort) done = true; // I cannot break here because otherwise I will be always in the critical section. 
			a.m_csUpdatePorfolioUpdateDoneShort.LeaveCriticalSection();
		}

		finalTime = time(0);
		if ((finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime)
		{
			bool b = true;
			try{ a.m_csUpdatePorfolioUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_updatePorfolioUpdateDoneShort = true;

				done = true;
				a.m_csUpdatePorfolioUpdateDoneShort.LeaveCriticalSection();
			}

			printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Waiting too long for downloading account position. \n"); 
			printf("If application is just started, this might cause problems.\n");
			break;
		} //end of if ( (finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime) 

		if (done) break; else Sleep(100);
	}

	//******* The following is for open order handling. Note I need keep position first and then open order second. This is because in the openOrder() I will use totalPosition to fix other stuff. If then the totalPosition is not available, then it is not good.  

	b = true;
	try{ a.m_csOpenOrderUpdateDoneShort.EnterCriticalSection(); }
	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_openOrderUpdateDoneShort = false;
		a.m_csOpenOrderUpdateDoneShort.LeaveCriticalSection();
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
		if (counter > (a.m_traderParameters.maxTimeToWaitSendingMessage)) { printf("***CS*** %s %d\n", __FILE__, __LINE__); ; }
	}

	done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csOpenOrderUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_openOrderUpdateDoneShort == true)
			{
				done = true;  //I cannot break here even temp is set to be true. Otherwise I may be out of while loop but still within critical section. And thus other code in different thread may need wait forever. 
			}
			a.m_csOpenOrderUpdateDoneShort.LeaveCriticalSection();
		}

		//Exception handling for long waiting of the end of open order download.
		finalTime = time(0);
		if ((finalTime - initialTime) > a.m_traderParameters.orderPositionWaitingTime)
		{
			bool b = true;
			try{ a.m_csOpenOrderUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				a.m_openOrderUpdateDoneShort = true;

				//**** If after waiting time, still no End signal, then flag is set as end = true. Then many symbols might not be updated with the information of open order. This is OK because: First, open order does not need update so frequently.
				//**** Second, I have to break out otherwise the application will be stopped here and all other places cannot work. I need make sure each important and necessary function is working.
				done = true;
				a.m_csOpenOrderUpdateDoneShort.LeaveCriticalSection();
			}

			for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
			{
				bool b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_traderVector.shortSell[reqId].orderUpdatedTime = time(0) * 2;
					a.m_traderVector.shortBuy[reqId].orderUpdatedTime = time(0) * 2;

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

void TraderEWrapperNormalShort::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState)
{
	TraderStructure traderStructure;
	bool b = true;
	try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		int reqId = -1;
		//**** This also need the protection of traderVector critical section as it might change during trading (in the future)
		auto it = a.m_newtxtMap.find(contract.symbol);
		if (it != a.m_newtxtMap.end()) reqId = it->second;

		if (order.action == "SELL") //***** This indicates that the reqId value is from the branch starting with if (a.m_traderVector.shortSell[i].orderId == orderId) 
		{
			if (reqId >= 0 && reqId < a.m_newtxtVector.size()) // meaning the symbol in the order is in my recorded vector.
			{
				if (a.m_traderVector.shortSell[reqId].orderId == orderId) //***** In the symbol list and is already recorded in the trader Vector
				{
					if (a.m_traderVector.shortSell[reqId].opening == true) a.m_traderVector.shortSell[reqId].modifiedSell = true;

					if (orderState.status != "Filled")
					{
						a.m_traderVector.shortSell[reqId].modifiedSell = true;
						a.m_traderVector.shortSell[reqId].opening = true;
						a.m_traderVector.shortSell[reqId].currentTotalSell = order.totalQuantity;
						a.m_traderVector.shortSell[reqId].sellingShares = a.m_traderVector.shortSell[reqId].currentTotalSell - a.m_traderVector.shortSell[reqId].filledSell; // .filledSell is already in the m_csVectorS critical section.
						a.m_traderVector.shortSell[reqId].myAsk = order.lmtPrice;
						a.m_traderVector.shortSell[reqId].action = order.action;
						a.m_traderVector.shortSell[reqId].symbol = contract.symbol;
						a.m_traderVector.shortSell[reqId].orderType = order.orderType;
						a.m_traderVector.shortSell[reqId].orderId = orderId;
						a.m_traderVector.shortSell[reqId].orderUpdatedTime = time(0);
					}
					else if (orderState.status == "Filled")
					{
						int totalPosition = a.m_traderVector.shortSell[reqId].totalPosition;
						double averageCost = a.m_traderVector.shortSell[reqId].averageCost; //**** Here I just leave the averageCost untouched because I don't know its value. 
						std::string symbol = a.m_newtxtVector[reqId];
						double stopLossShort = a.m_traderVector.shortSell[reqId].stopLossShort;

						a.m_traderVector.shortSell[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.shortSell[reqId].symbol = symbol;
						a.m_traderVector.shortSell[reqId].averageCost = averageCost;
						a.m_traderVector.shortSell[reqId].totalPosition = totalPosition;
						a.m_traderVector.shortSell[reqId].lastFilledShortTime = time(0);
						a.m_traderVector.shortSell[reqId].stopLossShort =stopLossShort;

						a.m_traderVector.shortSell[reqId].lastFilledShortTime = time(0);

					}
				}
				else //***** In the symbol list and but not recorded in the trader Vector
				{
					if (orderState.status != "Filled")
					{
						int totalPosition = a.m_traderVector.shortSell[reqId].totalPosition;
						double averageCost = a.m_traderVector.shortSell[reqId].averageCost;
						int filledSell = a.m_traderVector.shortSell[reqId].filledSell;
						double stopLossShort = a.m_traderVector.shortSell[reqId].stopLossShort;

						traderStructure = a.m_traderStructureDefault;
						traderStructure.totalPosition = totalPosition;
						traderStructure.averageCost = averageCost;
						traderStructure.sellingShares = order.totalQuantity - filledSell;
						traderStructure.orderId = orderId;
						traderStructure.opening = true;
						//traderStructure.closing = false;
						traderStructure.myAsk = order.lmtPrice;
						traderStructure.action = order.action;
						traderStructure.symbol = contract.symbol;
						traderStructure.orderType = order.orderType;
						traderStructure.orderUpdatedTime = time(0);
						traderStructure.currentTotalSell = order.totalQuantity;
						traderStructure.stopLossShort = stopLossShort;

						a.m_traderVector.shortSell[reqId] = traderStructure;
					}
					else if (orderState.status == "Filled")
					{
						int totalPosition = a.m_traderVector.shortSell[reqId].totalPosition;
						double averageCost = a.m_traderVector.shortSell[reqId].averageCost; //**** Here I just leave the averageCost untouched because I don't know its value. 
						std::string symbol = a.m_newtxtVector[reqId];
						double stopLossShort = a.m_traderVector.shortSell[reqId].stopLossShort;

						a.m_traderVector.shortSell[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.shortSell[reqId].symbol = symbol;
						a.m_traderVector.shortSell[reqId].averageCost = averageCost;
						a.m_traderVector.shortSell[reqId].totalPosition = totalPosition;
						a.m_traderVector.shortSell[reqId].lastFilledShortTime = time(0);
						a.m_traderVector.shortSell[reqId].stopLossShort = stopLossShort;

						a.m_traderVector.shortSell[reqId].lastFilledShortTime = time(0);
					}
				}
			}
		}

		if (order.action == "BUY")
		{
			if (reqId >= 0 && reqId < a.m_newtxtVector.size()) // the contract is in the symbol list considered.
			{
				if (orderId == a.m_traderVector.shortBuy[reqId].orderId) //***** In the symbol list and is already recorded in the trader Vector
				{
					if (a.m_traderVector.shortBuy[reqId].closing == true)	a.m_traderVector.shortBuy[reqId].modifiedBuy = true;
					if (orderState.status != "Filled")
					{
						//a.m_traderVector.shortBuy[reqId].opening = false;
						a.m_traderVector.shortBuy[reqId].closing = true;
						a.m_traderVector.shortBuy[reqId].currentTotalBuy = order.totalQuantity;
						a.m_traderVector.shortBuy[reqId].buyingShares = a.m_traderVector.shortBuy[reqId].currentTotalBuy - a.m_traderVector.shortBuy[reqId].filledBuy;
						a.m_traderVector.shortBuy[reqId].myBid = order.lmtPrice;
						a.m_traderVector.shortBuy[reqId].action = order.action;
						a.m_traderVector.shortBuy[reqId].symbol = contract.symbol;
						a.m_traderVector.shortBuy[reqId].orderType = order.orderType;
						a.m_traderVector.shortBuy[reqId].orderId = orderId;
						a.m_traderVector.shortBuy[reqId].orderUpdatedTime = time(0);
					}
					else if (orderState.status == "Filled") //**** This might be the second or multiple cases of setting fullyFilledSell = true. This is OK because system will responds only once. 
					{
						a.m_traderVector.shortBuy[reqId].modifiedBuy = true;
						std::string symbol = a.m_newtxtVector[reqId];
						a.m_traderVector.shortBuy[reqId] = a.m_traderStructureDefault; 
						a.m_traderVector.shortBuy[reqId].symbol = symbol;
						a.m_traderVector.shortSell[reqId].stopLossShort = 0;

						if (a.m_traderVector.shortSell[reqId].totalPosition == 0)
							a.m_traderVector.shortSell[reqId].algoString = "EMPTY";
						//**** No deletion: m_algoStringVector shares the critical section of traderVector. Thus it must always be within that critical section. 

					}
				}
				else //***** In the symbol list and but not recorded in the trader Vector
				{
					if (orderState.status != "Filled")
					{
						int filledBuy = a.m_traderVector.shortBuy[reqId].filledBuy;
						traderStructure = a.m_traderStructureDefault;
						traderStructure.buyingShares = order.totalQuantity - filledBuy;
						traderStructure.orderId = orderId;
						//traderStructure.opening = false;
						traderStructure.closing = true;
						traderStructure.myBid = order.lmtPrice;
						traderStructure.action = order.action;
						traderStructure.symbol = contract.symbol;
						traderStructure.orderType = order.orderType;
						traderStructure.orderUpdatedTime = time(0);
						traderStructure.currentTotalBuy = order.totalQuantity;
						a.m_traderVector.shortBuy[reqId] = traderStructure;
					}
					else if (orderState.status == "Filled") //**** This might be the second or multiple cases of setting fullyFilledSell = true. This is OK because system will responds only once. 
					{
						std::string symbol = a.m_newtxtVector[reqId];
						a.m_traderVector.shortBuy[reqId] = a.m_traderStructureDefault; printf("I am in 51 \n"); 
						a.m_traderVector.shortBuy[reqId].symbol = symbol;

						a.m_traderVector.shortSell[reqId].stopLossShort = 0;

						if (a.m_traderVector.shortSell[reqId].totalPosition == 0)
							a.m_traderVector.shortSell[reqId].algoString = "EMPTY";
						//**** No deletion: m_algoStringVector shares the critical section of traderVector. Thus it must always be within that critical section. 

					}
				}
			}
		}
		a.m_csTraderVector.LeaveCriticalSection();
	} // end of try{ a.m_csTraderVector.EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
}

void TraderEWrapperNormalShort::openOrderEnd()
{
	//**** Part III: open order does not exist, but records may exist or not. Anyway set to default is OK.
	//**** Seem hard to find another way to handle the this part of problem. The key reason is that we have to wait the all open orders are downloaded and then we can decide whether they are consistent with the records in the memory vector. 
	bool b = true;
	try{ a.m_csOpenOrderUpdateDoneShort.EnterCriticalSection(); }
	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_openOrderUpdateDoneShort = true;
		a.m_csOpenOrderUpdateDoneShort.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalShort::updateAccountValue(const IBString& key, const IBString& val, const IBString& currency, const IBString& accountName) // After m_EC->reqAccountUpdates(subscribe, m_accountCode), two EWrapper will receive: updateAccountValue and updatePortfolio
{
	int a;
	a = 1;
}

void TraderEWrapperNormalShort::updatePortfolio(const Contract& contract, int realPosition, double marketPrice, double marketValue, double averageCost, double unrealizedPNL, double realizedPNL, const IBString& accountName)
{
	if (realPosition <= 0) // It seems IB feed back all long and short positions to all clients. So I need this condition
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
				a.m_traderVector.shortSell[i_value].totalPosition = realPosition;
				a.m_traderVector.shortSell[i_value].averageCost = averageCost;
				a.m_traderVector.shortSell[i_value].positionUpdatedTime = time(0);
			}
			a.m_csTraderVector.LeaveCriticalSection();
		}
	}
}//updatePortfolio function end.

void TraderEWrapperNormalShort::accountDownloadEnd(const IBString& accountName)
{
	std::chrono::high_resolution_clock::time_point now;
	bool b = true;
	try{ a.m_csUpdatePorfolioUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_updatePorfolioUpdateDoneShort = true;
		a.m_csUpdatePorfolioUpdateDoneShort.LeaveCriticalSection();
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

void TraderEWrapperNormalShort::position(const IBString& account, const Contract& contract, int position, double avgCost)
{
	if (position <= 0) // It seems IB feed back all long and short positions to all clients. So I need this condition
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
				a.m_traderVector.shortSell[i_value].totalPosition = position;
				a.m_traderVector.shortSell[i_value].averageCost = avgCost;
				a.m_traderVector.shortSell[i_value].positionUpdatedTime = time(0);
			}
			a.m_csTraderVector.LeaveCriticalSection();
		}

	}
}

void TraderEWrapperNormalShort::positionEnd()
{
	bool b = true;
	try{ a.m_csPositionUpdateDoneShort.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_positionUpdateDoneShort = true;
		a.m_csPositionUpdateDoneShort.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalShort::accountSummary(unsigned int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency)
{
	int a;
	a = 1;
	int b;
	b = 1;
}

void TraderEWrapperNormalShort::accountSummaryEnd(unsigned int reqId)
{
	bool b = true;
	try{ a.m_csAccountSummaryDoneShort.EnterCriticalSection(); }
	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_accountSummaryDoneShort = true;
		a.m_csAccountSummaryDoneShort.LeaveCriticalSection();
	}
}

void TraderEWrapperNormalShort::cancelOrder(OrderId &orderId)
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
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	}

}

void TraderEWrapperNormalShort::cancelExistingShortSellOrder(int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.shortSell[reqId].cancelling)
	{
		if (a.m_traderVector.shortSell[reqId].orderId > 0 && a.m_traderVector.shortSell[reqId].opening == true)
		{
			cancelOrder(a.m_traderVector.shortSell[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.shortSell[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			std::cout << "At " << tempString << ":" << " A shortSell order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.shortSell[reqId].orderId << std::endl;
		}
	}
}

void TraderEWrapperNormalShort::cancelExistingShortBuyOrder(int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.shortBuy[reqId].cancelling)
	{
		if (a.m_traderVector.shortBuy[reqId].orderId > 0 && a.m_traderVector.shortBuy[reqId].closing == true)
		{
			cancelOrder(a.m_traderVector.shortBuy[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.shortBuy[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			std::cout << "At " << tempString << ":" << " A shortBuy order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.shortBuy[reqId].orderId << std::endl;
		}
	}
}