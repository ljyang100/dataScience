#include "traderNormalShort.h"

void TraderNormalShort::TraderNormalShortEntryPoint()
{
	//Sleep(10000);
	//***** Part I 
	time_t nextHeartBeatPosition = time(0);
	int counter = 0; bool done = false;
	while (1)
	{
		std::chrono::high_resolution_clock::time_point now;
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
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
		if (done == true) break; else { Sleep(100); counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	} //end of while(1)

	while (1)
	{
		bool b = true, break1 = false;
		try{ a.m_csPositionUpdateDoneShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_positionUpdateDoneShort) break1 = true; // I cannot break here because otherwise I will be always in the critical section. 
			a.m_csPositionUpdateDoneShort.LeaveCriticalSection();
		}

		if ((time(0) - nextHeartBeatPosition) >= 60 && time(0) != nextHeartBeatPosition)
		{
			printf(" I am waiting for position update.\n");
			nextHeartBeatPosition = time(0);
			//*****For position update, I cannot use non-blocking strategy to get out. Otherwise it may cause big diaster. For example, the app may keep opening repetitive positions. 
		}

		if (break1) break; else Sleep(100); ///
	}


	//***** Part II
	counter = 0; done = false;
	while (1)
	{
		std::chrono::high_resolution_clock::time_point now;
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)
			{
				m_EC->reqIds(1); //Note this reqIds has nothing to do with the reqId used all through my applications. Here the reqIs() requests an next valid orderId. Its correspondong EWrapper is nextValidId(). Also here I request only one orderId.
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
				done = true;
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}
		if (done == true) break; else { Sleep(100); counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	} //end of while(1)

	bool c = false;
	while (1) //This part is a little bit overkill.
	{
		bool b = true;
		try{ a.m_csOrderIdReadyShort.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			c = (a.m_orderIdReadyShort);
			a.m_csOrderIdReadyShort.LeaveCriticalSection();
		}
		if (!c) Sleep(100);
		else break;
	}
	//***** Once run into here, the m_orderId has already been set in TraderEWrapper.

	//***** Part III Calculated some parameters used in this class.
	bool b = true; 	//***** Note the following a.m_csVectorS critical seciton is within that of m_traderStructureVector and thus is a nested critical section with the specific entering order. There is a nested critical section with opposite entering order in setup.cpp. However, there will be no deadlock because the trader.cpp and setup.cpp do not run simultaneously.
	try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_barIndexShift = a.m_VectorS[0][0].size() - a.m_wholeDayTradeDuration / a.m_bardurationS; //**** Note that m_wholeDayTradeDuration is already in units of seconds, same as m_bardurationS.
		a.m_csVectorS.LeaveCriticalSection();
	}
	
	Sleep(2000); //**** For the position in other thread to be populated. 
	//***** Part IV. The start of trade.
	time_t nextHeartbeat1 = time(0), nextHeartbeat2 = time(0), nextHeartbeat3 = time(0), nextHeartbeat4 = time(0), nextHeartbeat5 = time(0), lastDownloadTime = time(0);

	//******* Because I may use part of IB accurate bars and part of my constructed bars to constructed even bigger bars, it might be dangerous to use IB bars because its volume is much bigger than mine and thus may overweight my bars in the big-bar construction. 
	while (1)
	{
		Sleep(2000); //*****This should be OK for normal stock long-term trading.

		if (time(0) > a.m_initime && time(0) <= a.m_tradingEndTime ) //**** When traderClientAlive = false or dataClientAlive = false, normally I don't have to do anything. In fact, I cannot do anything when client is inactive. 
		{
			if (m_openOrderCounter < 1)
			{
				//**** This branch guarantee that the trader vector will be populated by open orders at least once before trading.
				m_traderEWrapper.update_traderStructureVector_by_EWrappers();
				lastDownloadTime = time(0);
				m_openOrderCounter = m_openOrderCounter + 1;
			}
			else
			{
				if (((time(0) - a.m_initime) % a.m_traderParameters.timeBetweenReqOpenOrders == 0) && (time(0) != lastDownloadTime))
				{
					m_traderEWrapper.update_traderStructureVector_by_EWrappers();
					lastDownloadTime = time(0);
				}
			}


			m_numLoops = m_numLoops + 1;
			for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
			{
				//***** The following sentence is for testing.
				m_numSymbolsPassed = m_numSymbolsPassed + 1;

				b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					//****For testing.
					int timePassed = 600;
					if (((time(0) - a.m_initime) % timePassed == 0) && time(0) != nextHeartbeat3)
					{
						//double minutes = timePassed / 60;
						//time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						//std::cout << "At " << tempString << ":" << " total number of Short modifications is " << m_numModifications << " and within last " << minutes << " minutes the app has calculated " << m_numSymbolsPassed << " symbols. " << std::endl;

						nextHeartbeat3 = time(0);
						m_numSymbolsPassed = 0;
					}
					//****End of for testing. 

					if (a.m_traderVector.longBuy[reqId].opening || a.m_traderVector.longSell[reqId].closing || a.m_traderVector.longBuy[reqId].totalPosition > 0)
					{
						//******The purpose of this branch is to make sure that we will never simultaneously both long and short a symbol.
						cancelExistingShortBuyOrder(reqId, "There are long-related order or position.\n");
						cancelExistingShortSellOrder(reqId, "There are long-related order or position.\n");
					}
					else // shortRelated = false, meaning the specific symbol is not being shorted and is not with any short position. 
					{
						if (a.m_traderVector.shortSell[reqId].opening) //***** Note in Normal case, opening = true always means a long opening order. 
						{
							//***** Be careful, I cannot use the buySharesChanged=true as a condition to run modifyOrder(), because even shares not changed I still need run modifyOrder for the market price change. 
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if (((time(0) - a.m_traderVector.shortSell[reqId].lastModifiedTimeSell) > 5) && (a.m_traderVector.shortSell[reqId].modifiedSell == false))
									{
										a.m_traderVector.shortSell[reqId].modifiedSell = true;
										time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
										a.m_myfileShort << "At " << tempString << ":" << " entryPoint: modifiedSell is set to be true for symbol " << a.m_newtxtVector[reqId] << std::endl;
										a.m_traderVector.shortSell[reqId].lastModifiedTimeSell = time(0);
									}

									if (a.m_traderVector.shortSell[reqId].modifiedSell)
									{
										if (a.m_traderVector.shortSell[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.shortSell[reqId].noSellShort == false) //****First condition makes sure buy and sell order will never co-exist. Second condition makes sure that the order is really opened (acknowledged)
											modifySellOrder(reqId, a.m_currentDataStructureVector[reqId]);
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}

						} // if (a.m_traderVector.shortSell[reqId].opening)
						else //  a.m_traderVector.shortSell[reqId].opening = false.  **** Note in Normal case, even opening = false, it does not means closing = true. For the first half orders, opening can be either true or false, but false does not means closing = true. 
						{
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if (a.m_traderVector.shortSell[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.shortSell[reqId].noSellShort == false) //****This make sure buy and sell order will never co-exist. Note I cannot use .closing == false because closing = true will not be set when the selling order is fully filled at once, without partial fill. 
									{
										if (a.m_traderParameters.dayTrading == true)
										{
											if (time(0) <= (a.m_initime + a.m_traderParameters.timeShiftToOpenOrder))
												createSellOrder(reqId, a.m_currentDataStructureVector[reqId]);
										}
										else
											createSellOrder(reqId, a.m_currentDataStructureVector[reqId]);
									}
								}
									
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}
						} // end of  !a.m_traderVector.shortSell[reqId].opening.

						//******************************************************
						//******************************************************
						////For buying orders
						if (a.m_traderVector.shortBuy[reqId].closing)
						{
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								bool timeOK = (time(0) > a.m_traderVector.shortSell[reqId].positionUpdatedTime) && ((time(0) - a.m_traderVector.shortSell[reqId].positionUpdatedTime) < a.m_traderParameters.timeBetweenReqOpenOrders);
								if (timeOK) //***** I request position every 20 seconds.
								{
									if (abs(a.m_traderVector.shortSell[reqId].totalPosition) < (int) a.m_traderVector.shortBuy[reqId].buyingShares)
									{
										if (!a.m_traderVector.shortBuy[reqId].cancelling)
										{
											if (a.m_traderVector.shortBuy[reqId].orderId > 0)
											{
												a.m_traderVector.shortBuy[reqId].totalCancellationsSell = a.m_traderVector.shortBuy[reqId].totalCancellationsSell + 1;
												cancelExistingShortBuyOrder(reqId, " A short buy order is cancelled because totalPositoin is less than .buyingShares");
												a.m_traderVector.shortBuy[reqId].cancelling = true;
											}
										}
									}
								}

								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									//****This gurantee that any order might be scanned once within 5 seconds even if modifiedSell is wrongly set to be false for a long time elsewhere.
									if ((time(0) - a.m_traderVector.shortBuy[reqId].lastModifiedTimeBuy) > 5 && a.m_traderVector.shortBuy[reqId].modifiedBuy == false)
									{
										a.m_traderVector.shortBuy[reqId].modifiedBuy = true;
										time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
										a.m_myfileShort << "At " << tempString << ":" << " entryPoint: modifiedBuy is set to be true for symbol " << a.m_newtxtVector[reqId] << std::endl;

										a.m_traderVector.shortBuy[reqId].lastModifiedTimeBuy = time(0);
									}

									if (a.m_traderVector.shortBuy[reqId].modifiedBuy)
									{
										if (a.m_traderVector.shortBuy[reqId].manualOrder == false ) //****First condition makes sure buy and sell order will never co-exist.
										{
											if (a.m_traderVector.shortSell[reqId].averageCost > 0 && a.m_traderVector.shortSell[reqId].totalPosition < 0 && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.shortBuy[reqId].noBuyShort == false)
												modifyBuyOrder(reqId, a.m_currentDataStructureVector[reqId]);

										}
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}
						} //end of a.m_traderVector.shortBuy[reqId].closing == true
						else // a.m_traderVector.shortBuy[reqId].closing == false;
						{
							//**** For shoring case, I must be make sure there are no any long open order and long positions. 
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if (a.m_traderVector.shortSell[reqId].averageCost > 0 && a.m_traderVector.shortSell[reqId].totalPosition < 0)
									{
										if (a.m_traderVector.shortBuy[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.shortBuy[reqId].noBuyShort == false) //****This make sure buy and sell order will never co-exist.
											createBuyOrder(reqId, a.m_currentDataStructureVector[reqId]); //**** There must be shares to buy because .totalPosition > 0 is checked above. 
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}
						} // end of // !a.m_traderVector.shortBuy[reqId].closing

						//***** Note the database query above must be within the critical section. 
					} //End of not long-related. 

					a.m_csTraderVector.LeaveCriticalSection();
				} //end of trader critical section. 
			} //for (int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
		} //if time is within the trading session
		else if (time(0) < a.m_initime)
		{
			Sleep(1000);
			//**** Without the following sentences, then if we had an long-term internet disconnection, then the app will, e.g. requrest too many open orders when internect restored. 
			nextHeartbeat1 = time(0); nextHeartbeat2 = time(0); nextHeartbeat3 = time(0); 
		}
		else if (time(0) > (a.m_tradingEndTime + 90)) //Currently the m_endTime + 90 = 1:00pm + 75 seconds. 
		{
			for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
			{
				bool b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					int totalPosition = 0;
					if (a.m_traderVector.longBuy[reqId].totalPosition > 0) totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
					else if (a.m_traderVector.shortSell[reqId].totalPosition < 0) totalPosition = a.m_traderVector.shortSell[reqId].totalPosition;

					if (totalPosition == 0 && a.m_traderVector.shortSell[reqId].opening == true)
						cancelExistingShortSellOrder(reqId, "Cancel Short opening order before session close");

					a.m_csTraderVector.LeaveCriticalSection();
				}
			}
			int a = 1;
			//exit(0); //**** Note I cannot use exit(0) here. Sometimes if this exit(0) is executed earlier before the exit(0) in the long client, then it might loss data before the long client save the data. 
		}
	} //while(1) loop.
}

void TraderNormalShort::cancelExistingShortSellOrder(unsigned int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.shortSell[reqId].cancelling)
	{
		if (a.m_traderVector.shortSell[reqId].orderId > 0 && a.m_traderVector.shortSell[reqId].opening == true)
		{
			cancelOrder(a.m_traderVector.shortSell[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.shortSell[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			a.m_myfileShort << "At " << tempString << ":" << " A shortSell order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.shortSell[reqId].orderId << std::endl;
			printf("At %s, A shortSell order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

void  TraderNormalShort::cancelExistingShortBuyOrder(unsigned int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.shortBuy[reqId].cancelling)
	{
		if (a.m_traderVector.shortBuy[reqId].orderId > 0 && a.m_traderVector.shortBuy[reqId].closing == true)
		{
			cancelOrder(a.m_traderVector.shortBuy[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.shortBuy[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			a.m_myfileShort << "At " << tempString << ":" << " A shortBuy order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.shortBuy[reqId].orderId << std::endl;
			printf("At %s, A shortBuy order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

void TraderNormalShort::cancelOrder(OrderId &orderId)
{
	std::chrono::high_resolution_clock::time_point now;
	int counter = 0;
	bool done = false;
	while (1)
	{
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); }		catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				m_EC->cancelOrder(orderId);
				m_numModifications = m_numModifications + 1;

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


void TraderNormalShort::createSellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	//*****Note the i below is different from the usual i-index in m_VectorS. So later I need a different name.
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.shortSell[reqId].algoString = "whiteSwanShort";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "OPEN");

	if (a.m_traderVector.shortSell[reqId].shortHedgeOK)
	{
		if (a.m_traderVector.shortSell[reqId].basicOpen)
		{
			if (a.m_traderVector.shortSell[reqId].relevantShares > 0 && a.m_traderVector.shortSell[reqId].calculatedAsk > 2 * a.m_traderVector.shortSell[reqId].minPriceVariation)
			{
				if (a.m_traderVector.shortBuy[reqId].myBid > 0) a.m_traderVector.shortSell[reqId].calculatedAsk = max(a.m_traderVector.shortSell[reqId].calculatedAsk, (a.m_traderVector.shortBuy[reqId].myBid + 2 * a.m_traderVector.shortSell[reqId].minPriceVariation));
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.shortSell[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5)
				{
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.shortSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100;


				if (time(0) - a.m_traderVector.shortSell[reqId].lastFilledShortTime > a.m_traderParameters.timeBetweenReqOpenOrders && a.m_newtxtVector[reqId] != "SPY")
				{
					placeOrder(reqId, i, "SELL", "LMT", a.m_traderVector.shortSell[reqId].relevantShares, a.m_traderVector.shortSell[reqId].calculatedAsk);

					//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					std::cout << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short sell order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.shortSell[reqId].myAsk << ":" << a.m_traderVector.shortSell[reqId].sellingShares << ":" << a.m_traderVector.shortSell[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
						<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
						<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;

					a.m_myfileShort << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short sell order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.shortSell[reqId].myAsk << ":" << a.m_traderVector.shortSell[reqId].sellingShares << ":" << a.m_traderVector.shortSell[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
						<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
						<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;
				}
			}
		}
	}
}

void TraderNormalShort::modifySellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.shortSell[reqId].algoString = "whiteSwanShort";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "OPEN");

	if (a.m_traderVector.shortSell[reqId].manualOrder)
		cancelExistingShortSellOrder(reqId, " Short: manualOrder = true ");
	else
	{
		if (!a.m_traderVector.shortSell[reqId].shortHedgeOK)
			cancelExistingShortSellOrder(reqId, " Short: shortHedgeOK = false ");
		else
		{
			if (!a.m_traderVector.shortSell[reqId].basicOpen)
				cancelExistingShortSellOrder(reqId, " Short: shortOK = false. ");
			else
			{
				if (a.m_traderVector.shortSell[reqId].calculatedAsk < currentDataStructure.bid){
					cancelExistingShortSellOrder(reqId, " Short: calcuatedAsk is smaller than BID. ");
					if (a.m_traderVector.shortSell[reqId].relevantShares <= 0)
						cancelExistingShortSellOrder(reqId, " Short: relevantShares <= 0. ");
					else
					{
						if ((a.m_traderVector.shortSell[reqId].myAsk + a.m_traderVector.shortSell[reqId].minPriceVariation) <= a.m_traderVector.shortBuy[reqId].myBid && a.m_traderVector.shortSell[reqId].myAsk > 0 && a.m_traderVector.shortBuy[reqId].myBid > 0)
							if (!a.m_traderVector.shortSell[reqId].cancelling)
								if (a.m_traderVector.shortSell[reqId].orderId > 0) cancelExistingShortSellOrder(reqId, " Short: I need cancel the short sell order because possible cross order ");

						if (a.m_traderVector.shortBuy[reqId].myBid > 0)a.m_traderVector.shortSell[reqId].calculatedAsk = max(a.m_traderVector.shortSell[reqId].calculatedAsk, (a.m_traderVector.shortBuy[reqId].myBid + 2 * a.m_traderVector.shortSell[reqId].minPriceVariation));
						int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
						int priceLastDigit = lmtPrice100IntNew % 10;
						int minPriceVariationLastDigit = (int(a.m_traderVector.shortSell[reqId].minPriceVariation * 100)) % 10;
						if (minPriceVariationLastDigit == 5)
						{
							if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
							else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
						}
						a.m_traderVector.shortSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100;

						bool sharesChanged = false, priceChanged = false;
						if (a.m_traderVector.shortSell[reqId].sellingShares > 0){
							double shareDifferenceRatio = ((double)a.m_traderVector.shortSell[reqId].relevantShares - (double)a.m_traderVector.shortSell[reqId].sellingShares) / (double)a.m_traderVector.shortSell[reqId].sellingShares;
							if (abs(shareDifferenceRatio) > a.m_traderParameters.sharesUncertainty) sharesChanged = true;
						}
						priceChanged = a.m_traderVector.shortSell[reqId].priceChanged && (abs(a.m_traderVector.shortSell[reqId].myAsk - a.m_traderVector.shortSell[reqId].calculatedAsk) >= a.m_traderVector.shortSell[reqId].minPriceVariation);
						if (sharesChanged || priceChanged)
						{
							int oldShares = a.m_traderVector.shortSell[reqId].sellingShares;
							double oldPrice = a.m_traderVector.shortSell[reqId].myAsk;

							a.m_traderVector.shortSell[reqId].modifiedSell = false; //**** Although it is not right to give an "advanced" value (see notes elsewhere), this one is OK. Even I cannot set it true by accknowledgement, the app will set it to true after a pre-set time duration.
							a.m_traderVector.shortSell[reqId].lastModifiedTimeSell = time(0); //****  I need put one here in case there is no response. 
							a.m_traderVector.shortSell[reqId].currentTotalSell = a.m_traderVector.shortSell[reqId].relevantShares + a.m_traderVector.shortSell[reqId].filledSell;
							a.m_traderVector.shortSell[reqId].sellingShares = a.m_traderVector.shortSell[reqId].relevantShares;
							if (a.m_traderVector.shortSell[reqId].currentTotalSell > 0 && a.m_traderVector.shortSell[reqId].calculatedAsk > 2 * a.m_traderVector.shortSell[reqId].minPriceVariation)
							{
								time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf;
								modifyOrder_sub(reqId, "SELL", "LMT", a.m_traderVector.shortSell[reqId].currentTotalSell, a.m_traderVector.shortSell[reqId].calculatedAsk); //**** Here I change the price to calculatedBid even if the myBid price is not changed. Anyway, once arrived here, either price or shares need to change. CalculatedBid is better than .lmtPrice. 
								a.m_traderVector.shortSell[reqId].numModifiedSell = a.m_traderVector.shortSell[reqId].numModifiedSell + 1;

								//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
								std::cout << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Modify short sell order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.shortSell[reqId].calculatedAsk << " : " << a.m_traderVector.shortSell[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
									<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
									<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;
								a.m_myfileShort << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Modify short sell order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.shortSell[reqId].calculatedAsk << " : " << a.m_traderVector.shortSell[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
									<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
									<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;

								if (a.m_traderVector.shortSell[reqId].numModifiedSell > a.m_traderParameters.maxModificationsAllowed)
								{
									a.m_traderVector.shortSell[reqId].noSellShort = true; //**** This means I will not open a long order just after stopped out, unless I set noBuyLong to false manually during the session.
									a.m_myfileShort << tempString << " short Sell: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and nextTradingTime is changed to be very big. " << std::endl;
									std::cout << tempString << " short Sell: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and nextTradingTime is changed to be very big. " << std::endl;
								}


							}
						} // if (sharesChanged || priceChanged)
					}
				}
			}
		}
	}
}


void TraderNormalShort::createBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.shortSell[reqId].algoString = "whiteSwanShort";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "CLOSE");

	if (a.m_traderVector.shortBuy[reqId].basicClose)
	{
		if (a.m_traderVector.shortSell[reqId].myAsk > 0)	a.m_traderVector.shortBuy[reqId].calculatedBid = min(a.m_traderVector.shortBuy[reqId].calculatedBid, (a.m_traderVector.shortSell[reqId].myAsk - 2 * a.m_traderVector.shortBuy[reqId].minPriceVariation));
		int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
		int priceLastDigit = lmtPrice100IntNew % 10;
		int minPriceVariationLastDigit = (int(a.m_traderVector.shortBuy[reqId].minPriceVariation * 100)) % 10;
		if (minPriceVariationLastDigit == 5)
		{
			if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
			else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
		}
		a.m_traderVector.shortBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;

		if (a.m_traderVector.shortBuy[reqId].calculatedBid > 0)
		{
			placeOrder(reqId, i, "BUY", "LMT", a.m_traderVector.shortBuy[reqId].relevantShares, a.m_traderVector.shortBuy[reqId].calculatedBid);

			//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			std::cout << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short buy order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.shortBuy[reqId].myBid << ":" << a.m_traderVector.shortBuy[reqId].buyingShares << ":" << a.m_traderVector.shortSell[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
				<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
				<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;
			a.m_myfileShort << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short buy order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.shortBuy[reqId].myBid << ":" << a.m_traderVector.shortBuy[reqId].buyingShares << ":" << a.m_traderVector.shortSell[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
				<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
				<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;
		}
	}
}

void TraderNormalShort::modifyBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.shortSell[reqId].algoString = "whiteSwanShort";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "CLOSE");

	if (a.m_traderVector.shortBuy[reqId].manualOrder)
		cancelExistingShortBuyOrder(reqId, " Short: modifyBuyOrder, manualOrder = true ");
	else
	{
		if (a.m_traderVector.shortBuy[reqId].relevantShares <= 0)
			cancelExistingShortBuyOrder(reqId, " Short: modifyBuyOrder, m_algoBuy.relevantShares <= 0 ");
		else //m_algoBuy.relevantShares > 0
		{
			if (a.m_traderVector.shortBuy[reqId].basicClose == false)
				cancelExistingShortBuyOrder(reqId, " Short: modifyBuyOrder, a.m_traderVector.shortBuy[reqId].basicClose == false ");
			else
			{
				if ((a.m_traderVector.shortSell[reqId].myAsk + a.m_traderVector.shortBuy[reqId].minPriceVariation) <= a.m_traderVector.shortBuy[reqId].myBid && a.m_traderVector.shortSell[reqId].myAsk > 0 && a.m_traderVector.shortBuy[reqId].myBid > 0)
					if (!a.m_traderVector.shortBuy[reqId].cancelling)
						if (a.m_traderVector.shortBuy[reqId].orderId > 0) cancelExistingShortBuyOrder(reqId, " Short: I need cancel the short buy order because possible cross order ");

				if (a.m_traderVector.shortSell[reqId].myAsk > 0)	a.m_traderVector.shortBuy[reqId].calculatedBid = min(a.m_traderVector.shortBuy[reqId].calculatedBid, (a.m_traderVector.shortSell[reqId].myAsk - 2 * a.m_traderVector.shortBuy[reqId].minPriceVariation));
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.shortBuy[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5)
				{
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.shortBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;

				bool sharesChanged = false, priceChanged = false; //**** This part should be checked independently from the price checking part. This is because we will need to modify order even price not changed. 
				if (a.m_traderVector.shortBuy[reqId].buyingShares > 0)
				{
					double shareDifferenceRatio = ((double)a.m_traderVector.shortBuy[reqId].relevantShares - (double)a.m_traderVector.shortBuy[reqId].buyingShares) / (double)a.m_traderVector.shortBuy[reqId].buyingShares;
					if (abs(shareDifferenceRatio) > a.m_traderParameters.sharesUncertainty) sharesChanged = true;
				}
				priceChanged = a.m_traderVector.shortBuy[reqId].priceChanged && (abs(a.m_traderVector.shortBuy[reqId].myBid - a.m_traderVector.shortBuy[reqId].calculatedBid) >= a.m_traderVector.shortBuy[reqId].minPriceVariation);
				if (sharesChanged || priceChanged)
				{
					if (a.m_traderVector.shortBuy[reqId].calculatedBid > 0)
					{
						int oldShares = a.m_traderVector.shortBuy[reqId].buyingShares;
						double oldPrice = a.m_traderVector.shortBuy[reqId].myBid;

						a.m_traderVector.shortBuy[reqId].modifiedBuy = false; //This is fine although it is an "advanced" value, see notes in modifyBuy().
						a.m_traderVector.shortBuy[reqId].lastModifiedTimeBuy = time(0);
						a.m_traderVector.shortBuy[reqId].currentTotalBuy = a.m_traderVector.shortBuy[reqId].relevantShares + a.m_traderVector.shortBuy[reqId].filledBuy;
						a.m_traderVector.shortBuy[reqId].buyingShares = a.m_traderVector.shortBuy[reqId].relevantShares;

						if (a.m_traderVector.shortBuy[reqId].currentTotalBuy > 0 && a.m_traderVector.shortBuy[reqId].calculatedBid > 0)
						{
							time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
							modifyOrder_sub(reqId, "BUY", "LMT", a.m_traderVector.shortBuy[reqId].currentTotalBuy, a.m_traderVector.shortBuy[reqId].calculatedBid);

							//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
							std::cout << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Modify short buy order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.shortBuy[reqId].calculatedBid << " : " << a.m_traderVector.shortBuy[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
								<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
								<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;
							a.m_myfileShort << tempString << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Modify short buy order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.shortBuy[reqId].calculatedBid << " : " << a.m_traderVector.shortBuy[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
								<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
								<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;

							if (a.m_traderVector.shortBuy[reqId].numModifiedBuy > a.m_traderParameters.maxModificationsAllowed)
							{
								a.m_myfileShort << tempString << " short Buy: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and the order will be closed on ASK. " << std::endl;
								std::cout << tempString << " short Buy: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and the order will be closed on ASK. " << std::endl;
							}

						}
					}
				}
			}
		} // (a.m_traderVector.shortBuy[reqId].relevantShares > 0)
	}
}

void TraderNormalShort::placeOrder(unsigned int &reqId, unsigned int &i, std::string action, std::string orderType, unsigned int &relevantShares, double &lmtPrice)
{
	if (i >= 0 && i < a.m_baseNum.size() && reqId >= 0 && reqId < a.m_newtxtVector.size())
	{
		Contract C_d;
		Order order_d;

		C_d.symbol = a.m_newtxtVector[reqId];
		C_d.secType = "STK";
		C_d.currency = "USD";
		C_d.exchange = "SMART"; //If necessary, I may include "exchange" in the newtxtVector in the future.
		order_d.clientId = a.m_clientIdShort;
		order_d.action = action;
		order_d.totalQuantity = relevantShares;
		order_d.orderType = orderType;
		order_d.tif = "GTC";
		order_d.transmit = true;
		order_d.overridePercentageConstraints = true;
		order_d.allOrNone = false;
		order_d.lmtPrice = lmtPrice;

		std::chrono::high_resolution_clock::time_point now;
		int counter = 0;
		bool done = false;
		while (1) //*******Note this condition is specially for Normal trading
		{
			bool b = true;
			try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
				{
					bool b = true;
					try{ a.m_csOrderIdReadyShort.EnterCriticalSection(); }
					catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
					{
						order_d.orderId = a.m_orderIdShort;
						m_EC->placeOrder(a.m_orderIdShort, C_d, order_d);

						m_numModifications = m_numModifications + 1;

						if (action == "SELL")
						{
							a.m_traderVector.shortSell[reqId].opening = true;
							a.m_traderVector.shortSell[reqId].orderType = order_d.orderType;
							a.m_traderVector.shortSell[reqId].symbol = C_d.symbol;
							a.m_traderVector.shortSell[reqId].orderId = order_d.orderId;
							a.m_traderVector.shortSell[reqId].action = action;
							a.m_traderVector.shortSell[reqId].myAsk = order_d.lmtPrice;
							a.m_traderVector.shortSell[reqId].currentTotalSell = order_d.totalQuantity;
							a.m_traderVector.shortSell[reqId].sellingShares = order_d.totalQuantity;
						}
						if (action == "BUY")
						{
							a.m_traderVector.shortBuy[reqId].closing = true;
							a.m_traderVector.shortBuy[reqId].orderType = order_d.orderType;
							a.m_traderVector.shortBuy[reqId].symbol = C_d.symbol;
							a.m_traderVector.shortBuy[reqId].orderId = order_d.orderId;
							a.m_traderVector.shortBuy[reqId].action = action;
							a.m_traderVector.shortBuy[reqId].myBid = lmtPrice;
							a.m_traderVector.shortBuy[reqId].currentTotalBuy = relevantShares;
							a.m_traderVector.shortBuy[reqId].buyingShares = relevantShares;
						}

						a.m_orderIdShort = a.m_orderIdShort + 1;
						done = true;
						a.m_csOrderIdReadyShort.LeaveCriticalSection();
					}

					now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
					a.m_timeDeque.push_back(now);
				}

				a.m_csTimeDeque.LeaveCriticalSection();
			}

			if (done == true) break; else { Sleep(100); counter = counter + 1; }
			if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
		} //end of while(1)
	}
	else
	{
		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		a.m_myfileShort << "At " << tempString << ":" << " vector out of range for reqId : i = " << reqId << " : " << i << " at file " << __FILE__ << " and line " << __LINE__ << std::endl;
	}
}

void TraderNormalShort::modifyOrder_sub(unsigned int &reqId, std::string action, std::string orderType, unsigned int &totalQuantity, double &lmtPrice)
{
	Contract C_d;
	Order order_d;

	C_d.symbol = a.m_newtxtVector[reqId];
	C_d.secType = "STK";
	C_d.currency = "USD";
	C_d.exchange = "SMART"; //If necessary, I may include "exchange" in the newtxtVector in the future.
	order_d.clientId = a.m_clientIdShort;

	order_d.action = action;
	order_d.totalQuantity = totalQuantity;
	order_d.orderType = orderType;
	order_d.tif = "GTC";
	order_d.transmit = true;
	order_d.overridePercentageConstraints = true;
	order_d.allOrNone = false;
	order_d.lmtPrice = lmtPrice;

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
				if (action == "SELL")
				{
					a.m_traderVector.shortSell[reqId].myAsk = order_d.lmtPrice;
					order_d.orderId = a.m_traderVector.shortSell[reqId].orderId;
				}
				if (action == "BUY")
				{
					a.m_traderVector.shortBuy[reqId].myBid = order_d.lmtPrice;
					order_d.orderId = a.m_traderVector.shortBuy[reqId].orderId;
				}

				m_EC->placeOrder(order_d.orderId, C_d, order_d);

				m_numModifications = m_numModifications + 1;
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
				//The following if-structure is redundant and is used for absolutely correct. 

				done = true;
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}

		if (done == true) break; else { Sleep(100);	counter = counter + 1; }
		if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	}
}

