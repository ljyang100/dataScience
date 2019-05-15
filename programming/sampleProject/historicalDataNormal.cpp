#include "historicalDataNormal.h"
// This class acquires data from IB, construct bars, etc. 
// Need do things simultaneously and thus need multithreading programming. 

void HistoricalDataNormal::error(const int id, const int errorCode, const IBString errorString)
{
	if (errorCode == 162) //No historical data returned. This might be due to very unliquid symbol or other reason. 
	{
		//printf("***CS*** %s %d\n",__FILE__,__LINE__);	printf("Error for id=%d: %d = %s\n", id, errorCode, (const char*)errorString );  

		std::string violation = "violation", query = "query returned no";
		std::string tempStringViolation, tempStringQuery;
		tempStringViolation.clear(); tempStringQuery.clear();

		int violation_start = 0, violationSize = 9, query_start = 0, querySize = 17;
		bool v_found = false, q_found = false;

		for (unsigned int i = 0; i < errorString.size(); i++)
		{
			if (errorString.at(i) == 'v' && errorString.at(i + 1) == 'i' && errorString.at(i + 2) == 'o') { v_found = true; violation_start = i; tempStringViolation.append(std::string(1, errorString.at(i))); break; } //**** Note service err will be chosen if without the third condition.
			if (errorString.at(i) == 'q' && errorString.at(i + 1) == 'u' && errorString.at(i + 2) == 'e') { q_found = true; query_start = i; tempStringQuery.append(std::string(1, errorString.at(i))); break; } //**** I can add more and more condition to 100% locate the message.
		}

		if (v_found) for (int i = violation_start + 1; i < (violation_start + violationSize); i++) tempStringViolation.append(std::string(1, errorString.at(i)));
		if (q_found) for (int i = query_start + 1; i < (query_start + querySize); i++) tempStringQuery.append(std::string(1, errorString.at(i)));

		if (tempStringViolation == "violation")
		{
			printf("I found a pacing violation error and already stop requesting historical data for 10 minutes. Check it out!\n");
		}

		if (tempStringQuery == query)
		{
			if (id >= 0 && id < (const int)a.m_newtxtVector.size())
			{
				//printf("Set dayBar.ready = true for symbol %s because errorCode 162 says no dayBar so far.\n", a.m_newtxtVector[id].symbol.c_str() ); //**** Commented out because there are so many OTC symbols without day bars.
				bool b = true;
				try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					m_historicalDataEndVector[id] = true; //**** After setting this to be true the historical data of the sysmbol will use its default. The application will act according to this default. 
					m_csHistoricalDataEndVector.LeaveCriticalSection();
				}
			}
		}
	}
	else if (errorCode == 200) //meaning "No security defintion for", 
	{
		if (id >= 0 && id < (const int) a.m_newtxtVector.size()) 		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
		{
			printf("No security defintion for symbol %s for historical data download. The corresponding flag is set to be true for continue. \n", a.m_newtxtVector[id].c_str());

			bool b = true;
			try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				m_historicalDataEndVector[id] = true; //**** After setting this to be true the historical data of the sysmbol will use its default. The application will act according to this default. 
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}
		}
		else
		{
			printf("***CS*** %s %d\n", __FILE__, __LINE__);
			printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString);
		}
	}
	else if (errorCode == 203) //meaning the security is not available for this account.
	{
		if (id >= 0 && id < (const int)a.m_newtxtVector.size()) 		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
		{
			printf("***CS*** %s %d\n", __FILE__, __LINE__);
			printf("The security %s is not available for this account. The corresponding flag is set to be true for continue.\n", a.m_newtxtVector[id].c_str());

			bool b = true;
			try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				m_historicalDataEndVector[id] = true; //**** After setting this to be true the historical data of the sysmbol will use its default. The application will act according to this default. 
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}
		}
		else
		{
			printf("***CS*** %s %d\n", __FILE__, __LINE__);
			printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString);
		}
	}
	else if (errorCode == 504)
	{
		//Emailed to me. Because even it is not connected the app will wait a specific amount of time for each symbol and finally getting out. 
	}
	else if (errorCode == 1100) //Connection between IB and TWS has been lost.  
	{

	}
	else if (errorCode == 1101) //Connectivity between IB and TWS (for this client) is restored, but data lost (and thus need request again and account data subscription). Here the request related to this client is only reqMktData(). 
	{
		printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("TWS and IB connection restored for this client, but market data subscription lost. I have resubmitted the data request.\n"); 
	} //end of errorCode = 1101
	else if (errorCode == 1102)
	{

	}
	else if (errorCode == 1300)
	{
		// email to me. Must handle immediately. 
	}
	else
	{
		if (id >= 0 && id < (const int) a.m_newtxtVector.size()) printf("historicalData: Error for id=%d: %d = %s. For symbol %s. \n", id, errorCode, (const char*)errorString, a.m_newtxtVector[id].c_str());
		else printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString);
		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
	}
}

void   HistoricalDataNormal::winError(const IBString& str, int lastError)
{
	printf("historicalData: WinError for the datafeed client: %d = %s\n", lastError, (const char*)str);
}

void  HistoricalDataNormal::connectToTWS()
{
	m_EC = EClientL0::New( this); 
	if (m_EC == NULL) { printf("Creating EClient failed.\n"); 	getchar(); 	exit(0);} //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
	m_EC->eConnect("", a.m_port, a.m_clientId_HistoricalData_S); ////***** This function starts a new thread automatically. Therefore, this function can never be in another threadEntryPoint function. I once made very big mistake and wastes my many hours.	

	for (unsigned int J = 0; J < a.m_newtxtVector.size(); J++) 
	{
		bool b=true; 
		try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			m_historicalDataEndVector.push_back(false);
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}
	}
}

void  HistoricalDataNormal::downloadHistoricalData_Common()
{
	struct tm *ts = localtime(&a.m_timeNow);
	strftime(m_buf, sizeof(m_buf), "%Y%m%d %H:%M:%S", ts);
	std::stringstream ss;
	if (a.m_bardurationS >= 60)
	{
		//Note in this structure I have only included 1D case, and those mins bar. Week, Month bars are not included. 
		if (a.m_bardurationS / 86400 == 1) { m_barSize = "";	ss.str(""); ss << (a.m_bardurationS / 86400); m_barSize.append(ss.str()); m_barSize.append(" "); }
		else { m_barSize = "";	ss.str(""); ss << (a.m_bardurationS / 60); m_barSize.append(ss.str()); m_barSize.append(" "); }

		if (a.m_bardurationS / 60 == 1) 	m_barSize.append("min");
		else { if (a.m_bardurationS / 86400 == 1) m_barSize.append("day"); else m_barSize.append("mins"); } //Note in this structure I have only included 1D case, and those mins bar. Hour, Day, Week, Month bars are not included. 
	}
	else
	{
		m_barSize = "";
		ss.str(""); ss << (a.m_bardurationS); m_barSize.append(ss.str()); m_barSize.append(" ");
		m_barSize.append("secs");
	}

	std::stringstream ss1;
	if (a.m_daysOfHistoricalDataS <= 2)
	{
		ss1.str(""); ss1 << a.m_daysOfHistoricalDataS; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("D");
	}
	else if ((a.m_daysOfHistoricalDataS >2) && (a.m_daysOfHistoricalDataS < 30)) //m_daysOfHistoricalData are in units of 7 days (week)
	{
		int ssTemp = a.m_daysOfHistoricalDataS / 7;
		ss1.str(""); ss1 << ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("W");
	}
	else if ((a.m_daysOfHistoricalDataS >= 30) && (a.m_daysOfHistoricalDataS < 360)) //m_daysOfHistoricalData are in units of 30 days (month)
	{
		int ssTemp = a.m_daysOfHistoricalDataS / 30;
		ss1.str(""); ss1 << ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("M");
	}
	else if (a.m_daysOfHistoricalDataS >= 360) //m_daysOfHistoricalData are in units of 360 days (Year)
	{
		int ssTemp = a.m_daysOfHistoricalDataS / 360;
		ss1.str(""); ss1 << ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("Y");
	}

}

void  HistoricalDataNormal::downloadHistoricalData_Single()
{
	for (unsigned int J = 0; J < a.m_newtxtVector.size(); J++)
	{
		bool b = true;
		try{ m_csHistoricalDataEndVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_historicalDataEndVector[J] = false;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}
	}

	m_durationString.clear();
	downloadHistoricalData_Common(); //For downloading before trading, these function can be outside the while loop below. If downloading during trading, then it should be within the while loop.

	Contract			C;
	C.symbol = a.m_newtxtVector[0]; //Normally this is the symbol "SPY".
	C.secType = "STK";
	C.currency = "USD";
	C.exchange = "SMART";

	bool SPY_Done = false; //**** Sometimes the application is started before trading and database farm is not connected. Thus I need wait until it is done. Otherwise I cannot continue without the guidence of SPY dates.
	while (!SPY_Done)
	{
		time_t initialTime, finalTime;

		m_liquidStock.clear(); //**** Be careful emptify the vector before each trial downloading. 
		bool done = false;
		int counter = 0;
		while (1) //This makes sure that the relevant request must be sent out, even I have to wait for a while. 
		{
			std::chrono::high_resolution_clock::time_point now;
			bool b = true;
			try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
				{
					m_EC->reqHistoricalData
						(0  //Note the reqId here corresponds exactly to the reqId in newtxtVector.
						, C
						, m_buf //EndDateTime. It seems use the PST by default if I don't add time zone.Although here requires string type, but also accept c_string buf.	
						, m_durationString //duration string.
						, m_barSize //BarSizeSetting. 
						, "TRADES" //whatToShow. Also include: MIDPOINT, BID, ASK, BID_ASK, etc. 
						, 1 //use RTH.
						, 2 // FormatDate is 1 for yyyymmdd  hh:mm:dd and 2 for long int.  ****However, not this seems to be true only for barduration less than a day. Beyond that, no longer long int. 
						);

					done = true;
					now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
					a.m_timeDeque.push_back(now);

					initialTime = time(0);
				}
				a.m_csTimeDeque.LeaveCriticalSection();
			}

			if (done == true)  break; else { Sleep(100); counter = counter + 1; }
			if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
		} //end of while(1)


		done = false;
		while (1)
		{
			finalTime = time(0);
			bool b = true;
			try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				if (m_historicalDataEndVector[0])
				{
					done = true; // m_historicalDataEndVector[0] has already been set to be false in the downloadHistoricalData_Common().
					SPY_Done = true;
					printf("SPY single symbol downloading is over.\n");
				}
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}

			if ((finalTime - initialTime) > (a.m_traderParameters.historicalDataWaitingTime + 20)) //**** For SPY wait an extra of 20 seconds. 
			{
				m_EC->cancelHistoricalData(0); done = true; //**** Here I have not received reasonable data but set to true in order to re-download SPY data. 
				printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Waiting too long for the historical data of SPY. Need figure out why before continue. Or replace SPY with a different very liquid symbol. The app is now sleeping. \n");
			}

			if (done == true) break; else Sleep(100);
		}

		Sleep(15000); //I need sleep at least 15 seconds to avoid pacing violation in downloading a single same symbol. I always need keep thi sentence here because after downloading SPY here I may download it again in next function.
		if (SPY_Done) break;
	} //end of while(!SPY_Done)

	a.m_historicalDataDone = true; //This will let the code flow continue in the start.cpp.
}

void  HistoricalDataNormal::downloadHistoricalData_multiple_beforeTrading()
{
	for (unsigned int J = 0; J < a.m_newtxtVector.size(); J++)
	{
		bool b = true;
		try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }
		catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_historicalDataEndVector[J] = false;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}
	}

	m_durationString.clear();
	downloadHistoricalData_Common(); //For downloading before trading, these function can be outside the while loop below. If downloading during trading, then it should be within the while loop.

	Contract			C;
	std::vector<Contract> m_ContractVector;

	for (unsigned int i = 0; i< a.m_newtxtVector.size(); i++)
	{
		C.symbol = a.m_newtxtVector[i];
		C.secType = "STK";
		C.currency = "USD";
		C.exchange = "SMART";
		m_ContractVector.push_back(C);
	}

	int extraNumBars;
	if (a.m_bardurationS == 86400) extraNumBars = 1; else extraNumBars = a.m_wholeDayTradeDuration / a.m_bardurationS; //**** For Long bars, there are no extra bars. But for short bars, I have yesterday's bars. 
	if (a.m_backTest == true) extraNumBars = 0; //***** Later, I should put this in the AllData class. 


	time_t now, lastDownloadTime, finalBarTime_SPY, initialTime, finalTime;
	std::string dateString = m_liquidStock[a.m_numDataOfSingleSymbolS - extraNumBars - 1], tempString; //[m_numDataOfSingleSymbolL-1] is the final element for trading day. [m_numDataOfSingleSymbolL-2] is the final SPY day bar traded before next trading session.

	if (a.m_bardurationS < 86400)
	{
		finalBarTime_SPY = atoi(m_liquidStock[a.m_numDataOfSingleSymbolS - extraNumBars - 1].c_str());
	}
	else if (a.m_bardurationS == 86400)
	{
		//**** When comparing the calculated epoch time to website ones, consider time zones and summer or winter time. 
		now = time(0);
		struct tm *ts = localtime(&now);

		if (dateString.size() == 8)
		{
			tempString.clear();
			for (int i = 0; i < 4; i++)	tempString.append(std::string(1, dateString.at(i)));
			ts->tm_year = atoi(tempString.c_str()) - 1900; //****Why here is 1900? Only 1900 is OK, otherwise it is not in agreement with the epoch time calcualtion. 

			tempString.clear();
			for (int i = 4; i < 6; i++)	tempString.append(std::string(1, dateString.at(i)));
			ts->tm_mon = atoi(tempString.c_str()) - 1;

			tempString.clear();
			for (int i = 6; i < 8; i++)	tempString.append(std::string(1, dateString.at(i)));
			ts->tm_mday = atoi(tempString.c_str());
			ts->tm_hour = 6; ts->tm_min = 30; ts->tm_sec = 0;//*****Here I set it as local trade starting time. Thus I just add 6.5 hours for trade duration.

			finalBarTime_SPY = mktime(ts);
		}
		else
		{
			printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Bar duration is day, but the bar date is not in the 8-digits format. Something is wrong.\n");
		}
	} //end of for day bar.

	//****Once I arrived here, it indicates I have verified the download of SPY is over and thus the data farm is connected. Thus I don't have to verify the connection.
	int numScanned = 0, numCalculatedFromDownloadedData = 0, numCalculatedFromDataBase = 0;
	for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++) //***** Although SPY has been downloaded, it has not been stored in m_VectorTemp. Therefore here we need re-download SPY (reqId starts from 0) 
	{
		numScanned = numScanned + 1;
		bool b = true;
		try{ m_csHistoricalDataEndVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_historicalDataEndVector[reqId] = false;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}


		b = true;
		try{ a.m_csVectorS.EnterCriticalSection(); }
		catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			lastDownloadTime = atoi(a.m_VectorS[0][reqId][0].lastDownloadTime.c_str()); //***** When preparing historical data after hours, lastDownloadTime is always zero and thus we always need request historical data for all symbols. 
			a.m_csVectorS.LeaveCriticalSection();
		}

		if (lastDownloadTime > (finalBarTime_SPY + a.m_wholeDayTradeDuration)) //indicating the m_VectorTemp is most updated.
		{
			numCalculatedFromDataBase = numCalculatedFromDataBase + 1;
			printf("The algoVector number calculated from existing data is %d.\n", numCalculatedFromDataBase);
			printf("The algoVector number calculated from existing data is %d.\n", numCalculatedFromDataBase);
		}
		else //the historical data vector m_VectorTemp need re-download because it is not most updated.
		{
			std::chrono::high_resolution_clock::time_point now;
			int counter = 0;
			bool done = false;
			while (1)
			{
				bool b = true;
				try{ a.m_csTimeDeque.EnterCriticalSection(); }
				catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
					{

						m_EC->reqHistoricalData
							(reqId  //Note the reqId here corresponds exactly to the reqId in newtxtVector.
							, m_ContractVector[reqId]
							, m_buf //EndDateTime. It seems use the PST by default if I don't add time zone.Although here requires string type, but also accept c_string buf.	
							, m_durationString //duration string.
							, m_barSize //BarSizeSetting. 
							, "TRADES" //whatToShow. Also include: MIDPOINT, BID, ASK, BID_ASK, etc. 
							, 1 //use RTH.
							, 2 // FormatDate is 1 for yyyymmdd  hh:mm:dd and 2 for long int. ****However, not this seems to be true only for barduration less than a day. Beyond that, no longer long int. 
							);

						done = true;

						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);
						numCalculatedFromDownloadedData = numCalculatedFromDownloadedData + 1;

						initialTime = time(0);
					}

					a.m_csTimeDeque.LeaveCriticalSection();
				}//end of try-catch

				if (done == true) break; else { Sleep(100); counter = counter + 1; } //**** break means historicalData has already been requested
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)

			done = false;
			while (1)
			{
				finalTime = time(0);
				bool b = true;
				try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (m_historicalDataEndVector[reqId])
					{
						done = true;
						//calculateAlgoStructureVector(reqId); 
						printf("The algoVector number calculated from new download  is %d.\n", numCalculatedFromDownloadedData);
					}
					m_csHistoricalDataEndVector.LeaveCriticalSection();
				}

				if ((finalTime - initialTime) > a.m_traderParameters.historicalDataWaitingTime)
				{
					bool b = true;
					try{ m_csHistoricalDataEndVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
					{
						//m_EC->cancelHistoricalData(reqId); //**** This is not useful but also cause ErrorCode 366 error. 
						m_historicalDataEndVector[reqId] = true; //**** No waiting for the data of this sysmbol. Just use the default.
						done = true; //*****This is important. For example, if this is true, then anyway I will break out, whether the algoVector has been calculated or not. 
						m_csHistoricalDataEndVector.LeaveCriticalSection();
					}
					printf("***CS*** %s %d\n", __FILE__, __LINE__); printf("Waiting too long for the historical data of this symbol %s. Set flag = true and break out. \n", a.m_newtxtVector[reqId].c_str());
					break;
				}

				if (done == true)
					break;
				else
					Sleep(100); //**** break means historicalData has already been downloaded.
			}


			b = true;
			try{ a.m_csVectorS.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				std::stringstream ss;
				ss << time(0);
				a.m_VectorS[0][reqId][0].lastDownloadTime = ss.str();

				a.m_csVectorS.LeaveCriticalSection();
			}

			//printf("Download historical for L-type bars. The number of downloaded symbols is %d.\n", numScanned);

		} //end of else, for the case of re-downloading 

	} //end of for loop.


	if (remove(a.m_vectorFileName_S_Test.c_str()) != 0)
	{
		printf("Error deleting file %s.\n", a.m_vectorFileName_S_Test.c_str());
	}
	else
	{
		printf("File %s successfully deleted", a.m_vectorFileName_S_Test.c_str());
	}

	std::ofstream o_new(a.m_vectorFileName_S_Test.c_str(), std::ios::app);
	bool b = true;
	try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			for (unsigned int j = 0; j < a.m_VectorS[0][i].size(); j++) //numDataOfSingleSymbol Actually this also include an extra element for today's bar, not just the number of historical data downloaded.
			{
				o_new << a.m_VectorS[0][i][j].date << ',' << a.m_VectorS[0][i][j].yearMonthDateString << ',' << a.m_VectorS[0][i][j].open << ',' << a.m_VectorS[0][i][j].high << ',' << a.m_VectorS[0][i][j].low << ',' << a.m_VectorS[0][i][j].close << ',' << a.m_VectorS[0][i][j].volume << ','
					<< a.m_VectorS[0][i][j].lastDownloadTime << ',' << a.m_VectorS[0][i][j].symbol << std::endl;
			}
		}
		a.m_csVectorS.LeaveCriticalSection();
	}
	o_new.close();

	a.m_historicalDataDone = true; //**** Whenever being here, the downloading of historial data of either ONE or multiple symbols is done. 
	//m_EC->eDisconnect(); //**** I repeatedly download, and thus cannot disconnect it as in the L-bar case. 

}


void HistoricalDataNormal::historicalData(TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps)
{
	if (a.m_numSymbolsToDownload == 1)
		historicalData_Single(reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps);
	else 
		historicalData_multiple_beforeTrading(reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps); //Only handle bars before today. For today's bar construction from, say, 1-min bars, check order version. 
}

void HistoricalDataNormal::historicalData_Single(TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps) //Single symbol download. Passed all by reference.
{
	if (IsEndOfHistoricalData(date) == false)
	{
		m_liquidStock.push_back(date);
	}
	else // The download is over for a specific symbol
	{
		int extraNumBars;
		if (a.m_bardurationS == 86400) extraNumBars = 1; else extraNumBars = a.m_wholeDayTradeDuration / a.m_bardurationS;
		if (a.m_backTest == true) extraNumBars = 0; //***** Later, I should put this in the AllData class. 
		a.m_numDataOfSingleSymbolS = m_liquidStock.size() + extraNumBars;

		bool b = true;
		try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_historicalDataEndVector[reqId] = true;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}

		time_t now;
		struct tm *ts;
		std::string tempString;
		ts = localtime(&now);

		now = time(0); char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		printf("At %s, downloading of small bars is over. \n", tempString.c_str());

	} // End of The download is over for a specific symbol
}

void HistoricalDataNormal::historicalData_multiple_beforeTrading(TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps) //Multiple symbol download. Passed all by reference.
{
	//***** Keep these important comments about historical data
	//***** [1] IsEndOfHistoricalData(date) function below is not from IB and sometimes might be not OK. In that case I may need study the fedback data in details to meet my specific needs. [2] If I download dayBar from a point during trading session, then I got the data from today's opening up to now, although in the "finished" fed-back string it is said the bar is from yesterday to today. 
	//******[3] However, if I download 1-min bars under the same condition, then it might contain bars of both days? 
	if (IsEndOfHistoricalData(date) == true)
	{
		bool b = true;
		try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_historicalDataEndVector[reqId] = true;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}

	}
	else //**** origimal code was without this "else" and the following is in parallel with the if branch above. Thus even at the end of historical data download, the following code is still run one and thus cause vector out of range (index = -1)
	{
		if ((unsigned)reqId <  a.m_newtxtVector.size())
		{
			int index = -1;
			for (unsigned int i = 0; i < m_liquidStock.size(); i++) //****If historical data arrives in strict order, we don't have to search the whole array for a single index. However, for absolutely sure, here I always search the whole array. 
			{
				if (m_liquidStock[i] == date)
				{
					index = i;
					break;
				}
			}

			if (index >= 0) //indicating the date does not belong to any dates of SPY, and something must be wrong. 
			{
				std::string symbolTemp; //Introduce this variable to avoid nested critical section.
				symbolTemp = a.m_newtxtVector[reqId]; //***** can use newtxt instead without 

				bool b = true;
				try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (index >= 0 && index < a.m_VectorS[0][reqId].size())
					{
						//time_t timeDate = std::stol(date);
						std::time_t timeDate = std::stol(date); //**** this time_t is not time_t defined elsewhere. 
						std::tm *ts = std::localtime(&timeDate); 
						//tm *ts = localtime(&timeDate);
						std::string yearString = std::to_string(ts->tm_year + 1900), monthString = std::to_string(ts->tm_mon + 1), dateString = std::to_string(ts->tm_mday);
						if (monthString.length() == 1) monthString.insert(0, "0"); if (dateString.length() == 1) dateString.insert(0, "0");
						std::string yearMonthDateString = yearString + monthString + dateString;

						a.m_VectorS[0][reqId][index].date = date;
						a.m_VectorS[0][reqId][index].yearMonthDateString = yearMonthDateString;
						a.m_VectorS[0][reqId][index].open = open;
						a.m_VectorS[0][reqId][index].high = high;
						a.m_VectorS[0][reqId][index].low = low;
						a.m_VectorS[0][reqId][index].close = close;
						a.m_VectorS[0][reqId][index].volume = volume;
						a.m_VectorS[0][reqId][index].symbol = symbolTemp;
					}

					a.m_csVectorS.LeaveCriticalSection();
				}
			} //end of index != -1
		} // End of if(reqId <  a.m_newtxtVector.size() )
	}// end of if(IsEndOfHistoricalData(date) == true) 
}

void  HistoricalDataNormal::downloadHistoricalData_multiple_duringTrading()
{
	bool b = true;
	try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		m_numSmallBarsOfDay = a.m_wholeDayTradeDuration / a.m_bardurationS;
		a.m_barIndexShift = a.m_VectorS[0][0].size() - m_numSmallBarsOfDay; //**** Note that m_wholeDayTradeDuration is already in units of seconds, same as m_bardurationS.
		a.m_csVectorS.LeaveCriticalSection();
	}

	int timeToSleep = 10 * 60 * 1000; //in units if miliseconds.
	printf(" I am sleepting %d mili-seconds to avoid pacing error.\n", timeToSleep);
	Sleep(timeToSleep); //To avoid pacing error. 
	//int minutes = 60000 / 60000;
	//printf(" I am sleepting %d minute to avoid pacing error.\n", minutes);


	Contract			C;
	std::vector<Contract> m_ContractVector;
	for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
	{
		C.symbol = a.m_newtxtVector[i];
		C.secType = "STK";
		C.currency = "USD";
		C.exchange = "SMART";
		m_ContractVector.push_back(C);
	}

	int numDownloaded = 0;
	while (1) //***** This downloading will be done repeatedly. This is particularly important becaues IB's RTVolume is also sampled. So re-downloading can always help to make the bars more accurate.
	{
		m_durationString.clear();
		downloadHistoricalData_Common(); //For downloading before trading, these function can be outside the while loop below. If downloading during trading, then it should be within the while loop.
		//****If the above sentence is outside the while loop, then I will have the problem of no-end for the historical data for a specific reqId. 

		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++) //***** Although SPY has been downloaded, it has not been stored in m_VectorTemp. Therefore here we need re-download SPY (reqId starts from 0) 
		{
			bool b = true;
			try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }
			catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				m_historicalDataEndVector[reqId] = false;
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}

			time_t initialTime, finalTime;
			std::chrono::high_resolution_clock::time_point now;
			int counter = 0;
			bool done = false;
			while (1)
			{
				bool b = true;
				try{ a.m_csTimeDeque.EnterCriticalSection(); }
				catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
					{
						m_EC->reqHistoricalData
							(reqId  //Note the reqId here corresponds exactly to the reqId in newtxtVector.
							, m_ContractVector[reqId]
							, m_buf //EndDateTime. It seems use the PST by default if I don't add time zone.Although here requires string type, but also accept c_string buf.	
							, m_durationString //duration string.
							, m_barSize //BarSizeSetting. 
							, "TRADES" //whatToShow. Also include: MIDPOINT, BID, ASK, BID_ASK, etc. 
							, 1 //use RTH.
							, 2 // FormatDate is 1 for yyyymmdd  hh:mm:dd and 2 for long int. ****However, not this seems to be true only for barduration less than a day. Beyond that, no longer long int. 
							);

						done = true;

						now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
						a.m_timeDeque.push_back(now);

						initialTime = time(0);
					}

					a.m_csTimeDeque.LeaveCriticalSection();
				}//end of try-catch

				if (done == true) break; else { Sleep(100); counter = counter + 1; } //**** break means historicalData has already been requested
				if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) { printf("***CS*** %s %d\n", __FILE__, __LINE__); }
			} //end of while(1)


			done = false;
			while (1)
			{
				finalTime = time(0);
				bool b = true;
				try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }
				catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (m_historicalDataEndVector[reqId])
					{
						done = true;
					}
					m_csHistoricalDataEndVector.LeaveCriticalSection();
				}

				if ((finalTime - initialTime) > a.m_traderParameters.historicalDataWaitingTime) //**** If internet is disconnected, then hisClientActive will be false and we cannot arrive here. 
				{
					bool b = true;
					try{ m_csHistoricalDataEndVector.EnterCriticalSection(); }
					catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
					{
						//m_EC->cancelHistoricalData(reqId); 
						m_historicalDataEndVector[reqId] = true; //**** No waiting for the data of this sysmbol. Just use the default.
						done = true; //*****This is important. For example, if this is true, then anyway I will break out, whether the algoVector has been calculated or not. 
						m_csHistoricalDataEndVector.LeaveCriticalSection();
					}
					//printf("***CS*** %s %d\n",__FILE__,__LINE__); 
					printf("Waiting too long symbol %s. Set flag = true and break out. \n", a.m_newtxtVector[reqId].c_str());
					break;
				}

				if (done == true) break; else Sleep(100); //**** break means historicalData has already been downloaded.
			}

			numDownloaded = numDownloaded + 1;

		} //end of for loop.

		//Sleep(600000); printf("I am Sleeping around %d miliseconds to avoid historical data download pacing violation.\n", m_traderParameters.timeToAvoidPacingError);

	}//end of while(1) loop.

	a.m_historicalDataDone = true; //**** In S-bar case, I did not use this variable at all. 

}

void HistoricalDataNormal::historicalData_multiple_duringTrading(TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps) //Multiple symbol download. Passed all by reference.
{
	//***** If download 1-min bars for a duration of 1 day, then the first bar start from today's session, but not include yesterday's bars. 2000 bars are maximum but not always 2000 bars. 
	My_BarDataNormal barData;
	if(IsEndOfHistoricalData(date) == true) 
	{
		bool b = true;
		try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			m_historicalDataEndVector[reqId] = true;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}
	}
	else //This else is very important. Whenever the download is over, the following code will not be run again. Without this else, it will be run once even download is over. 
	{
		unsigned int dayIndex = (atoi(date.c_str()) - a.m_initime) / a.m_bardurationL;
		unsigned int index;

		if (dayIndex >= 0 && dayIndex < m_numSmallBarsOfDay) //indicating the date does not belong to any dates of SPY, and something must be wrong. 
		{
			bool b = true;
			try{ a.m_csVectorS.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				index = a.m_barIndexShift + dayIndex; //***** a.m_barIndexShift is also protected by a.m_cs_VectorS. 
				if (index >= 0 && index < a.m_VectorS[0][reqId].size())
				{
					//*****Never set currentShareSum and currentMoneySum in historical downloading process because here is exact while in datafeed is sampled. Also I should completely separate the downloading process here and the bar construction process elsewhere.
					a.m_VectorS[0][reqId][index].date = date;
					a.m_VectorS[0][reqId][index].open = open;
					a.m_VectorS[0][reqId][index].high = high;
					a.m_VectorS[0][reqId][index].low = low;
					a.m_VectorS[0][reqId][index].close = close;
					a.m_VectorS[0][reqId][index].volume = volume;
				}
				a.m_csVectorS.LeaveCriticalSection();
			}
		}
	}
}

