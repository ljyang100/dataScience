#ifndef startScanner_h_
#define startScanner_h_

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

#include "TraderEWrapperNormalLong.h"
#include "TraderEWrapperNormalShort.h"
#include "datafeedNormal.h"
#include "traderNormalLong.h"
#include "traderNormalShort.h"
#include "CriticalSection.h"
#include "sharedstructure.h"
#include "historicalDataNormal.h"
#include "historicalData_L.h"
#include "commonFunctions.h"
#include "allData.h"
//using namespace std; //**** Prefer not use this statement in the future

#include <ctime>
#include <chrono>

class  StartScanner
{
private:
	
public:

	static void __cdecl StartStaticEntryPoint(void * pThis)
	{
		StartScanner * pStart = (StartScanner*)pThis;
		pStart->StartEntryPoint(); //call the true entry-point-function
	}
	void StartEntryPoint();

	~StartScanner()
	{} 


};
#endif
