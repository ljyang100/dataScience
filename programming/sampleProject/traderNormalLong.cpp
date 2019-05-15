#include "traderNormalLong.h"

void TraderNormalLong::TraderNormalLongEntryPoint()
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
		if (counter >a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	} //end of while(1)

	while (1)
	{
		bool b = true, break1 = false;
		try{ a.m_csPositionUpdateDoneLong.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_positionUpdateDoneLong) break1 = true; // I cannot break here because otherwise I will be always in the critical section. 
			a.m_csPositionUpdateDoneLong.LeaveCriticalSection();
		}

		if ((time(0) - nextHeartBeatPosition) >= 60 && time(0) != nextHeartBeatPosition)
		{
			printf(" I am waiting for position update for long client.\n");
			nextHeartBeatPosition = time(0);
			//*****For position update, I cannot use non-blocking strategy to get out. Otherwise it may cause big diaster. For example, the app may keep opening repetitive positions. 
		} 

		if (break1) break; else Sleep(500); ///
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
		if (counter >a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	} //end of while(1)

	bool c = false;
	while (1) //This part is a little bit overkill.
	{
		bool b = true;
		try{ a.m_csOrderIdReadyLong.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			c = a.m_orderIdReadyLong;
			a.m_csOrderIdReadyLong.LeaveCriticalSection();
		}
		if (!c) Sleep(100);
		else break;
	}
	//***** Once run into here, the m_orderId has already been set in TraderEWrapper.

	//***** Part III Calculated some parameters used in this class.
	bool b = true; 	//***** Note the following m_csVectorS critical seciton is within that of m_traderStructureVector and thus is a nested critical section with the specific entering order. There is a nested critical section with opposite entering order in setup.cpp. However, there will be no deadlock because the trader.cpp and setup.cpp do not run simultaneously.
	try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		a.m_barIndexShift = a.m_VectorS[0][0].size() - a.m_wholeDayTradeDuration / a.m_bardurationS; //**** Note that m_wholeDayTradeDuration is already in units of seconds, same as m_bardurationS.
		a.m_csVectorS.LeaveCriticalSection();
	}

	Sleep(5000); //**** For the position in other thread to be populated. 
	//***** Part IV. The start of trade.
	time_t nextHeartbeat1 = time(0), nextHeartbeat2 = time(0), nextHeartbeat3 = time(0), nextHeartbeat4 = time(0), nextHeartbeat5 = time(0), lastDownloadTime = time(0), lastFileSavingTime = time(0);
	time_t heartBeat1 = time(0);
	//******* Because I may use part of IB accurate bars and part of my constructed bars to constructed even bigger bars, it might be dangerous to use IB bars because its volume is much bigger than mine and thus may overweight my bars in the big-bar construction. 
	time_t lastMarketDataRequestTime = time(0);
	while (1)
	{
		Sleep(2000); //*****This should be OK for normal stock long-term trading.

		if (time(0) > a.m_initime && time(0) <= a.m_tradingEndTime) //**** When traderClientAlive = false or dataClientAlive = false, normally I don't have to do anything. In fact, I cannot do anything when client is inactive. 
		{

			if (((time(0) - nextHeartbeat2) >= 20) && (time(0) != nextHeartbeat2)) //**** This might not be run if the loop below takes long time.
			{
				cleaning();
				manualReset(); //***** Note this is with critical sections, see details in the function.
				a.unhandledSymbols();
				nextHeartbeat2 = time(0);
			}

			if (m_openOrderCounter < 1)
			{
				//**** This branch guarantee that the trader vector will be populated by open orders at least once before trading.
				m_traderEWrapper.update_traderStructureVector_by_EWrappers(); //**** this function is with a lot of critical sections. So don't let it in a nested critical sections here. 
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
				if (((time(0) - heartBeat1) >= 60) && (time(0) != heartBeat1))
				{
					bool b = true;
					try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
					{
						time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						a.m_myfileLong << tempString << ":" << " hedge related current exposure is " << (a.m_traderVector.longBuy[0].totalLongValue + a.m_traderVector.shortSell[0].totalShortValue) << std::endl;
						printf("hedge related current exposure is %f. \n", a.m_traderVector.longBuy[0].totalLongValue + a.m_traderVector.shortSell[0].totalShortValue);

						remove(a.m_manuals_OLD.c_str());
						rename(a.m_manuals_NEW.c_str(), a.m_manuals_OLD.c_str()); //**** rename new.txt to old.txt, but not the opposite.
						std::ofstream o_new(a.m_manuals_NEW.c_str(), std::ios::app);
						for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
						{
							if (a.m_traderVector.longBuy[reqId].totalPosition < 0)
							{
								a.m_myfileLong << tempString << " A weird negative position " << a.m_traderVector.longBuy[reqId].totalPosition << " occurs for symbol " << a.m_newtxtVector[reqId] << std::endl;
								printf("%s: A weird negative position %d occurs in long client for symbol %s.\n", tempString.c_str(), a.m_traderVector.longBuy[reqId].totalPosition, a.m_newtxtVector[reqId].c_str());
							}
							if (a.m_traderVector.shortSell[reqId].totalPosition > 0)
							{
								a.m_myfileLong << tempString << " A weird positive " << a.m_traderVector.shortSell[reqId].totalPosition << " occurs for symbol " << a.m_newtxtVector[reqId] << std::endl;
								printf("%s: A weird positive position %d occurs in Short client for symbol %s.\n", tempString.c_str(), a.m_traderVector.shortSell[reqId].totalPosition, a.m_newtxtVector[reqId].c_str());
							}

							int totalPosition = 0;
							if (a.m_traderVector.longBuy[reqId].totalPosition > 0) totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
							else if (a.m_traderVector.shortSell[reqId].totalPosition < 0) totalPosition = a.m_traderVector.shortSell[reqId].totalPosition; // feeding back poision should be in the same client?
							if (totalPosition != 0)
							{
								o_new << a.m_newtxtVector[reqId] << ',' << a.m_traderVector.longBuy[reqId].stopLossLong << ',' << a.m_traderVector.shortSell[reqId].stopLossShort << ',' << a.m_traderVector.longBuy[reqId].algoString << ',' << a.m_traderVector.shortSell[reqId].algoString << ','
									<< totalPosition << std::endl;
							}
						}
						o_new.close();
						//time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						//a.m_myfileLong << "At " << tempString << ":" << " manuals successfully written to a new file " << m_manuals_NEW << std::endl;
						a.m_csTraderVector.LeaveCriticalSection();
					}
					heartBeat1 = time(0);
				}

				//***** The following sentence is for testing.
				m_numSymbolsPassed = m_numSymbolsPassed + 1;

				b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					//****For testing. Do not delete, I need know the code is running. Similar to the market data. 
					int timePassed = 120;
					if ((time(0) - nextHeartbeat3) > timePassed && time(0) != nextHeartbeat3)
					{
						double minutes = timePassed / 60;
						time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						a.m_myfileLong << "At " << tempString << ":" << " total number of Long modifications is " << m_numModifications << " and within last " << minutes << " minutes the app has calculated " << m_numSymbolsPassed << " symbols. " << std::endl;

						nextHeartbeat3 = time(0);
						m_numSymbolsPassed = 0;
					}
					//****End of for testing. 

					if (a.m_traderVector.shortSell[reqId].opening || a.m_traderVector.shortBuy[reqId].closing || a.m_traderVector.shortSell[reqId].totalPosition < 0)
					{
						//******The purpose of this branch is to make sure that we will never simultaneously both long and short a symbol.
						cancelExistingLongBuyOrder(reqId, "There are short-related order or position.\n");
						cancelExistingLongSellOrder(reqId, "There are short-related order or position.\n");
					}
					else // shortRelated = false, meaning the specific symbol is not being shorted and is not with any short position. 
					{
						if (a.m_traderVector.longBuy[reqId].opening) //***** Note in Normal case, opening = true always means a long opening order. 
						{
							//***** Be careful, I cannot use the buySharesChanged=true as a condition to run modifyOrder(), because even shares not changed I still need run modifyOrder for the market price change. 
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0 
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if (((time(0) - a.m_traderVector.longBuy[reqId].lastModifiedTimeBuy) > 5) && (a.m_traderVector.longBuy[reqId].modifiedBuy == false))
									{
										a.m_traderVector.longBuy[reqId].modifiedBuy = true;
										time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
										a.m_myfileLong << "At " << tempString << ":" << " entryPoint: modifiedBuy is set to be true for symbol " << a.m_newtxtVector[reqId] << std::endl;
										printf("At %s, modifiedBuy is set to be true for symbol %s.\n", tempString.c_str(), a.m_newtxtVector[reqId].c_str());
										a.m_traderVector.longBuy[reqId].lastModifiedTimeBuy = time(0);
									}

									if (a.m_traderVector.longBuy[reqId].modifiedBuy)
									{
										if (a.m_traderVector.longBuy[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.longBuy[reqId].noBuyLong == false)
											modifyBuyOrder(reqId, a.m_currentDataStructureVector[reqId]);
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}

						} // if (a.m_traderVector.longBuy[reqId].opening)
						else //  a.m_traderVector.longBuy[reqId].opening = false.  **** Note in Normal case, even opening = false, it does not means closing = true. For the first half orders, opening can be either true or false, but false does not means closing = true. 
						{
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if ( a.m_traderVector.longBuy[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY" && a.m_traderVector.longBuy[reqId].noBuyLong == false) //****This make sure buy and sell order will never co-exist. Note I cannot use .closing == false because closing = true will not be set when the selling order is fully filled at once, without partial fill. 
									{
										if (a.m_traderParameters.dayTrading == true)
										{
											if (time(0) <= (a.m_initime + a.m_traderParameters.timeShiftToOpenOrder))
												createBuyOrder(reqId, a.m_currentDataStructureVector[reqId]);
										}
										else
											createBuyOrder(reqId, a.m_currentDataStructureVector[reqId]);
									}
								}

								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}
						} // end of  !a.m_traderVector.longBuy[reqId].opening.

						//******************************************************
						//******************************************************

						////For selling orders
						if (a.m_traderVector.longSell[reqId].closing)
						{
							bool timeOK = (time(0) > a.m_traderVector.longBuy[reqId].positionUpdatedTime) && ((time(0) - a.m_traderVector.longBuy[reqId].positionUpdatedTime) < a.m_traderParameters.timeBetweenReqOpenOrders);
							if (timeOK) //***** I request position every 20 seconds.
							{
								if (a.m_traderVector.longBuy[reqId].totalPosition < (int) a.m_traderVector.longSell[reqId].sellingShares)
								{
									if (!a.m_traderVector.longSell[reqId].cancelling)
									{
										if (a.m_traderVector.longSell[reqId].orderId > 0)
										{
											a.m_traderVector.longSell[reqId].totalCancellationsSell = a.m_traderVector.longSell[reqId].totalCancellationsSell + 1;
											cancelExistingLongSellOrder(reqId, " A long sell order is cancelled because totalPositoin is less than .sellingShares");
											a.m_traderVector.longSell[reqId].cancelling = true;
										}
									}
								}
							}

							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									//****This gurantee that any order might be scanned once within 5 seconds even if modifiedSell is wrongly set to be false for a long time elsewhere.
									if ((time(0) - a.m_traderVector.longSell[reqId].lastModifiedTimeSell) > 5 && a.m_traderVector.longSell[reqId].modifiedSell == false)
									{
										a.m_traderVector.longSell[reqId].modifiedSell = true;
										time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
										a.m_myfileLong << "At " << tempString << ":" << " entryPoint: modifiedSell is set to be true for symbol " << a.m_newtxtVector[reqId] << std::endl;
										printf("At %s, modifiedSell is set to be true for symbol %s.\n", tempString.c_str(), a.m_newtxtVector[reqId].c_str());

										a.m_traderVector.longSell[reqId].lastModifiedTimeSell = time(0);
									}

									if (a.m_traderVector.longSell[reqId].modifiedSell)
									{
										if (a.m_traderVector.longSell[reqId].manualOrder == false ) //****First condition makes sure buy and sell order will never co-exist.
										{
											if (a.m_traderVector.longBuy[reqId].averageCost > 0 && a.m_traderVector.longBuy[reqId].totalPosition > 0 && a.m_newtxtVector[reqId] != "SPY")
												modifySellOrder(reqId, a.m_currentDataStructureVector[reqId]);
										}
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}

						} //end of a.m_traderVector.longSell[reqId].closing
						else // 
						{
							//**** For shoring case, I must be make sure there are no any long open order and long positions. 
							bool b = true; //***** Note here I used nested critical sections: csTraderSTructureVector first and csCurrentDataStructureVector second. I need always keep this ordering. Also note, it is not that nested critical sections always bad. 
							try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
							{
								if (a.m_currentDataStructureVector[reqId].bid > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].last > 0
									&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0)
								{
									if (a.m_traderVector.longBuy[reqId].averageCost > 0 && a.m_traderVector.longBuy[reqId].totalPosition > 0)
									{
										if (a.m_traderVector.longSell[reqId].manualOrder == false && a.m_newtxtVector[reqId] != "SPY") //****This make sure buy and sell order will never co-exist.
											createSellOrder(reqId, a.m_currentDataStructureVector[reqId]); //**** There must be shares to sell because .totalPosition > 0 is checked above. 
									}
								}
								a.m_csCurrentDataStructureVector.LeaveCriticalSection();
							}
						} // end of // !a.m_traderVector.longSell[reqId].closing

						//***** Note the database query above must be within the critical section. 
					} //End of not short-related. 

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
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			if (remove(a.m_manuals_OLD.c_str()) != 0)
			{
				printf("Error deleting file %s.\n", a.m_manuals_OLD.c_str());
				a.m_myfileLong << "At " << tempString << ":" << " Error deleting file " << a.m_manuals_OLD << std::endl;
			}
			else
			{
				printf("File %s successfully deleted", a.m_manuals_OLD.c_str());
				a.m_myfileLong << "At " << tempString << ":" << " File " << a.m_manuals_OLD << " successfully deleted " << std::endl;
			}


			int file_result;
			file_result = rename(a.m_manuals_NEW.c_str(), a.m_manuals_OLD.c_str()); //**** rename new.txt to old.txt, but not the opposite.
			if (file_result == 0)
			{
				printf("File name successfully changed from %s to %s.\n", a.m_manuals_NEW.c_str(), a.m_manuals_OLD.c_str());
				a.m_myfileLong << "At " << tempString << ":" << " File name sucessfully changed from " << a.m_manuals_NEW << " to " << a.m_manuals_OLD << std::endl;
			}
			else
			{
				printf("Error occured when changing file name from %s to %s.\n", a.m_manuals_NEW.c_str(), a.m_manuals_OLD.c_str());
				a.m_myfileLong << "At " << tempString << ":" << " Error occured when changing file name from " << a.m_manuals_NEW << " to " << a.m_manuals_OLD << std::endl;
			}

			std::ofstream o_new(a.m_manuals_NEW.c_str(), std::ios::app);

			for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
			{
				bool b = true;
				try{ a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					int totalPosition = 0;
					if (a.m_traderVector.longBuy[reqId].totalPosition > 0) totalPosition = a.m_traderVector.longBuy[reqId].totalPosition;
					else if (a.m_traderVector.shortSell[reqId].totalPosition < 0) totalPosition = a.m_traderVector.shortSell[reqId].totalPosition;

					if (totalPosition == 0 && a.m_traderVector.longBuy[reqId].opening == true) cancelExistingLongBuyOrder(reqId, "Cancel Long opening order before session close");

					if (totalPosition != 0)
					{
						o_new << a.m_newtxtVector[reqId] << ',' << a.m_traderVector.longBuy[reqId].stopLossLong << ',' << a.m_traderVector.shortSell[reqId].stopLossShort << ',' << a.m_traderVector.longBuy[reqId].algoString << ',' << a.m_traderVector.shortSell[reqId].algoString << ','
							<< totalPosition << std::endl;

					}
					a.m_csTraderVector.LeaveCriticalSection();
				}
			}
			o_new.close();

			a.m_myfileLong << "At " << tempString << ":" << " manuals successfully written to a new file " << a.m_manuals_NEW << std::endl;
			Sleep(25000); //**** Wait the short client to cancel the opening order.
			exit(0);//***** Note I cannot have a exit(0) in the same place of short client. See the comments there. //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
		}
	} //while(1) loop.
}


void TraderNormalLong::cancelExistingLongBuyOrder(unsigned int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.longBuy[reqId].cancelling)
	{
		if (a.m_traderVector.longBuy[reqId].orderId > 0 && a.m_traderVector.longBuy[reqId].opening == true)
		{
			cancelOrder(a.m_traderVector.longBuy[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.longBuy[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			a.m_myfileLong << "At " << tempString << ":" << " A longBuy order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.longBuy[reqId].orderId << std::endl;
			printf("At %s, A longBuy order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

void  TraderNormalLong::cancelExistingLongSellOrder(unsigned int &reqId, std::string reasonToCancel)
{
	if (!a.m_traderVector.longSell[reqId].cancelling)
	{
		if (a.m_traderVector.longSell[reqId].orderId > 0 && a.m_traderVector.longSell[reqId].closing == true)
		{
			cancelOrder(a.m_traderVector.longSell[reqId].orderId); //**** once arrived here, we already know that book kepts .orderId is not same as the incoming orderId. Otherwise it will be handled in the branch before. 
			a.m_traderVector.longSell[reqId].cancelling = true;
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			a.m_myfileLong << "At " << tempString << ":" << " A longSell order is being cancelled because: " << reasonToCancel << " for symbol " << a.m_newtxtVector[reqId] << " with orderId " << a.m_traderVector.longSell[reqId].orderId << std::endl;
			printf("At %s, A longSell order is being cancelled because: %s for symbol %s.\n", tempString.c_str(), reasonToCancel.c_str(), a.m_newtxtVector[reqId].c_str());
		}
	}
}

void TraderNormalLong::cancelOrder(OrderId &orderId)
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
		if (counter >a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
	}
}

void TraderNormalLong::createBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;
	
	a.m_traderVector.longBuy[reqId].algoString = "whiteSwanLong";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "OPEN");

	if (a.m_traderVector.longBuy[reqId].longHedgeOK){
		if (a.m_traderVector.longBuy[reqId].basicOpen){
			if (a.m_traderVector.longBuy[reqId].relevantShares > 0 && a.m_traderVector.longBuy[reqId].calculatedBid > 0){
				if (a.m_traderVector.longSell[reqId].myAsk > 0)	a.m_traderVector.longBuy[reqId].calculatedBid = min(a.m_traderVector.longBuy[reqId].calculatedBid, (a.m_traderVector.longSell[reqId].myAsk - 2 * a.m_traderVector.longBuy[reqId].minPriceVariation));
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.longBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.longBuy[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5){
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.longBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;

				if (time(0) - a.m_traderVector.longBuy[reqId].lastFilledLongTime > a.m_traderParameters.timeBetweenReqOpenOrders){
					placeOrder(reqId, i, "BUY", "LMT", a.m_traderVector.longBuy[reqId].relevantShares, a.m_traderVector.longBuy[reqId].calculatedBid);
								
					//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
					time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
					a.m_myfileLong << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create buy order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.longBuy[reqId].myBid << ":" << a.m_traderVector.longBuy[reqId].buyingShares << ":" << a.m_traderVector.longBuy[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
						<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
						<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;

					std::cout << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create buy order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.longBuy[reqId].myBid << ":" << a.m_traderVector.longBuy[reqId].buyingShares << ":" << a.m_traderVector.longBuy[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
						<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
						<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;

				}
			}
		}
	}
}

void TraderNormalLong::modifyBuyOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;
	
	a.m_traderVector.longBuy[reqId].algoString = "whiteSwanLong";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "OPEN");
	
	if (a.m_traderVector.longBuy[reqId].manualOrder)
		cancelExistingLongBuyOrder(reqId, " Long: order is cancelled because manualOrder = true. ");
	else{
		if (!a.m_traderVector.longBuy[reqId].longHedgeOK)
			cancelExistingLongBuyOrder(reqId, " Long: order is cancelled because longHedgeOK = false. ");
		else
		{
			if (!a.m_traderVector.longBuy[reqId].basicOpen)
				cancelExistingLongBuyOrder(reqId, " Long: order is cancelled because basicOpen = false. ");
			else
			{
				if (a.m_traderVector.longBuy[reqId].calculatedBid > currentDataStructure.ask){
					cancelExistingLongBuyOrder(reqId, " Long: order is cancelled because myBid is even bigger than ASK. ");
					if (a.m_traderVector.longBuy[reqId].relevantShares <= 0)
						cancelExistingLongBuyOrder(reqId, " Long: order is cancelled because relevantShares <= 0. ");
					else
					{
						if ((a.m_traderVector.longSell[reqId].myAsk + a.m_traderVector.longBuy[reqId].minPriceVariation) <= a.m_traderVector.longBuy[reqId].myBid && a.m_traderVector.longSell[reqId].myAsk > 0 && a.m_traderVector.longBuy[reqId].myBid > 0)
							if (!a.m_traderVector.longBuy[reqId].cancelling)
								if (a.m_traderVector.longBuy[reqId].orderId > 0) cancelExistingLongBuyOrder(reqId, " Long: I need cancel the long buy order because possible cross order ");

						if (a.m_traderVector.longSell[reqId].myAsk > 0)	a.m_traderVector.longBuy[reqId].calculatedBid = min(a.m_traderVector.longBuy[reqId].calculatedBid, (a.m_traderVector.longSell[reqId].myAsk - 2 * a.m_traderVector.longBuy[reqId].minPriceVariation));
						int lmtPrice100IntNew = int(ceil(a.m_traderVector.longBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
						int priceLastDigit = lmtPrice100IntNew % 10;
						int minPriceVariationLastDigit = (int(a.m_traderVector.longBuy[reqId].minPriceVariation * 100)) % 10;
						if (minPriceVariationLastDigit == 5)
						{
							if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
							else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
						}
						a.m_traderVector.longBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;

						bool sharesChanged = false, priceChanged = false;
						if (a.m_traderVector.longBuy[reqId].buyingShares > 0)
						{
							double shareDifferenceRatio = ((double)a.m_traderVector.longBuy[reqId].relevantShares - (double)a.m_traderVector.longBuy[reqId].buyingShares) / (double)a.m_traderVector.longBuy[reqId].buyingShares;
							if (abs(shareDifferenceRatio) > a.m_traderParameters.sharesUncertainty) sharesChanged = true;
						}
						priceChanged = a.m_traderVector.longBuy[reqId].priceChanged && (abs(a.m_traderVector.longBuy[reqId].myBid - a.m_traderVector.longBuy[reqId].calculatedBid) >= a.m_traderVector.longBuy[reqId].minPriceVariation);
						if (sharesChanged || priceChanged)
						{
							int oldShares = a.m_traderVector.longBuy[reqId].buyingShares;
							double oldPrice = a.m_traderVector.longBuy[reqId].myBid;

							a.m_traderVector.longBuy[reqId].modifiedBuy = false; //**** Although it is not right to give an "advanced" value (see notes elsewhere), this one is OK. Even I cannot set it true by accknowledgement, the app will set it to true after a pre-set time duration.
							a.m_traderVector.longBuy[reqId].lastModifiedTimeBuy = time(0); //****  I need put one here in case there is no response. 
							a.m_traderVector.longBuy[reqId].currentTotalBuy = a.m_traderVector.longBuy[reqId].relevantShares + a.m_traderVector.longBuy[reqId].filledBuy;
							a.m_traderVector.longBuy[reqId].buyingShares = a.m_traderVector.longBuy[reqId].relevantShares;
							if (a.m_traderVector.longBuy[reqId].currentTotalBuy > 0 && a.m_traderVector.longBuy[reqId].calculatedBid > 0)
							{
								modifyOrder_sub(reqId, "BUY", "LMT", a.m_traderVector.longBuy[reqId].currentTotalBuy, a.m_traderVector.longBuy[reqId].calculatedBid); //**** Here I change the price to calculatedBid even if the myBid price is not changed. Anyway, once arrived here, either price or shares need to change. CalculatedBid is better than .lmtPrice. 
								a.m_traderVector.longBuy[reqId].numModifiedBuy = a.m_traderVector.longBuy[reqId].numModifiedBuy + 1;

								//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
								time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
								std::cout << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Modify buy order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.longBuy[reqId].calculatedBid << " : " << a.m_traderVector.longBuy[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
									<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
									<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;

								a.m_myfileLong << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Modify buy order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.longBuy[reqId].calculatedBid << " : " << a.m_traderVector.longBuy[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
									<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
									<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;

								if (a.m_traderVector.longBuy[reqId].numModifiedBuy > a.m_traderParameters.maxModificationsAllowed)
								{
									a.m_traderVector.longBuy[reqId].noBuyLong = true; //**** This means I will not open a long order just after stopped out, unless I set noBuyLong to false manually during the session.
									a.m_myfileLong << tempString << " Long Buy: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and noBuyLong is set to false. " << std::endl;
									std::cout << tempString << " Long Buy: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and noBuyLong is set to false. " << std::endl;
								}
							}
						} // if (sharesChanged || priceChanged)
					}
				}
			}
		}
	}
}

void TraderNormalLong::createSellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.longBuy[reqId].algoString = "whiteSwanLong";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "CLOSE");
	if (a.m_traderVector.longSell[reqId].basicClose)
	{   
		if (a.m_traderVector.longBuy[reqId].myBid > 0) a.m_traderVector.longSell[reqId].calculatedAsk = max(a.m_traderVector.longSell[reqId].calculatedAsk, (a.m_traderVector.longBuy[reqId].myBid + 2 * a.m_traderVector.longSell[reqId].minPriceVariation));
		int lmtPrice100IntNew = int(ceil(a.m_traderVector.longSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
		int priceLastDigit = lmtPrice100IntNew % 10;
		int minPriceVariationLastDigit = (int(a.m_traderVector.longSell[reqId].minPriceVariation * 100)) % 10;
		if (minPriceVariationLastDigit == 5)
		{
			if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
			else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
		}
		a.m_traderVector.longSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100; //**** This include the case of closeLongMid. 

		//**** Once the final version of the following is fixed, apply it in the modifySellOrder, and also in the counterpart places in short client.
		if (a.m_traderParameters.dayTrading == true)
		{
			if (time(0) >= (a.m_initime + a.m_traderParameters.timeShiftToCloseOrder))
			{
				if (currentDataStructure.ask - currentDataStructure.bid < a.m_traderVector.longSell[reqId].minPriceVariation * 1.5)
					a.m_traderVector.longSell[reqId].calculatedAsk = currentDataStructure.ask;
				else
					a.m_traderVector.longSell[reqId].calculatedAsk = (currentDataStructure.bid + currentDataStructure.ask) / 2;
			}
		}

		if (a.m_traderVector.longSell[reqId].calculatedAsk > 2 * a.m_traderVector.longSell[reqId].minPriceVariation) //**** Even .calculatedAsk is zero, the adjustment due to minPriceVariation may cause it to be a small positive number. Thus I require it to be bigger than 2*minPriceVariation.
		{
			placeOrder(reqId, i, "SELL", "LMT", a.m_traderVector.longSell[reqId].relevantShares, a.m_traderVector.longSell[reqId].calculatedAsk); //**** close when it is not good to reopen. Otherwise it will jerk away commissions. 

			//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			std::cout << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create sell order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.longSell[reqId].myAsk << ":" << a.m_traderVector.longSell[reqId].sellingShares << ":" << a.m_traderVector.longBuy[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
				<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
				<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;

			a.m_myfileLong << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create sell order for symbol " << a.m_newtxtVector[reqId] << " with price: shares: totalPosition " << a.m_traderVector.longSell[reqId].myAsk << ":" << a.m_traderVector.longSell[reqId].sellingShares << ":" << a.m_traderVector.longBuy[reqId].totalPosition << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
				<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
				<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;

		}
	}
}

void TraderNormalLong::modifySellOrder(unsigned int &reqId, CurrentDataStructure &currentDataStructure)
{
	//**** changed to favorable, modify, change to non-favorable, not modify.
	unsigned int i = a.m_traderParameters.i_periodIndex;

	a.m_traderVector.longBuy[reqId].algoString = "whiteSwanLong";
	m_cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "CLOSE");

	if (a.m_traderVector.longSell[reqId].manualOrder)
		cancelExistingLongSellOrder(reqId, " Long: modifySellOrder, manualOrder = true ");
	{
		if (a.m_traderVector.longSell[reqId].relevantShares <= 0)
			cancelExistingLongSellOrder(reqId, " Long: modifySellOrder, relevantShares <= 0 ");
		else //relevantShares > 0
		{
			if (a.m_traderVector.longSell[reqId].basicClose == false) //**** When condition gets better, the app will cancel the order. 
				cancelExistingLongSellOrder(reqId, " Long: modifySellOrder, a.m_traderVector.longSell[reqId].basicClos == false ");
			else
			{
				if ((a.m_traderVector.longSell[reqId].myAsk + a.m_traderVector.longSell[reqId].minPriceVariation) <= a.m_traderVector.longBuy[reqId].myBid && a.m_traderVector.longSell[reqId].myAsk > 0 && a.m_traderVector.longBuy[reqId].myBid > 0)
					if (!a.m_traderVector.longSell[reqId].cancelling)
						if (a.m_traderVector.longSell[reqId].orderId > 0) cancelExistingLongSellOrder(reqId, " Long: I need cancel the long sell order because possible cross order ");


				if (a.m_traderVector.longBuy[reqId].myBid > 0) a.m_traderVector.longSell[reqId].calculatedAsk = max(a.m_traderVector.longSell[reqId].calculatedAsk, (a.m_traderVector.longBuy[reqId].myBid + 2 * a.m_traderVector.longSell[reqId].minPriceVariation));
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.longSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.longSell[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5)
				{
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.longSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100; //**** This include the case of closeLongMid. 

				bool priceChanged = false, sharesChanged = false;
				if (a.m_traderVector.longSell[reqId].sellingShares > 0)
				{
					double shareDifferenceRatio = ((double)a.m_traderVector.longSell[reqId].relevantShares - (double)a.m_traderVector.longSell[reqId].sellingShares) / (double)a.m_traderVector.longSell[reqId].sellingShares;
					if (abs(shareDifferenceRatio) > a.m_traderParameters.sharesUncertainty) sharesChanged = true;
				}
				priceChanged = a.m_traderVector.longSell[reqId].priceChanged && (abs(a.m_traderVector.longSell[reqId].myAsk - a.m_traderVector.longSell[reqId].calculatedAsk) >= a.m_traderVector.longSell[reqId].minPriceVariation);
				if (sharesChanged || priceChanged)
				{
					if (a.m_traderVector.longSell[reqId].calculatedAsk > 2 * a.m_traderVector.longSell[reqId].minPriceVariation)
					{
						int oldShares = a.m_traderVector.longSell[reqId].sellingShares;
						double oldPrice = a.m_traderVector.longSell[reqId].myAsk;

						a.m_traderVector.longSell[reqId].modifiedSell = false; //This is fine although it is an "advanced" value, see notes in modifyBuy().
						a.m_traderVector.longSell[reqId].lastModifiedTimeSell = time(0);
						a.m_traderVector.longSell[reqId].currentTotalSell = a.m_traderVector.longSell[reqId].relevantShares + a.m_traderVector.longSell[reqId].filledSell;
						a.m_traderVector.longSell[reqId].sellingShares = a.m_traderVector.longSell[reqId].relevantShares;

						if (a.m_traderVector.longSell[reqId].currentTotalSell > 0 && a.m_traderVector.longSell[reqId].calculatedAsk > 0)
						{
							time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
							modifyOrder_sub(reqId, "SELL", "LMT", a.m_traderVector.longSell[reqId].currentTotalSell, a.m_traderVector.longSell[reqId].calculatedAsk); //****calculatedAsk is assigned a value in the beginning of this function. 
							a.m_traderVector.longSell[reqId].numModifiedSell = a.m_traderVector.longSell[reqId].numModifiedSell + 1;

							//**** The output sentence below must be after placeOrder() in order for .orderId etc. to be initialized. 
							std::cout << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Modify sell order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.longSell[reqId].calculatedAsk << " : " << a.m_traderVector.longSell[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
								<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
								<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;
							a.m_myfileLong << tempString << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Modify sell order for symbol " << a.m_newtxtVector[reqId] << " from old price:shares " << oldPrice << " : " << oldShares << " to " << a.m_traderVector.longSell[reqId].calculatedAsk << " : " << a.m_traderVector.longSell[reqId].relevantShares << " with priceChanged:sharesChanged " << priceChanged << " : " << sharesChanged << " and BID:ASK " << a.m_currentDataStructureVector[reqId].bid << ":" << a.m_currentDataStructureVector[reqId].ask
								<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
								<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;

							if (a.m_traderVector.longSell[reqId].numModifiedSell > a.m_traderParameters.maxModificationsAllowed)
							{
								time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
								a.m_myfileLong << tempString << " Long Sell: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and order will be closed with BID price. " << std::endl;
								std::cout << tempString << " Long Sell: For " << a.m_newtxtVector[reqId] << " the number of modifications pass the limit and order will be closed with BID price. " << std::endl;
							}

						}
					}
				}
			}
		} //relevantShares > 0
	}
}

void TraderNormalLong::placeOrder(unsigned int &reqId, unsigned int &i, std::string action, std::string orderType, unsigned int &relevantShares, double &lmtPrice)
{
	if (i >= 0 && i < a.m_baseNum.size() && reqId >= 0 && reqId < a.m_newtxtVector.size())
	{
		Contract C_d;
		Order order_d;

		C_d.symbol = a.m_newtxtVector[reqId];
		C_d.secType = "STK";
		C_d.currency = "USD";
		C_d.exchange = "SMART"; //If necessary, I may include "exchange" in the newtxtVector in the future.
		order_d.clientId = a.m_clientIdLong;
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
			try{ a.m_csTimeDeque.EnterCriticalSection(); } 	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
				{
					bool b = true;
					try{ a.m_csOrderIdReadyLong.EnterCriticalSection(); }
					catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
					{
						order_d.orderId = a.m_orderIdLong;
						m_EC->placeOrder(a.m_orderIdLong, C_d, order_d);

						m_numModifications = m_numModifications + 1;

						if (action == "BUY")
						{
							a.m_traderVector.longBuy[reqId].opening = true;
							a.m_traderVector.longBuy[reqId].orderType = order_d.orderType;
							a.m_traderVector.longBuy[reqId].symbol = C_d.symbol;
							a.m_traderVector.longBuy[reqId].orderId = order_d.orderId;
							a.m_traderVector.longBuy[reqId].action = action;
							a.m_traderVector.longBuy[reqId].myBid = order_d.lmtPrice;
							a.m_traderVector.longBuy[reqId].currentTotalBuy = order_d.totalQuantity;
							a.m_traderVector.longBuy[reqId].buyingShares = order_d.totalQuantity;

						}
						if (action == "SELL")
						{
							a.m_traderVector.longSell[reqId].closing = true;
							a.m_traderVector.longSell[reqId].orderType = order_d.orderType;
							a.m_traderVector.longSell[reqId].symbol = C_d.symbol;
							a.m_traderVector.longSell[reqId].orderId = order_d.orderId;
							a.m_traderVector.longSell[reqId].action = action;
							a.m_traderVector.longSell[reqId].myAsk = lmtPrice;
							a.m_traderVector.longSell[reqId].currentTotalSell = relevantShares;
							a.m_traderVector.longSell[reqId].sellingShares = relevantShares;


						}

						a.m_orderIdLong = a.m_orderIdLong + 1;
						done = true;
						a.m_csOrderIdReadyLong.LeaveCriticalSection();
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
		a.m_myfileLong << "At " << tempString << ":" << " vector out of range for reqId : i = " << reqId << " : " << i << " at file " << __FILE__ << " and line " << __LINE__ << std::endl;
	}

}

void TraderNormalLong::modifyOrder_sub(unsigned int &reqId, std::string action, std::string orderType, unsigned int &totalQuantity, double &lmtPrice)
{
	Contract C_d;
	Order order_d;

	C_d.symbol = a.m_newtxtVector[reqId];
	C_d.secType = "STK";
	C_d.currency = "USD";
	C_d.exchange = "SMART"; //If necessary, I may include "exchange" in the newtxtVector in the future.
	order_d.clientId = a.m_clientIdLong;

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
		try{ a.m_csTimeDeque.EnterCriticalSection(); } 	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
			{
				if (action == "BUY")
				{
					a.m_traderVector.longBuy[reqId].myBid = order_d.lmtPrice;
					order_d.orderId = a.m_traderVector.longBuy[reqId].orderId;
				}
				if (action == "SELL")
				{
					a.m_traderVector.longSell[reqId].myAsk = order_d.lmtPrice;
					order_d.orderId = a.m_traderVector.longSell[reqId].orderId;
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
		if (counter > a.m_traderParameters. maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  }
	}

}

void TraderNormalLong::cleaning()
{
	//**** NEVER DELETE the following comments
	//**** In the cleaning process, never automatically reset something. For example, if the app automatically reset a trader vector, it may submit infinite number of opening orders. At most I can reset once with some prescribed counter controlled.
	//**** The cleaning process usually should only give warnings and let developer to be aware of something wrong.
	//**** No longer to complicate the cleaning process by counting the number of requests of open orders etc. This cleaning() has already been put in a time-restricted place. That is, it can be run only every, say, 10 seconds.
	for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
	{
		bool b = true;
		try { a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			//Part I. Warning if .opening = true but actually there are no open orders.
			bool noOpenOrders;
			noOpenOrders = ((time(0) - a.m_traderVector.longBuy[reqId].orderUpdatedTime) > 3 * a.m_traderParameters.timeBetweenReqOpenOrders); //**** if .orderUpdatedTime > time (0), it indicates it is default value time(0)*2
			if (noOpenOrders && a.m_traderVector.longBuy[reqId].opening) printf("There are no opening long orders but a.m_traderVector.longBuy[reqId].opening is %d for symbol %s.\n", a.m_traderVector.longBuy[reqId].opening, a.m_newtxtVector[reqId].c_str());

			noOpenOrders = ((time(0) - a.m_traderVector.longSell[reqId].orderUpdatedTime) > 3 * a.m_traderParameters.timeBetweenReqOpenOrders); //**** if .orderUpdatedTime > time (0), it indicates it is default value time(0)*2
			if (noOpenOrders && a.m_traderVector.longSell[reqId].closing) printf("There are no closing long orders but a.m_traderVector.longSell[reqId].closing is %d for symbol %s.\n", a.m_traderVector.longSell[reqId].closing, a.m_newtxtVector[reqId].c_str());

			noOpenOrders = ((time(0) - a.m_traderVector.shortSell[reqId].orderUpdatedTime) > 3 * a.m_traderParameters.timeBetweenReqOpenOrders); //**** if .orderUpdatedTime > time (0), it indicates it is default value time(0)*2
			if (noOpenOrders && a.m_traderVector.shortSell[reqId].opening) printf("There are no opening short orders but a.m_traderVector.shortSell[reqId].opening is %d for symbol %s.\n", a.m_traderVector.shortSell[reqId].opening, a.m_newtxtVector[reqId].c_str());

			noOpenOrders = ((time(0) - a.m_traderVector.shortBuy[reqId].orderUpdatedTime) > 3 * a.m_traderParameters.timeBetweenReqOpenOrders); //**** if .orderUpdatedTime > time (0), it indicates it is default value time(0)*2
			if (noOpenOrders && a.m_traderVector.shortBuy[reqId].closing) printf("There are no closing short orders but a.m_traderVector.shortBuy[reqId].closing is %d for symbol %s.\n", a.m_traderVector.shortBuy[reqId].closing, a.m_newtxtVector[reqId].c_str());

			if (a.m_traderVector.longBuy[reqId].totalPosition < 0 ) printf("Long position is negative %d for symbol %s.\n", a.m_traderVector.longBuy[reqId].totalPosition, a.m_newtxtVector[reqId].c_str());
			if (a.m_traderVector.shortSell[reqId].totalPosition > 0) printf("Short position is positive %d for symbol %s.\n", a.m_traderVector.shortSell[reqId].totalPosition, a.m_newtxtVector[reqId].c_str());

			//Part II. 
			//**** No deletion: m_algoStringVector shares the critical section of traderVector. Thus it must always be within that critical section. 
			if (a.m_traderVector.longBuy[reqId].totalPosition != 0 && a.m_traderVector.longBuy[reqId].algoString == "EMPTY")
			printf("Long position is not zero but a.m_algoStringVectorOpen is EMPTY.\n");

			if (a.m_traderVector.shortSell[reqId].totalPosition != 0 && a.m_traderVector.shortSell[reqId].algoString == "EMPTY")
			printf("Short position is not zero but a.m_algoStringVectorOpen is EMPTY.\n");

			a.m_csTraderVector.LeaveCriticalSection();
		}
	}
}


void TraderNormalLong::manualReset()
{
	double newParameter = 0;
	std::string tempString;
	std::istringstream instream;
	std::string String, symbol, actionString, inputFileString = "C:/symbols/Inputfile"; inputFileString.append(a.m_accountCode);  inputFileString.append(".txt"); //This make sure the application calls the right file.
	std::ifstream i_inputFile(inputFileString, std::ios_base::in);
	m_inputStructureVectorLong.clear();
	if (!i_inputFile.is_open())
		printf("Failed in opening the InputfileLong***.txt.\n");
	else
	{
		std::istringstream instream;
		InputStructure inputStructure;
		std::string wholeLineString, subString;
		std::vector<std::string> stringVector;
		std::istringstream instream0, instream1, instream2;
		while (getline(i_inputFile, wholeLineString, '\n'))
		{
			std::stringstream   linestream(wholeLineString);
			stringVector.clear();
			while (getline(linestream, subString, ','))
			{
				stringVector.push_back(subString);
			}
			instream0.clear(); instream1.clear(); instream2.clear();
			instream0.str(stringVector[0]); instream1.str(stringVector[1]); instream2.str(stringVector[2]);
			instream0 >> inputStructure.string0; instream1 >> inputStructure.string1; instream2 >> inputStructure.string2; //**** sometimes I need a number and sometimes I need a string.
			m_inputStructureVectorLong.push_back(inputStructure);
		}//while (getline(i_inputFile, wholeLineString, '\n'))
		i_inputFile.close();
	}

	for (unsigned int index = 0; index < m_inputStructureVectorLong.size(); index++) //**** index = 0 is reserved for different information
	{
		symbol = m_inputStructureVectorLong[index].string0;
		actionString = m_inputStructureVectorLong[index].string1;
		std::istringstream instream(m_inputStructureVectorLong[index].string2); instream >> newParameter; //instream >> tempString;
		if (symbol == "ALLSYMBOLS")
		{
			if (actionString == "volatilityFactorLong")
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_volatilityFactorLong = newParameter; //***** a.m_volatilityFactorLong must always be protected by the locks of a.m_csTraderVector
					a.m_volatilityFactorLong = max(a.m_volatilityFactorLong, 1.0);

					a.m_csTraderVector.LeaveCriticalSection();
				}
				printf("a.m_volatilityFactorLong is set to be %f. \n", a.m_volatilityFactorLong);
			}
			else if (actionString == "volatilityFactorShort")
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_volatilityFactorShort = newParameter;
					a.m_volatilityFactorShort = max(a.m_volatilityFactorShort, 1.0);
	
					a.m_csTraderVector.LeaveCriticalSection();
				}
				printf("a.m_volatilityFactorShort is set to be %f. \n", a.m_volatilityFactorShort);
			}
			else if (actionString == "closeLongPositions")
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_closeLongPositions = true;
					a.m_csTraderVector.LeaveCriticalSection();
				}
				printf("a.m_closeLongPositions is set to be true. \n");
			}
			else if (actionString == "closeShortPositions")
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					a.m_closeShortPositions = true;
					a.m_csTraderVector.LeaveCriticalSection();
				}
				printf("a.m_closeShortPositions is set to be true.\n");
			}
			else if (actionString == "SHOWALLCONDITIONS")
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					showAllTradingConditions();
					a.m_csTraderVector.LeaveCriticalSection();
				}
			}
		}
		else // for action on a specific symbol.
		{
			int reqId = -1; //**** Defined for not modifying legacy code
			bool b = true;
			try { a.m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				auto it = a.m_newtxtMap.find(symbol);
				if (it != a.m_newtxtMap.end()) reqId = it->second;
				a.m_csTraderVector.LeaveCriticalSection();
			}

			if (reqId >= 0 && reqId < a.m_newtxtVector.size())
			{
				bool b = true;
				try { a.m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if(actionString == "SHOWSINGLECONDITIONS")
					{
						showSingleTradingConditions(reqId);
					}
					else if (actionString == "MANUALORDER")
					{
						a.m_traderVector.longBuy[reqId].manualOrder = true;
						a.m_traderVector.longSell[reqId].manualOrder = true;
						a.m_traderVector.shortSell[reqId].manualOrder = true;
						a.m_traderVector.shortBuy[reqId].manualOrder = true;

					}
					else if (actionString == "PRINTSTATUSLONG")
					{
						bool b = true;
						try { a.m_csCurrentDataStructureVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
						{
							printf("BID is %f and ASK is %f for symbol %s.\n", a.m_currentDataStructureVector[reqId].bid, a.m_currentDataStructureVector[reqId].ask, a.m_newtxtVector[reqId].c_str());
							a.m_csCurrentDataStructureVector.LeaveCriticalSection();
						}

						printf("myBid is %f buy and %f sell.\n", a.m_traderVector.longBuy[reqId].myBid, a.m_traderVector.longSell[reqId].myBid);
						printf("totalPosition is %d and %d.\n", a.m_traderVector.longBuy[reqId].totalPosition, a.m_traderVector.longSell[reqId].totalPosition);
						printf("buyingShares is %d and  %d. \n", a.m_traderVector.longBuy[reqId].buyingShares, a.m_traderVector.longSell[reqId].buyingShares);
						printf("currentTotalBuy is %d and %d.\n", a.m_traderVector.longBuy[reqId].currentTotalBuy, a.m_traderVector.longSell[reqId].currentTotalBuy);
						printf("currentTotalSell is %d and %d.\n", a.m_traderVector.longBuy[reqId].currentTotalSell, a.m_traderVector.longSell[reqId].currentTotalSell);
						printf("averageCost is %f and %f.\n", a.m_traderVector.longBuy[reqId].averageCost, a.m_traderVector.longSell[reqId].averageCost);
						printf("closing is %d and %d.\n", a.m_traderVector.longBuy[reqId].closing, a.m_traderVector.longSell[reqId].closing);
						printf("opening is %d and %d.\n", a.m_traderVector.longBuy[reqId].opening, a.m_traderVector.longSell[reqId].opening);
						printf("cancelling is %d and %d.\n", a.m_traderVector.longBuy[reqId].cancelling, a.m_traderVector.longSell[reqId].cancelling);
						printf("orderId is %d and %d.\n", a.m_traderVector.longBuy[reqId].orderId, a.m_traderVector.longSell[reqId].orderId);
						printf("symbol is %s and %s.\n", a.m_traderVector.longBuy[reqId].symbol.c_str(), a.m_traderVector.longSell[reqId].symbol.c_str());
						printf(".sellingShares is %d and %d.\n", a.m_traderVector.longBuy[reqId].sellingShares, a.m_traderVector.longSell[reqId].sellingShares);
						printf("action is %s and %s.\n", a.m_traderVector.longBuy[reqId].action.c_str(), a.m_traderVector.longSell[reqId].action.c_str());
						printf("myAsk is %f and %f.\n", a.m_traderVector.longBuy[reqId].myAsk, a.m_traderVector.longSell[reqId].myAsk);
						printf("modifiedBuy is %d and %d.\n", a.m_traderVector.longBuy[reqId].modifiedBuy, a.m_traderVector.longSell[reqId].modifiedBuy);
						printf("modifiedSell is %d and %d.\n", a.m_traderVector.longBuy[reqId].modifiedSell, a.m_traderVector.longSell[reqId].modifiedSell);
						printf("stopLossLong is %f.\n", a.m_traderVector.longBuy[reqId].stopLossLong);
						printf("a.m_algoStringVectorOpen for symbol %s is %s.\n", a.m_traderVector.longBuy[reqId].symbol.c_str(), a.m_traderVector.longBuy[reqId].algoString.c_str());

					}
					else if (actionString == "PRINTSTATUSSHORT")
					{
						bool b = true;
						try { a.m_csCurrentDataStructureVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
						{
							printf("BID is %f and ASK is %f for symbol %s.\n", a.m_currentDataStructureVector[reqId].bid, a.m_currentDataStructureVector[reqId].ask, a.m_newtxtVector[reqId].c_str());
							a.m_csCurrentDataStructureVector.LeaveCriticalSection();
						}

						printf("myBid is %f buy and %f sell.\n", a.m_traderVector.shortSell[reqId].myBid, a.m_traderVector.shortBuy[reqId].myBid);
						printf("totalPosition is %d and %d.\n", a.m_traderVector.shortSell[reqId].totalPosition, a.m_traderVector.shortBuy[reqId].totalPosition);
						printf("buyingShares is %d and  %d. \n", a.m_traderVector.shortSell[reqId].buyingShares, a.m_traderVector.shortBuy[reqId].buyingShares);
						printf("currentTotalBuy is %d and %d.\n", a.m_traderVector.shortSell[reqId].currentTotalBuy, a.m_traderVector.shortBuy[reqId].currentTotalBuy);
						printf("currentTotalSell is %d and %d.\n", a.m_traderVector.shortSell[reqId].currentTotalSell, a.m_traderVector.shortBuy[reqId].currentTotalSell);
						printf("averageCost is %f and %f.\n", a.m_traderVector.shortSell[reqId].averageCost, a.m_traderVector.shortBuy[reqId].averageCost);
						printf("closing is %d and %d.\n", a.m_traderVector.shortSell[reqId].closing, a.m_traderVector.shortBuy[reqId].closing);
						printf("opening is %d and %d.\n", a.m_traderVector.shortSell[reqId].opening, a.m_traderVector.shortBuy[reqId].opening);
						printf("cancelling is %d and %d.\n", a.m_traderVector.shortSell[reqId].cancelling, a.m_traderVector.shortBuy[reqId].cancelling);
						printf("orderId is %d and %d.\n", a.m_traderVector.shortSell[reqId].orderId, a.m_traderVector.shortBuy[reqId].orderId);
						printf("symbol is %s and %s.\n", a.m_traderVector.shortSell[reqId].symbol.c_str(), a.m_traderVector.shortBuy[reqId].symbol.c_str());
						printf(".sellingShares is %d and %d.\n", a.m_traderVector.shortSell[reqId].sellingShares, a.m_traderVector.shortBuy[reqId].sellingShares);
						printf("action is %s and %s.\n", a.m_traderVector.shortSell[reqId].action.c_str(), a.m_traderVector.shortBuy[reqId].action.c_str());
						printf("myAsk is %f and %f.\n", a.m_traderVector.shortSell[reqId].myAsk, a.m_traderVector.shortBuy[reqId].myAsk);
						printf("modifiedBuy is %d and %d.\n", a.m_traderVector.shortSell[reqId].modifiedBuy, a.m_traderVector.shortBuy[reqId].modifiedBuy);
						printf("modifiedSell is %d and %d.\n", a.m_traderVector.shortSell[reqId].modifiedSell, a.m_traderVector.shortBuy[reqId].modifiedSell);
						printf("stopLossShort is %f.\n", a.m_traderVector.shortSell[reqId].stopLossShort);
						printf("a.m_algoStringVectorOpen for symbol %s is %s.\n", a.m_traderVector.shortSell[reqId].symbol.c_str(), a.m_traderVector.shortSell[reqId].algoString.c_str());
					}
					a.m_csTraderVector.LeaveCriticalSection();
				}
			}
		}
	}
}

void TraderNormalLong::showAllTradingConditions()
{
	//**** Later if for several types of TFLongs for example, then we can use algoStringVector types to design vectors, but not as below where we use MrLong, TFLong.....
	std::vector<std::string> tempVector; tempVector.clear();
	std::vector<std::vector<std::string> > longSymbolsVectorBuy;longSymbolsVectorBuy.clear();
	std::vector<std::vector<std::string> > longSymbolsVectorSell; longSymbolsVectorSell.clear();
	std::vector<std::vector<std::string> > shortSymbolsVectorSell; shortSymbolsVectorSell.clear();
	std::vector<std::vector<std::string> > shortSymbolsVectorBuy; shortSymbolsVectorBuy.clear();

	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		longSymbolsVectorBuy.push_back(tempVector);
		longSymbolsVectorSell.push_back(tempVector);
		shortSymbolsVectorSell.push_back(tempVector);
		shortSymbolsVectorBuy.push_back(tempVector);
	}
	
	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
		{
			if (a.m_traderVector.longBuy[reqId].conditions[j]) longSymbolsVectorBuy[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.longSell[reqId].conditions[j])longSymbolsVectorSell[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.shortSell[reqId].conditions[j]) shortSymbolsVectorSell[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.shortBuy[reqId].conditions[j]) shortSymbolsVectorBuy[j].push_back(a.m_newtxtVector[reqId]);
		}
	}

	time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
	for (unsigned int i = 0; i < a.m_traderParameters.maxNumConditions; i++)
	{
		if (longSymbolsVectorBuy[i].size() > 0)      
		{
			printf("The number of symbols satisfying longBuy[reqId].conditions.condition %d is %d, e.g. %s.\n", i, longSymbolsVectorBuy[i].size(), longSymbolsVectorBuy[i][0].c_str());
			//a.m_myfileLong << tempString << "The number of symbols satisfying longBuy[reqId].conditions.condition " << i << " is " << longSymbolsVectorBuy[i].size() << " , e.g., " << longSymbolsVectorBuy[i][0] << std::endl;
		}
	}
	for (unsigned int i = 0; i < a.m_traderParameters.maxNumConditions; i++)
	{
		if (longSymbolsVectorSell[i].size() > 0)
		{
			printf(" The number of symbols satisfying longSell[reqId].conditions.condition %d is %d, e.g. %s.\n", i, longSymbolsVectorSell[i].size(), longSymbolsVectorSell[i][0].c_str());
			//a.m_myfileLong << tempString << "The number of symbols satisfying longSell[reqId].conditions.condition " << i << " is " << longSymbolsVectorSell[i].size() << " , e.g., " << longSymbolsVectorSell[i][0] << std::endl;
		}
	}

	for (unsigned int i = 0; i < a.m_traderParameters.maxNumConditions; i++)
	{
		if (shortSymbolsVectorSell[i].size() > 0)
		{
			printf(" The number of symbols satisfying shortSell[reqId].conditions.condition %d is %d, e.g. %s.\n", i, shortSymbolsVectorSell[i].size(), shortSymbolsVectorSell[i][0].c_str());
			//a.m_myfileLong << tempString << "The number of symbols satisfying shortSell[reqId].conditions.condition " << i << " is " << shortSymbolsVectorSell[i].size() << " , e.g., " << shortSymbolsVectorSell[i][0] << std::endl;
		}
	}

	for (unsigned int i = 0; i < a.m_traderParameters.maxNumConditions; i++)
	{
		if (shortSymbolsVectorBuy[i].size() > 0)
		{
			printf(" The number of symbols satisfying shortBuy[reqId].conditions.condition %d is %d, e.g. %s.\n", i, shortSymbolsVectorBuy[i].size(), shortSymbolsVectorBuy[i][0].c_str());
			//a.m_myfileLong << tempString << "The number of symbols satisfying shortBuy[reqId].conditions.condition " << i << " is " << shortSymbolsVectorBuy[i].size() << " , e.g., " << shortSymbolsVectorBuy[i][0] << std::endl;
		}
	}
}

void TraderNormalLong::showSingleTradingConditions(int &reqId)
{
	time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		if (a.m_traderVector.longBuy[reqId].conditions[j])
		{
			printf(" symbol %s satisfies the longBuy[reqId].conditions.condition %d.\n", a.m_newtxtVector[reqId].c_str(), j);
			//a.m_myfileLong << tempString <<  " symbol " << a.m_newtxtVector[reqId] << " satisfies longBuy[reqId].conditions.condition " << j << std::endl;
		}
	}
	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		if (a.m_traderVector.longSell[reqId].conditions[j])
		{
			printf(" symbol %s satisfies the longSell[reqId].conditions.condition %d.\n", a.m_newtxtVector[reqId].c_str(), j);
			//a.m_myfileLong << tempString << " symbol " << a.m_newtxtVector[reqId] << " satisfies longSell[reqId].conditions.condition " << j << std::endl;
		}
	}
	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		if (a.m_traderVector.shortSell[reqId].conditions[j])
		{
			printf(" symbol %s satisfies the shortSell[reqId].conditions.condition %d.\n", a.m_newtxtVector[reqId].c_str(), j);
			//a.m_myfileLong << tempString << " symbol " << a.m_newtxtVector[reqId] << " satisfies shortSell[reqId].conditions.condition " << j << std::endl;
		}
	}
	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		if (a.m_traderVector.shortBuy[reqId].conditions[j])
		{
			printf(" symbol %s satisfies the shortBuy[reqId].conditions.condition %d.\n", a.m_newtxtVector[reqId].c_str(), j);
			//a.m_myfileLong << tempString << " symbol " << a.m_newtxtVector[reqId] << "  satisfies shortBuy[reqId].conditions.condition " << j << std::endl;
		}
	}
	

	printf(" a.m_traderVector.longBuy[reqId].algoString has a value of %s for symbol %s.\n", a.m_traderVector.longBuy[reqId].algoString.c_str(), a.m_newtxtVector[reqId].c_str());
	//a.m_myfileLong << tempString << "a.m_traderVector.longBuy[reqId].algoString has a value of " << a.m_traderVector.longBuy[reqId].algoString << " for symbol " << a.m_newtxtVector[reqId] << std::endl;
}