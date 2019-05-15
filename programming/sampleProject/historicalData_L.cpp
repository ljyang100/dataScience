#include "historicalData_L.h"
void HistoricalDataL::error( const int id, const int errorCode, const IBString errorString )
{
	if (errorCode == 200) //meaning "No security defintion for", 
	{
		if ( id >= 0 && id < (const int) a.m_newtxtVector.size()) 		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
		{
			printf("No security defintion for symbol %s for historical data download. The corresponding flag is set to be true for continue. \n", a.m_newtxtVector[id].c_str() );
		
			bool b = true;
			try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
			{
				m_historicalDataEndVector[id] = true; //**** After setting this to be true the historical data of the sysmbol will use its default. The application will act according to this default. 
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}
		}
		else
		{
			printf("***CS*** %s %d\n",__FILE__,__LINE__); 
			printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString );
		}
	}
	else if (errorCode == 203) //meaning the security is not available for this account.
	{
		if ( id >= 0 && id < (const int) a.m_newtxtVector.size()) 		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
		{
			printf("***CS*** %s %d\n",__FILE__,__LINE__);	
			printf("The security %s is not available for this account. The corresponding flag is set to be true for continue.\n", a.m_newtxtVector[id].c_str() );
	
			bool b = true;
			try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
			{
				m_historicalDataEndVector[id] = true; //**** After setting this to be true the historical data of the sysmbol will use its default. The application will act according to this default. 
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}
		}
		else
		{
			printf("***CS*** %s %d\n",__FILE__,__LINE__); 
			printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString );
		}
	}
	else
	{
		if ( id >= 0 && id < a.m_newtxtVector.size()) printf("historicalData: Error for id=%d: %d = %s. For symbol %s. \n", id, errorCode, (const char*)errorString, a.m_newtxtVector[id].c_str() );
		else printf("historicalData: Error for id=%d: %d = %s. \n", id, errorCode, (const char*)errorString);
		//***** if without the if condition above, sometimes internet disconnection caused error such as farm disconnected will feed back id = -1 and thus cause vector out of range, which is a disaster and will crash the system if internet is disrrupted.
	}
}


void  HistoricalDataL::connectToTWS()
{
	m_EC = EClientL0::New(this);
	if (m_EC == NULL) { printf("Creating EClient failed.\n"); 	getchar(); 	exit(0); } //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
	bool returnValue = false;
	std::chrono::high_resolution_clock::time_point now;
	while (1)
	{
		bool b = true;
		try{ a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond)
			{
				returnValue = m_EC->eConnect("", a.m_port, a.m_clientId_HistoricalData_L); //***** This function starts a new thread automatically. Therefore, this function can never be in another threadEntryPoint function. I once made very big mistake and wastes my many hours.	
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}
		printf(" socket client is being created in historicalData_L.\n");

		if (returnValue == true) break; else { Sleep(20000); }
	} //end of while(1)

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

void  HistoricalDataL::scannerParameters(const IBString &xml)
{
	int a = 1;
}

void  HistoricalDataL::scannerData(int reqId, int rank, const ContractDetails &contractDetails, const IBString &distance, const IBString &benchmark, const IBString &projection, const IBString &legsStr)
{
	int a = 1;
	tempVector.push_back(contractDetails.summary.symbol);
}
void  HistoricalDataL::scannerDataEnd(int reqId)
{
	int a = 1;
}

void  HistoricalDataL::downloadHistoricalData_Common()
{
	////m_EC->reqScannerParameters();
	////Sleep(1000000);
	//ScannerSubscription scanSub;
	//scanSub.instrument = "STK";
	//scanSub.locationCode = "STK.US.MAJOR";   //"STK.US";  //"STK.US.MAJOR"; // STK.US.MAJOR";
	//scanSub.scanCode = "HIGH_OPEN_GAP"; //"HIGH_LAST_VS_EMA20"; // "LOW_OPEN_GAP"; // "TOP_PERC_GAIN"; //"HIGH_OPEN_GAP"; 
	//scanSub.stockTypeFilter = "ALL";
	//scanSub.abovePrice = 2.0;
	//scanSub.belowPrice = 150;
	//scanSub.numberOfRows = 50;
	//m_EC->reqScannerSubscription(100, scanSub);

	//ScannerSubscription scanSub1;
	//scanSub1.instrument = "STK";
	//scanSub1.locationCode = "STK.US.MAJOR";   //"STK.US";  //"STK.US.MAJOR"; // STK.US.MAJOR";
	//scanSub1.scanCode = "AVGVOLUME_USD"; //"HIGH_LAST_VS_EMA20"; // "LOW_OPEN_GAP"; // "TOP_PERC_GAIN"; //"HIGH_OPEN_GAP"; 
	//scanSub1.stockTypeFilter = "ALL";
	//scanSub1.abovePrice = 2.0;
	//scanSub1.belowPrice = 150;
	//scanSub1.numberOfRows = 300;
	//m_EC->reqScannerSubscription(200, scanSub);

	//Sleep(500000);

	struct tm *ts = localtime(&a.m_timeNow);
	strftime(m_buf, sizeof(m_buf), "%Y%m%d %H:%M:%S", ts);  
	std::stringstream ss;
	if (a.m_bardurationL >= 60)
	{
		//Note in this structure I have only included 1D case, and those mins bar. Week, Month bars are not included. 
		if (a.m_bardurationL / 86400 == 1) { m_barSize = "";	ss.str(""); ss << (a.m_bardurationL / 86400); m_barSize.append(ss.str()); m_barSize.append(" "); }
		else { m_barSize = "";	ss.str(""); ss << (a.m_bardurationL / 60); m_barSize.append(ss.str()); m_barSize.append(" "); }

		if (a.m_bardurationL / 60 == 1) 	m_barSize.append("min");
		else { if (a.m_bardurationL / 86400 == 1) m_barSize.append("day"); else m_barSize.append("mins"); } //Note in this structure I have only included 1D case, and those mins bar. Week, Month bars are not included. 
	}	
	else 
	{
		m_barSize = "";
		ss.str(""); ss << (a.m_bardurationL); m_barSize.append(ss.str()); m_barSize.append(" ");
		m_barSize.append("secs");
	}
	
	std::stringstream ss1;
	if (a.m_daysOfHistoricalDataL <= 2)
	{
		ss1.str(""); ss1 << a.m_daysOfHistoricalDataL; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("D");
	}
	else if ((a.m_daysOfHistoricalDataL >2) && (a.m_daysOfHistoricalDataL < 30)) //m_daysOfHistoricalDataL are in units of 7 days (week)
	{
		int ssTemp = a.m_daysOfHistoricalDataL / 7;
		ss1.str(""); ss1<<ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("W");
	}
	else if ((a.m_daysOfHistoricalDataL >= 30) && (a.m_daysOfHistoricalDataL < 360)) //m_daysOfHistoricalDataL are in units of 30 days (month)
	{
		int ssTemp = a.m_daysOfHistoricalDataL / 30;
		ss1.str(""); ss1<<ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("M");
	}
	else if (a.m_daysOfHistoricalDataL >= 360) //m_daysOfHistoricalDataL are in units of 360 days (Year)
	{
		int ssTemp = a.m_daysOfHistoricalDataL / 360;
		ss1.str(""); ss1<<ssTemp; m_durationString.append(ss1.str()); m_durationString.append(" ");  m_durationString.append("Y");
	}
	
} 

void  HistoricalDataL::downloadHistoricalData_Single()
{
	m_durationString.clear();
	downloadHistoricalData_Common();

	Contract			C;
	C.symbol			= a.m_newtxtVector[0]; //Normally this is the symbol "SPY".
	C.secType			= "STK";
	C.currency			= "USD";
	C.exchange			= "SMART";

	bool SPY_Done = false; //**** Sometimes the application is started before trading and database farm is not connected. Thus I need wait until it is done. Otherwise I cannot continue without the guidence of SPY dates.
	while(!SPY_Done)
	{
		time_t initialTime, finalTime;

		m_liquidStock.clear(); //**** Be careful emptify the vector before each trial downloading. 
		bool done = false;
		int counter = 0;
		while(1) //This makes sure that the relevant request must be sent out, even I have to wait for a while. 
		{
			std::chrono::high_resolution_clock::time_point now; 
			bool b = true;
			try{ a.m_csTimeDeque. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
			{
				if(a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
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
				a.m_csTimeDeque. LeaveCriticalSection(); 
			}

			if (done == true)  break; else {Sleep(100); counter = counter + 1;} 
			if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) {  printf("***CS*** %s %d\n",__FILE__,__LINE__); }
		} //end of while(1)

	
		done = false;
		while(1)
		{
			finalTime = time(0);
			bool b=true; 
			try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
			{
				if( m_historicalDataEndVector[0]) 
				{
					done = true; // m_historicalDataEndVector[0] has already been set to be false in the downloadHistoricalData_Common().
					SPY_Done = true;
					printf("SPY single symbol downloading is over.\n");
				}
				m_csHistoricalDataEndVector.LeaveCriticalSection();
			}

			if ( (finalTime - initialTime) > ( a.m_traderParameters.historicalDataWaitingTime + 20)) //**** For SPY wait an extra of 20 seconds. 
			{
				m_EC->cancelHistoricalData(0); done = true; //**** Here I have not received reasonable data but set to true in order to re-download SPY data. 
				printf("***CS*** %s %d\n",__FILE__,__LINE__); printf("Waiting too long for the historical data of SPY. Need figure out why before continue. Or replace SPY with a different very liquid symbol. The app is now sleeping. \n"); 
			}

			if (done == true) break; else Sleep(100); 
		}

		Sleep(16000); //I need sleep at least 15 seconds to avoid pacing violation in downloading a single same symbol. I always need keep thi sentence here because after downloading SPY here I may download it again in next function.
		if (SPY_Done) break;
	} //end of while(!SPY_Done)

	a.m_historicalDataDone = true; //This will let the code flow continue in the start.cpp.
}

void  HistoricalDataL::downloadHistoricalData_Multiple_L()
{
	m_durationString.clear();
	downloadHistoricalData_Common();

	Contract			C;
	std::vector<Contract> m_ContractVector; 
	
	for (unsigned int i=0; i< a.m_numSymbolsToDownload; i++) 
	{
		C.symbol			=  a.m_newtxtVector[i];
		C.secType			= "STK";
		C.currency			= "USD";
		C.exchange			= "SMART";
		m_ContractVector.push_back(C);
	}

	time_t lastDownloadTime, finalBarTime_SPY, initialTime, finalTime,tradingDuration = 6.5*60*60; 
	std::string dateString = m_liquidStock[a.m_numDataOfSingleSymbolL-1]; // Here I use index-1, rather than index-2 because I don't have the final bar storing today's bar.

	if (a.m_bardurationL < 86400)
	{
		finalBarTime_SPY = atoi(m_liquidStock[a.m_numDataOfSingleSymbolL-1].c_str()); 
	}
	else if (a.m_bardurationL == 86400)
	{
		//**** When comparing the calculated epoch time to website ones, consider time zones and summer or winter time. 
		std::string  tempString;
		time_t now = time(0);
		struct tm *ts = localtime(&now);  
 
		if ( dateString.size() == 8)
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
			
			finalBarTime_SPY = mktime (ts);
		}
		else
		{
			 printf("***CS*** %s %d\n",__FILE__,__LINE__); printf("Bar duration is day, but the bar date is not in the 8-digits format. Something is wrong.\n"); 
		}
	} //end of for day bar.

	//****Once I arrived here, it indicates I have verified the download of SPY is over and thus the data farm is connected. Thus I don't have to verify the connection.
	int numScanned = 0, numCalculatedFromDownloadedData = 0, numCalculatedFromDataBase = 0;
	for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++) //***** Although SPY has been downloaded, it has not been stored in m_VectorTemp. Therefore here we need re-download SPY (reqId starts from 0) 
	{
		numScanned =  numScanned + 1;
		bool b = true;
		try{m_csHistoricalDataEndVector.EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			m_historicalDataEndVector[reqId] = false; 
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}

		b=true; 
		try{ a. m_csVectorL. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			lastDownloadTime = atoi(a.m_VectorL[reqId][0].lastDownloadTime.c_str()); 
			a. m_csVectorL.LeaveCriticalSection();
		}

		if (lastDownloadTime > (finalBarTime_SPY + tradingDuration)) // finalBarTime is 6:30 of the last trading day. So if we download historical data after that time plus tradingDuration, then the bar must be most updated. 
		{
			numCalculatedFromDataBase = numCalculatedFromDataBase + 1;
			printf("The algoVector number calculated from existing data is %d.\n", numCalculatedFromDataBase);
		}
		else //the historical data vector m_VectorTemp need re-download because it is not most updated.
		{
			std::chrono::high_resolution_clock::time_point now;
			int counter = 0;
			bool done = false;
			while(1)
			{
				bool b = true;
				try{ a.m_csTimeDeque. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
				{
					if(a.m_timeDeque.size() < a.m_messageLimitPerSecond)  // Need a variable even thought currently it is always 50. Otherwise, hard-coding in various places is dangerous because will forget this later. 
					{
						m_EC->reqHistoricalData
						( reqId  //Note the reqId here corresponds exactly to the reqId in newtxtVector.
						, m_ContractVector[reqId]
						, m_buf //EndDateTime. It seems use the PST by default if I don't add time zone.Although here requires string type, but also accept c_string buf.	
						, m_durationString //duration string.
						, m_barSize //BarSizeSetting. 
						, "TRADES" //whatToShow. Also include: MIDPOINT, BID, ASK, BID_ASK, etc. 
						, 1 //use RTH.
						, 2 // FormatDate is 1 for yyyymmdd  hh:mm:dd and 2 for long int. ****However, not this seems to be true only for barduration less than a day. Beyond that, no longer long int. 
						);

						done = true;

						now = std::chrono::high_resolution_clock::now();
						a.m_timeDeque.push_back(now);
						numCalculatedFromDownloadedData = numCalculatedFromDownloadedData + 1;

						initialTime = time(0);
					}

					a.m_csTimeDeque. LeaveCriticalSection(); 
				}//end of try-catch

				if (done == true) break; else { Sleep(100); counter = counter + 1;} //**** break means historicalData has already been requested
				//if (counter > a.m_traderParameters.maxTimeToWaitSendingMessage) {printf("***CS*** %s %d\n",__FILE__,__LINE__); }
			} //end of while(1)

			done = false;
			while(1)
			{
				finalTime = time(0);
				bool b=true; 
				try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
				{
					if( m_historicalDataEndVector[reqId])
					{
						done = true;
						printf("The algoVector number calculated from new download  is %d.\n", numCalculatedFromDownloadedData);
					}
					m_csHistoricalDataEndVector.LeaveCriticalSection();
				}

				if ( (finalTime - initialTime) > a.m_traderParameters.historicalDataWaitingTime) 
				{
					bool b = true;
					try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
					{
						//m_EC->cancelHistoricalData(reqId); //**** This is not useful but also cause ErrorCode 366 error. 
						m_historicalDataEndVector[reqId] = true; //**** No waiting for the data of this sysmbol. Just use the default.
						done = true; //*****This is important. For example, if this is true, then anyway I will break out, whether the algoVector has been calculated or not. 
						m_csHistoricalDataEndVector.LeaveCriticalSection();
					}
					printf("***CS*** %s %d\n",__FILE__,__LINE__); printf("Waiting too long for the historical data of this symbol %s. Set flag = true and break out. \n", a.m_newtxtVector[reqId].c_str()); 
					break;
				}

				if (done == true) 
					break; 
				else 
					Sleep(100); //**** break means historicalData has already been downloaded.
			}

			b=true; 
			try{ a. m_csVectorL. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
			{
				std::stringstream ss;
				ss << time(0);
				a.m_VectorL[reqId][0].lastDownloadTime = ss.str();
				a. m_csVectorL.LeaveCriticalSection();
			}

		} //end of else, for the case of re-downloading 

	} //end of for loop.

	time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
	std::string fileName;
	if (a.m_backTest == true) fileName = a.m_vectorFileName_S_Test; else fileName = a.m_vectorL_NEW; //*****Be very careful about here! 

	if (remove(fileName.c_str()) != 0)
	{
		printf("Error deleting file %s.\n", fileName.c_str());
	}
	else
	{
		printf("File %s successfully deleted", fileName.c_str());
	}

	std::ofstream o_new(fileName.c_str(), std::ios::app);
	bool b = true;
	try{ a. m_csVectorL.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		for (unsigned int i = 0; i < a.m_newtxtVector.size(); i++)
		{
			for (unsigned int j = 0; j < a.m_VectorL[i].size(); j++) //numDataOfSingleSymbol Actually this also include an extra element for today's bar, not just the number of historical data downloaded.
			{
				o_new << a.m_VectorL[i][j].date << ',' << a.m_VectorL[i][j].yearMonthDateString << ',' << a.m_VectorL[i][j].open << ',' << a.m_VectorL[i][j].high << ',' << a.m_VectorL[i][j].low << ',' << a.m_VectorL[i][j].close << ',' << a.m_VectorL[i][j].volume << ','
					<< a.m_VectorL[i][j].lastDownloadTime << ',' << a.m_VectorL[i][j].symbol << std::endl;
			}
		}
		a. m_csVectorL.LeaveCriticalSection();
	}

	o_new.close();

	printf("I am sleeping after every 50 symbols downloaded in L-bar historical data downloading.\n");

	a.m_historicalDataDone = true; //**** Whenever being here, the downloading of historial data of either ONE or multiple symbols is done. 
	m_EC->eDisconnect(); //**** Once L-bar is downloaded, it will never necessary to download within this session. So I disconnect the client. 

}

void HistoricalDataL::historicalData( TickerId reqId, const IBString& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps )
{
	if (a.m_numSymbolsToDownload == 1)
		historicalData_Single( reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps );
	else 
		historicalData_M_notToday_L( reqId, date, open, high, low, close, volume, barCount, WAP, hasGaps ); //Only handle bars before today. For today's bar construction from, say, 1-min bars, check order version. 
}

void HistoricalDataL::historicalData_Single(  TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps  ) //Single symbol download. Passed all by reference.
{
	if(IsEndOfHistoricalData(date) == false)
	{
		m_liquidStock.push_back(date);
	}
 	else // The download is over for a specific symbol
	{
		a.m_numDataOfSingleSymbolL = m_liquidStock.size(); //**** Note this is different from the OTC case, where an extra bar is added. However, in the short bars, there are extra bars. 
		bool b = true;
		try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			m_historicalDataEndVector[reqId] = true;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}

		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		printf("At %s, the download of SPY data is over. \n", tempString.c_str());

	} // End of The download is over for a specific symbol
}

void HistoricalDataL::historicalData_M_notToday_L( TickerId &reqId, const IBString& date, double &open, double &high, double &low, double &close, int &volume, int &barCount, double &WAP, int &hasGaps ) //Multiple symbol download. Passed all by reference.
{
	//***** Keep these important comments about historical data
	//***** [1] IsEndOfHistoricalData(date) function below is not from IB and sometimes might be not OK. In that case I may need study the fedback data in details to meet my specific needs. 
	//***** [2] If I download dayBar from a point during trading session, then I got the data from today's opening up to now, although in the "finished" fed-back string it is said the bar is from yesterday to today. 
	//******[3] However, if I download 1-min bars under the same condition, then it might contain bars of both days? 
	if(IsEndOfHistoricalData(date) == true) 
	{
		bool b = true;
		try{m_csHistoricalDataEndVector. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
		{
			m_historicalDataEndVector[reqId] = true;
			m_csHistoricalDataEndVector.LeaveCriticalSection();
		}

	}
	else //**** origimal code was without this "else" and the following is in parallel with the if branch above. Thus even at the end of historical data download, the following code is still run one and thus cause vector out of range (index = -1)
	{
		if((unsigned int)reqId <  a.m_newtxtVector.size() ) 
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

			if (index == -1) //indicating the date does not belong to any dates of SPY, and something must be wrong. 
			{
				//***** Sometimes I download SPY for 6 Month and got data from November 25, 2013. But for the same duration, I got data for other symbol from November 18,2013. Thus the earlier data of course does not belong to the time set of SPY. However, for long period, this can be neglectic.
				//printf("HistoricalData.cpp:: One downloaded data has no room in the m_Vector. The relevant reqId = %d.\n", reqId); 
			}
			else //indicating my_barData is initialized with reasonable data.
			{
				std::string symbolTemp; //Introduce this variable to avoid nested critical section.
				symbolTemp = a.m_newtxtVector[reqId]; //***** can use newtxt instead without 
				bool b=true; 
				try{ a. m_csVectorL. EnterCriticalSection();} catch(...) { printf("***CS*** %s %d\n",__FILE__,__LINE__);  b = false; } if ( b ) 
				{
					//**** Here I only need update part of elements of m_Vector
					std::stringstream ss;
					ss << time(0);

					
					a.m_VectorL[reqId][index].date = date; 
					a.m_VectorL[reqId][index].yearMonthDateString = date;
					a.m_VectorL[reqId][index].open = open;  
					a.m_VectorL[reqId][index].high = high;  
					a.m_VectorL[reqId][index].low = low;  
					a.m_VectorL[reqId][index].close = close; 
					a.m_VectorL[reqId][index].volume = volume; 
					a.m_VectorL[reqId][index].lastDownloadTime = ss.str(); 
					a.m_VectorL[reqId][index].symbol = symbolTemp; 

					a. m_csVectorL. LeaveCriticalSection(); 
				}

			} //end of index != -1

		} // End of if(reqId <  a.m_newtxtVector.size() )
	}// end of if(IsEndOfHistoricalData(date) == true) 
}

