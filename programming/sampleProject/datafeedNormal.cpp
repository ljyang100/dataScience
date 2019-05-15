#include "datafeedNormal.h"

void DatafeedNormal::error(const int id, const int errorCode, const IBString errorString)
{
	if (errorCode == 2105 || errorCode == 2106)
	{
		//No action required because these two are related to historical data and are handled in historical data class. 
	}
	else if (errorCode == 1100) //Connection between IB and TWS has been lost.  
	{
		//**** I use errorCode 2103 to record the internet disconnection caused inactivity of data client.
		//**** Normally 1100 comes later after 2103. Sometimes 1100 even internet come back (dataclient = true already) with 2104. So 1100 will set wrongly set dataclient = false even after 2104 set it to be true. 
		//**** However, this is OK (but not OK in the historical client) because anyway 1101 or 1102 will set dataClient = true later. So I don't use 1100 here. But it is OK even I use it to set dataClient = false. 
		//**** Note I cannot use only 2104 in datafeed becaues I need to decide whether to re-subscribe data or not according to whether the data is lost of maintained (1101, 1102). 
	
	}
	else if (errorCode == 1101) //Connectivity between IB and TWS (for this client) is restored, but data lost (and thus need request again and account data subscription). Here the request related to this client is only reqMktData(). 
	{
		Contract			C;
		C.secType = "STK";
		C.currency = "USD";
		C.exchange = "SMART";
		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			std::chrono::high_resolution_clock::time_point now;
			int counter = 0;
			bool done = false;
			while (1)
			{
				bool b = true;
				try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
					{
						C.symbol = a.m_newtxtVector[i];
						m_EC->cancelMktData(i);
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);
						done = true;
					}
					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)
		} //end of for loop

		printf("At %s, Datafeed: Market data was lost before. To request again, I have just cancelled the former request.\n", tempString.c_str());

		Sleep(5000);
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
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
						C.symbol = a.m_newtxtVector[i];
						m_EC->reqMktData(i, C, "233,236", false); //233 for RTVolume, 236 for shortable. 
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);

						done = true;
					}

					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)
		} //end of for loop
		printf("At %s, Datafeed: Market data was lost before. After just cancelling the previous request, I have just requested again for all symbols.\n", tempString.c_str());

	} //end of errorCode = 1101
	else if (errorCode == 1102)
	{
		Contract			C;
		C.secType = "STK";
		C.currency = "USD";
		C.exchange = "SMART";
		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
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
						C.symbol = a.m_newtxtVector[i];
						m_EC->cancelMktData(i);
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);
						done = true;
					}
					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)
		} //end of for loop

		printf("At %s, Datafeed: Market data was restored and maintained. But I still cancelled the former request in order to request again.\n", tempString.c_str());

		Sleep(5000);
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
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
						C.symbol = a.m_newtxtVector[i];
						m_EC->reqMktData(i, C, "233,236", false); //233 for RTVolume, 236 for shortable. 
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);

						done = true;
					}

					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)
		} //end of for loop
		printf("At %s, Datafeed: Market data was lost and maintained. But I still cancelled the former request and request again for all symbols.\n", tempString.c_str());

	}
	else if (errorCode == 1300)
	{
		// email to me. Must handle immediately. 
	}
	else if (errorCode == 200) //meaning "No security defintion for the request", 
	{
		if ( id >= 0 && id < (const int) a.m_newtxtVector.size()) 
			printf("Datafeed: No security defintion for symbol %s.\n", a.m_newtxtVector[id].c_str() );
		else 
		{
			printf("Datafeed: No security defintion for a un-recorded symbol with id = %d", id );
		}
	}
	else
	{
		printf( " errorcode %d from datafeed messages %s. \n", errorCode, (const char*)errorString); 

		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
	}

}

void   DatafeedNormal::winError(const IBString& str, int lastError)
{
	printf( "WinError for the datafeed client: %d = %s\n", lastError, (const char*)str );
}

void  DatafeedNormal::DatafeedEntryPoint()
{
	Contract			C;
	C.secType			= "STK";
	C.currency			= "USD";
	C.exchange			= "SMART";

	if (a.m_traderParameters.fullDataLinesSubscribed )
	{
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++) //**** Start from 1 because zeroth already submitted.
		{
			std::chrono::high_resolution_clock::time_point now;
			int counter = 0;
			bool done = false;
			while (1)
			{
				bool b = true;
				try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
					{
						C.symbol = a.m_newtxtVector[i];
						m_EC->reqMktData(i, C, "", false, TagValueListSPtr());
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);

						done = true;
					}
					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("I cannot reqMktData after 0.5 seconds, which is weird.\n"); }
			} //end of while(1)
			if (i == a.m_newtxtVector.size() - 1)
			{
				time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
				printf("At %s,  I have requested marketData for the last symbol in my list.\n", tempString.c_str());
			}
		} //end of for loop
	}
	else //**** data cycling. The case of a.m_newtxtVector.size() <= a.m_traderParameters.numDataRequested has been handled earlier. So here we must have > a.m_traderParameters.numDataRequested.
	{
		//**** The case of a.m_newtxtVector.size() <= a.m_traderParameters.numDataRequested has been handled earlier. So here we must have > a.m_traderParameters.numDataRequested.
		int counterOutside = 0;
		int reqId = 0; //**** Note the = 0 case has been handled in the beginning.
		while(1) 
		{
			Sleep(100); //rest 100ms between each round. Note let consume too much CPU, and let cancelling of data subscription done
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
						//reqMktData(TickerId id, const Contract &contract, const IBString& genericTicks, bool snapshot, const TagValueListSPtr& mktDataOptions); //Prototype.
						C.symbol = a.m_newtxtVector[reqId];
						m_EC->reqMktData(reqId, C, "", false, TagValueListSPtr());
						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);
						counterOutside = counterOutside + 1;
						m_submittedReqIdVector.push_back(reqId);
						if (reqId == (a.m_newtxtVector.size() - 1))
							reqId = 0; //***** This very important! This is like a periodic boundary condition, saving a lot of time in handling index such as whether it is multiple of 100, etc., as did in my first app which only cancel orders.
						else
							reqId = reqId + 1;
						done = true;
					}
					a.m_csTimeDeque.LeaveCriticalSection();
				}

				if (done == true) break; else { Sleep(100); counter = counter + 1; }
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("I cannot reqMktData after 0.5 seconds, which is weird.\n"); }
			} //end of while(1)

			if (counterOutside == a.m_traderParameters.numDataRequested) 
			{
				std::this_thread::sleep_for(std::chrono::seconds(a.m_traderParameters.maxWaitingTimeForMarketData)); //**** normally wait for three seconds for the market data of 100 symbols to feed back.
				for (unsigned int i = 0; i < m_submittedReqIdVector.size(); i++)
				{
					std::chrono::high_resolution_clock::time_point now;
					int counter = 0;
					bool done = false;
					while (1)
					{
						bool b = true;
						try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
						{
							if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
							{
								m_EC->cancelMktData(m_submittedReqIdVector[i]); // Here I don't need RTVolume because I don't construct bars. 
								now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
								a.m_timeDeque.push_back(now);
								done = true;
							}
							a.m_csTimeDeque.LeaveCriticalSection();
						}

						if (done == true) break; else { Sleep(100); counter = counter + 1; }
						if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("I cannot reqMktData after 0.5 seconds, which is weird.\n"); }
					} //end of while(1)
				}

				if (reqId >= 0 && reqId < a.m_newtxtVector.size())
				{
					if (((time(0) - m_nextHeartBeat) >= 60) && (time(0) != m_nextHeartBeat)) //**** print every 60 seconds in order to know market data is going on.
					{
						//**** Keep this print out so as to know that the data is always there. 
						time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						std::cout << "At " << tempString << ":" << " I have requested marketData for " << a.m_traderParameters.numDataRequested << " symbols. The previous symbol is " << a.m_newtxtVector[reqId] << std::endl;
						m_nextHeartBeat = time(0);
					}
				}

				counterOutside = 0;
				m_submittedReqIdVector.clear();
			} // if (counterOutside == a.m_traderParameters.numDataRequested)
		} // outmost while(1)
	}
}

void  DatafeedNormal::tickSize(TickerId tickerId, TickType field, int size)
{
	bool b=true; 
	try{ a.m_csCurrentDataStructureVector.EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
	{
		if(field == ASK_SIZE) //**** Note the field cannot be ASK, BID, or LAST here. Using the follwing brackets to check whether code can really run into here.
		{
			a.m_currentDataStructureVector[tickerId].askSize = size; 
		}
		if(field == BID_SIZE)
		{
			a.m_currentDataStructureVector[tickerId].bidSize = size;
		}
		if(field == LAST_SIZE)
		{
			a.m_currentDataStructureVector[tickerId].lastSize = size; 
		}
		a.m_csCurrentDataStructureVector.LeaveCriticalSection();
	}
}
void  DatafeedNormal::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)
{
	bool b = true;
	try{ a.m_csCurrentDataStructureVector.EnterCriticalSection(); }		catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		if (field == ASK)		a.m_currentDataStructureVector[tickerId].ask = price;
		if (field == BID)		a.m_currentDataStructureVector[tickerId].bid = price;
		if (field == LAST)		a.m_currentDataStructureVector[tickerId].last = price; 
		if (field == OPEN)		a.m_currentDataStructureVector[tickerId].open = price; //today's open
		if (field == HIGH)		a.m_currentDataStructureVector[tickerId].high = price; //Today's high
		if (field == LOW)		a.m_currentDataStructureVector[tickerId].low = price;
		if (field == CLOSE)		a.m_currentDataStructureVector[tickerId].close = price; //Yesterday's close

		a.m_csCurrentDataStructureVector.LeaveCriticalSection();
	}
}

