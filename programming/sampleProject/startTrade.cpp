#include "startTrade.h"

void StartTrade::StartEntryPoint()
{
	AllData a;  
	a.initializeTraderParameters();  a.initializeOtherParameters();
	a.initializeSymbolVectors_Prev();
	a.m_backTest = false; //**** IMPORTANT! This must be false all the time for real trading.
	a.initializeSymbolVectors(); 
	a.initializeBaseVector("EMPTY"); a.initializeCurrentDataVector();  a.initializeOtherVectors();
	a.unhandledSymbols(); //**** I need run once here, and then run repetitively after. 

	int err = _beginthread(AllData::MessageLimitationStatic, 0, &a);  if (err < 0) { printf("Could not start new threads.\n"); } // This is for handling 50 message limitation. 
	
	CommonFunctions cfs(a);
	cfs.initialization();
	
	HistoricalDataL historicalDataL(cfs, a);
	historicalDataL.connectToTWS();
	HistoricalDataNormal historicalDataS(cfs, a);
	historicalDataS.connectToTWS();


	if (time(0) > a.m_tradingEndTime){//Preparing data for tomorrow. When runing this, must make sure SPY dayBar for today is already available. The application will complain if this is not true. 
		a.m_timeNow = time(0);
		a.m_numSymbolsToDownload = 1; a.m_historicalDataDone = false; historicalDataL.downloadHistoricalData_Single(); //****After this call, a.m_numDataOfSingleSymbolL will be set with a value. 
		a.L_BarVectorDefaul();
		a.update_manualsVector();
		a.earningReportDate(); //**** This is actually not necessary here. Just for testing purpose. It only matters in trading process.
		a.m_numSymbolsToDownload = a.m_newtxtVector.size(); a.m_historicalDataDone = false;
		err = _beginthread(HistoricalDataL::HistoricalDataStaticEntryPoint, 0, &historicalDataL); if (err < 0) { printf("Could not start new threads.\n"); }
		while (1) { if (a.m_historicalDataDone) break; else Sleep(1000); }
		Sleep(10000);
		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		exit(0);
	}
	else{ //time(0) <= tradingEndTime. During or before trading.
		a.m_timeNow = time(0) - 13 * 60 * 60; //****If I changed it into eastern time, I need modify this?
		a.m_numSymbolsToDownload = 1; a.m_historicalDataDone = false; historicalDataL.downloadHistoricalData_Single(); //****After this call, numDataOfSingleSymbolL will be set with a value. 
		a.L_BarVectorDefaul();
		a.update_manualsVector();
		a.earningReportDate();
		a.update_historyVector();
		time_t startTime = time(0), timePassed, nextHeartBeat = time(0);
		a.m_numSymbolsToDownload = a.m_newtxtVector.size(); a.m_historicalDataDone = false;
		err = _beginthread(HistoricalDataL::HistoricalDataStaticEntryPoint, 0, &historicalDataL); if (err < 0) { printf("Could not start new threads.\n"); }
		while (1){
			timePassed = time(0) - startTime;
			if (a.m_historicalDataDone) break;
			Sleep(100);
			if (time(0) > nextHeartBeat) { printf("I am waiting %d seconds for most historical data download done.\n", a.m_traderParameters.timeToContinue); nextHeartBeat = nextHeartBeat + 60; }
		}
		cfs.calculateBaseVector();

		a.m_numSymbolsToDownload = 1; a.m_historicalDataDone = false; historicalDataS.downloadHistoricalData_Single(); //****After this call, numDataOfSingleSymbolL will be set with a value. 
		a.S_BarVectorDefaul();
		//**** Similarly I can download s bars if necessary
	}

	EClientL0 *EC_datafeed, *EC_Long, *EC_Short; //**** ECLientL0 is abstract class and cannot be instantiated. However, we can create pointers to an abstract class. 
	DatafeedNormal datafeed(a);
	EC_datafeed = EClientL0::New(&datafeed); if (EC_datafeed == NULL) { printf("Creating EClient for dataFeed failed.\n"); 	getchar(); 	exit(0); } //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
	bool returnValue = false;
	std::chrono::high_resolution_clock::time_point now;
	while (1){
		bool b = true;
		try { a.m_csTimeDeque.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b){
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond){
				returnValue = EC_datafeed->eConnect("", a.m_port, a.m_clientId_MarketData); //***** This function starts a new thread automatically. Therefore, this function can never be in another threadEntryPoint function. I once made very big mistake and wastes my many hours.	
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}
		printf(" socket client is being created for datafeed in start.cpp.\n");

		if (returnValue == true) break; else { Sleep(20000); }
	} //end of while(1)
	datafeed.set_EC(EC_datafeed);

	TraderEWrapperNormalLong traderEWrapperLong(a);
	EC_Long = EClientL0::New(&traderEWrapperLong); if (EC_Long == NULL) { printf("Creating EClient failed.\n"); 	getchar(); 	exit(0); } //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
	returnValue = false;
	while (1){
		bool b = true;
		try { a.m_csTimeDeque.EnterCriticalSection(); }catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b){
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond){
				returnValue = EC_Long->eConnect("", a.m_port, a.m_clientIdLong); //***** This function starts a new thread automatically. Therefore, this function can never be in another threadEntryPoint function. I once made very big mistake and wastes my many hours.	
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}
		printf(" long socket client is being created in startNormal.cpp.\n");

		if (returnValue == true) break; else { Sleep(20000); }

	} //end of while(1)

	TraderNormalLong trader1(cfs, a, EC_Long, traderEWrapperLong);
	traderEWrapperLong.set_EC(EC_Long);
	traderEWrapperLong.update_traderStructureVector_by_EWrappers();

	TraderEWrapperNormalShort traderEWrapperShort(a);
	EC_Short = EClientL0::New(&traderEWrapperShort); if (EC_Short == NULL) { printf("Creating EClient failed.\n"); 	getchar(); 	exit(0); } //****  Avoid using exit(0) as it may cause memory leaks if not properly used. 
	returnValue = false;
	while (1){
		bool b = true;
		try { a.m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b){
			if (a.m_timeDeque.size() < a.m_messageLimitPerSecond){
				returnValue = EC_Short->eConnect("", a.m_port, a.m_clientIdShort); //***** This function starts a new thread automatically. Therefore, this function can never be in another threadEntryPoint function. I once made very big mistake and wastes my many hours.	
				now = std::chrono::high_resolution_clock::now(); //GetSystemTimeAsFileTime is system API, and can also be called as ::GetSystemTimeAsFileTime.
				a.m_timeDeque.push_back(now);
			}
			a.m_csTimeDeque.LeaveCriticalSection();
		}
		printf(" short socket client is being created in startNormal.cpp.\n");

		if (returnValue == true) break; else { Sleep(20000); }
	} //end of while(1)
	traderEWrapperShort.set_EC(EC_Short);
	traderEWrapperShort.update_traderStructureVector_by_EWrappers();
	TraderNormalShort trader2(cfs, a, EC_Short, traderEWrapperShort);

	//Sleep(10000); //It seems here above does not have problems.
	if (time(0) <= a.m_tradingEndTime){ //Preparing data for tomorrow. When runing this, must make sure SPY dayBar for today is already available. The application will complain if this is not true. 
		err = _beginthread(DatafeedNormal::DatafeedStaticEntryPoint, 0, &datafeed);  if (err < 0) { printf("Could not start new threads.\n"); }
		err = _beginthread(TraderNormalLong::TraderNormalLongStaticEntryPoint, 0, &trader1);  if (err < 0) { printf("Could not start new threads for normalLong.\n"); }
		err = _beginthread(TraderNormalShort::TraderNormalShortStaticEntryPoint, 0, &trader2);  if (err < 0) { printf("Could not start new threads for normalShort.\n"); }

		//**** Note to keep the threads initiated here alive. I must keep the thead here alive. Otherwise all threads will be dead tother with weird errors. 
		///************** Don't comment this line
		//*************** Two IMPORTANT Points: [1] If without the Sleep(100000000) below, then once the main thread is gone, then all other threads will be done. [2] When starting a new thread I need make sure that the class instance
		//*************** used in the new thread will not be dead. The class instance should be alive all the time when the application is running, unless I really don't need the thread to be alive that long. In the main() above,
		//*************** I define several class instances but not define them in a new class member function. This is because when that member function is gone, then class instance will often be gone too. If that happens, I will get errors such as "pure virtual funtion call " or "invalid access" or "unhandled exception" etc. 
		Sleep(100000000); // This is critically important for old C++ standard. Without it, the termination of main thread will terminate all other running threads. 
	}

}
