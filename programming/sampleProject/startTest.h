#ifndef startTest_h_
#define startTest_h_

#include "TwsApiL0.h"
// to use the Sleep function
#ifdef WIN32
#include <windows.h>		// Sleep(), in miliseconds
#include <process.h>
#define CurrentThreadId GetCurrentThreadId
#else
#include <unistd.h>			// usleep(), in microseconds
#define Sleep( m ) usleep( m*1000 )
#include <pthread.h>
#define CurrentThreadId pthread_self
#endif

#include <time.h>
#include<iostream> //My code, for MyWrapper::historicalData to use std:cin, std:out, and std::std::endl,etc. 
#include<fstream>  //My code, for file manipulation.
#include<sstream>  //My code for using  ss<<int_reqId, etc;	
#include<math.h>   //My code, for mathematical operations. 
#include<assert.h> //My code for useing assert. 
#include <process.h> // for _beginthread()
//#include <windows.h>

#include "CriticalSection.h"
#include "sharedstructure.h"
#include "historicalDataNormal.h"
#include "historicalData_L.h"
#include "commonFunctions.h"
#include "allData.h"
//using namespace std; //**** Prefer not use this statement in the future

#include <ctime>
#include <chrono>
#include <thread>

class  StartTest
{
private:
	
public:

	std::string m_longOrNot, m_manualLongOrNot;
	std::string m_shortOrNot, m_manualShortOrNot;

	AllData &a;
	CommonFunctions &cfs;


	StartTest(CommonFunctions &cfs_input, AllData &a_instance)
		:cfs(cfs_input)
		, a(a_instance)
		, m_longOrNot("LONG")
		, m_manualLongOrNot("LONG")
		, m_shortOrNot("SHORT")
		, m_manualShortOrNot("SHORT")

	{}

	void StartEntryPoint();

	void createBuyOrderLong(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex);
	void createSellOrderLong(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndexs);
	void createBuyOrderShort(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex);
	void createSellOrderShort(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndexs);
	void summarize();
	void showAllTradingConditions();

	~StartTest()
	{
	}
};
#endif
