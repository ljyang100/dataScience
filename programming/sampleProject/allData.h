#ifndef allData_h_
#define allData_h_

#include <time.h> //**** This not used. Delete it in the future
#include<iostream> //My code, for MyWrapper::historicalData to use std:cin, std:out, and std::std::endl,etc. 
#include<fstream>  //My code, for file manipulation.
#include<sstream>  //My code for using  ss<<int_reqId, etc;	
#include<math.h>   //My code, for mathematical operations. 
#include<assert.h> //My code for useing assert. 
#include <process.h> // for _beginthread()
#include "CriticalSection.h"
#include "sharedstructure.h"
#include <ctime> //delete in the future. time_t 
#include <chrono> //delete in the future
#include <thread>
#include <list>
#include <map>
#include <unordered_map>

class AllData{

public: //Unlike a structure, members are private in default and hence cannot be accessed outside of class


	//***** This class provide data shared by many threads, and therefore it is necessary to clearly separate their attricutes. sharedStructure.h also provide common data, but they will all be accessed here. For those constant data like traderParameters have already been declared as constant there.
	// [1] For those constant quantities, declare as constants to preven accident modification in other threads. [If only a few member variables in a class need change, use the keyword mutable].
	// [2] For variables occurring in multiple threads, must have critical section.
	// [3] For variables occurring only in one thread, go without critical section protection. **** Normally these variables should declared within the specific thread but not in the common data here. Sometimes we put them here for clarity, or for not always passing by class constructor.


	//**** Below are some tricks for providing critical sections.
	// [1] To avoid many critical section variables, I may combine many to-be-protected variables in a structure, and protect them with a single crtical section variable.
	// [2] Sometimes even we cannot group many variables togeter such as traderVector, newtxtVector, algoStringVector, we still can use a single critical seciton variable to protect them. However, do this as less as possible.
	// [3] In the future, when the lockless programming become more mature and easier to handle, may consider use them. For example, the c++ 11 atomicity, etc. 


	AllData() {};  
	~AllData() {};

	//Part I; constants: critical section protection unnecessary. 
	//One part of constants already defined in sharedStructure.h.
	TraderParameters m_traderParameters; //**** I cannot add a const before TraderParameters
	const unsigned int m_clientIdLong = 11;
	const unsigned int m_clientId_MarketData = 12;
	const unsigned int m_clientId_HistoricalData_L = 13;
	const unsigned int m_clientId_HistoricalData_S = 14;
	const unsigned int m_clientIdShort = 15;
	const unsigned int m_bardurationL = 86400;
	const unsigned int m_bardurationS = 180;
	//*****************************************************************************************************
	//barduration.. is in units of seconds. daysOfHis...if bigger than 2, should be multiple of 7, 30, or 360.
	//Be careful I need simultaneously modify the min and max number of bars when modifying the above bardurationS.  
	//******************************************************************************************************
	const unsigned int m_daysOfHistoricalDataL = 180;
	const unsigned int m_daysOfHistoricalDataS = 7;
	const unsigned int m_wholeDayTradeDuration = 6.5 * 60 * 60;
	const unsigned int m_messageLimitPerSecond = 40; //******* Note the message limit per username or per log-in is 50. If I have 32 client, then I have to share the 50 message limit. Currently I have three application sharing the 50. 
	const unsigned int m_port = 7497;
	const std::string m_accountCode = "DU104212";
	const std::string m_newSymbols = "C:/symbols/symbolsNEW" + m_accountCode + ".txt";
	const std::string m_errorOutputLong = "C:/symbols/NormalReportLong" + m_accountCode + ".txt";
	const std::string m_errorOutputShort = "C:/symbols/NormalReportShort" + m_accountCode + ".txt";
	const std::string m_vectorL_NEW = "C:/symbols/vectorL_NEW" + m_accountCode + ".txt";
	const std::string m_manuals_OLD = "C:/symbols/manualsNormal_OLD" + m_accountCode + ".txt";
	const std::string m_manuals_NEW = "C:/symbols/manualsNormal_NEW" + m_accountCode + ".txt";
	const std::string m_earningReportDate = "C:/symbols/earningReportDate.txt";
	const unsigned int m_noStopOrder = 1000000;




	//Part II Variables: critical section Protection necessary.
	std::vector<CurrentDataStructure> m_currentDataStructureVector;
	Critical_Section m_csCurrentDataStructureVector;
	std::vector<std::vector<std::vector<My_BarDataNormal>>>  m_VectorS;
	std::vector<std::vector<My_BarDataNormal>>  m_VectorL;
	Critical_Section m_csVectorL, m_csVectorS;
	std::deque<std::chrono::high_resolution_clock::time_point> m_timeDeque;
	Critical_Section m_csTimeDeque;
	bool m_positionUpdateDoneLong, m_accountSummaryDoneLong, m_executionDoneLong, m_positionUpdateDoneShort, m_accountSummaryDoneShort, m_executionDoneShort;
	Critical_Section m_csPositionUpdateDoneLong, m_csAccountSummaryDoneLong, m_csExecutionDoneLong, m_csPositionUpdateDoneShort, m_csAccountSummaryDoneShort, m_csExecutionDoneShort;
	bool m_openOrderUpdateDoneLong, m_updatePorfolioUpdateDoneLong, m_openOrderUpdateDoneShort, m_updatePorfolioUpdateDoneShort;
	Critical_Section m_csOpenOrderUpdateDoneLong, m_csUpdatePorfolioUpdateDoneLong, m_csOpenOrderUpdateDoneShort, m_csUpdatePorfolioUpdateDoneShort;
	bool m_orderIdReadyLong, m_orderIdReadyShort;
	Critical_Section m_csOrderIdReadyLong, m_csOrderIdReadyShort;
	OrderId m_orderIdLong, m_orderIdShort; //**** shared protected by m_csOrderIdReadyLong, m_csOrderIdReadyShort
	unsigned int m_barIndexShift; //**** shared protected by m_csVectorS. m_barIndexShift is currently not used. If it will be used in the future, it might be appropriate to have a new protection
	TraderVector m_traderVector;
	Critical_Section m_csTraderVector;
	std::vector<std::vector<BaseStructure>> m_baseVector; //**** Always make sure it is always within the critical section of m_traderVector. If I don't calculate baseVector during trading, then it is OK without protection.
	std::vector<std::string> m_newtxtVector; //**** Always make sure it is always within the critical section of m_traderVector.
	double m_volatilityFactorLong, m_volatilityFactorShort; //**** Always make sure it is always within the critical section of m_traderVector.
	bool m_closeLongPositions, m_closeShortPositions;  //**** Always make sure it is always within the critical section of m_traderVector.
	std::unordered_map<std::string, unsigned int> m_newtxtMap; 
	//**** This is a symbol list parallel to the m_newtxtVector. If in the future symbol list is changing during running time, then I need protect it with the same critical section of trader Vector.


	//Part III. Variables never change after initialization in the start of application, Or just used in the same thread. And hence no critical section is necessary.
	std::map<std::string, unsigned int> m_manualMap;//**** For the purpose of simple mapping (not Hash searching) of a map, I may use std::map (no need for std::unordered_map) or even use pair struct pushed into vector.
	std::map<std::string, unsigned int> m_historyMap; //**** The two maps are defined here because it is not easy to initialize them to be constant. 
	std::vector<AlgoLiteral> m_algoString; 
	TraderStructure m_traderStructureDefault;
	bool m_backTest; //This variable although introduced by backtesting, it also appear in many other threads involving normal trading. But never change once initialized in the beginning. 
	std::vector<int> m_baseNum;
	time_t m_initime;
	time_t m_tradingEndTime;
	unsigned int m_iSize;
	std::ofstream m_myfileLong, m_myfileShort; //**** Just used in one thread respectively. 
	time_t m_timeNow;
	unsigned int m_numSymbolsToDownload, m_numDataOfSingleSymbolL, m_numDataOfSingleSymbolS; 
	bool m_historicalDataDone;
	bool m_mostRecentDate = false;
	//**** Only appear in one thread respectively for long and short. Make sure this by searching the whole solution.


	// Part IV. Variables does not involve multi-threading. They are for either selection symbol or for back testing. 
	std::deque<My_BarDataNormal>  m_index_deque;
	std::string  m_dataFolderPrefix;
	const std::string m_endDateIndex = "20180208"; // This is for backtesting and symbol selection.
	time_t m_backTest_symbolSection_EndTime;
	//***** The following are for backTest project
	std::string m_vectorFileName_L_Test = "C:/symbols/vectorL_Test" + m_accountCode + ".txt";
	std::string m_vectorFileName_S_Test = "C:/symbols/vectorS_Test" + m_accountCode + "_.txt";
	std::list<FinishedTradeStructure> m_finishedTradesList;
	std::vector<TradeStructureTest> m_traderVectorTest;
	FinishedTradeStructure m_finishedTradeStructure_default;
	unsigned int m_baseVectorLastDayIndex;
	unsigned int m_tradingDayIndex;
	const unsigned int m_numShortBarsPeriodTest = 2; //**** If the testing period is one week, this indicates I will test in a two-week period.
	std::vector<std::vector<My_BarDataNormal>> m_VectorS_Test;
	std::vector<double> m_lastDayClose;


public:
	static void __cdecl MessageLimitationStatic(void * pThis)
	{
		AllData * pAllData = (AllData*)pThis;
		pAllData->MessageLimitation();
	}

	void MessageLimitation();
	void initializeTraderParameters();
	void initializeOtherParameters();
	void initializeSymbolVectors_Prev();
	void initializeSymbolVectors();
	void initializeBaseVector(std::string string);
	void initializeOtherVectors();//**** Can initialize with only the symbolVector size
	void initializeCurrentDataVector();
	void initializeBackTestVectors();
	void L_BarVectorDefaul();
	void S_BarVectorDefaul();
	void update_manualsVector();
	void update_historyVector();
	void historyVectorLoad_backTest(std::vector<std::vector<My_BarDataNormal>> &m_vector, std::string &FileName);
	void earningReportDate();
	void unhandledSymbols();

};

#endif
