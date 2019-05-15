#include "startSymbol.h"
#include "startTrade.h"
#include "startTest.h"
#include "startScanner.h"

#include <cstdio>
#include <ctime>
#include <chrono>
#include <set>
#include <list>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <iostream>     // std::cout, std::ostream, std::ios
#include <fstream>      // std::filebuf
#include <thread>


using namespace std;

int main( void )
{

	int err = 0;

	//// Part 0: Scanner-helped Trading
	//StartScanner startScanner;
	//err = _beginthread(StartScanner::StartStaticEntryPoint, 0, &startScanner);  if (err < 0) { printf("Could not start new threads.\n"); } //*****If I only run one application on one account, I don't need a different thread.


	//// Part I: Symbol Selection
	//StartSymbol startSymbol;
	////*****
	////***** NOTE I NEED change the endDateIndex for symbol selection and testing.
	//err = _beginthread(StartSymbol::StartStaticEntryPoint, 0, &startSymbol);  if (err < 0) { printf("Could not start new threads.\n"); } //*****If I only run one application on one account, I don't need a different thread.
	////***** NOTE I NEED change the endDateIndex for symbol selection and testing. 


	//// Part II: back testing
	////***** NOTE I NEED change the endDateIndex for symbol selection and testing.
	//AllData a;  //Is it like a namespace, but with only one big container class a, and hence eliminate the namespace name. Furthermore with a short class name. In the namespace (class), there are many other container classes defined. So it looks fine.
	//a.initializeTraderParameters();  a.initializeOtherParameters();
	//a.m_backTest = true; //**** IMPORTANT! This must be false all the time for real trading.
	//a.initializeSymbolVectors();
	//a.initializeBaseVector(); a.initializeCurrentDataVector();  a.initializeOtherVectors();
	//err = _beginthread(AllData::MessageLimitationStatic, 0, &a);  if (err < 0) { printf("Could not start new threads.\n"); } // This is for handling 50 message limitation. 
	//CommonFunctions cfs(a);
	//cfs.initialization();
	//StartTest startTest(cfs,a);
	////***** NOTE I NEED change the endDateIndex for symbol selection and testing.
	//startTest.StartEntryPoint();
	////***** NOTE I NEED change the endDateIndex for symbol selection and testing.
	//startTest.summarize();


	// Part III: trading
	StartTrade startTrade;
	err = _beginthread(StartTrade::StartStaticEntryPoint, 0, &startTrade);  if (err < 0) { printf("Could not start new threads.\n"); } //*****If I only run one application on one account, I don't need a different thread.
	
	
	std::this_thread::sleep_for(std::chrono::hours(100)); //This basically keeps the thread always alive.
	/////************** Don't comment this line
	/////************** Don't comment this line
	////*************** Two IMPORTANT Points: [1] If without the Sleep(100000000) below, then once the main thread is gone, then all other threads will be done. [2] When starting a new thread I need make sure that the class instance
	////*************** used in the new thread will not be dead. The class instance should be alive all the time when the application is running, unless I really don't need the thread to be alive that long. In the main() above,
	////*************** I define several class instances but not define them in a new class member function. This is because when that member function is gone, then class instance will often be gone too. If that happens, I will get errors such as "pure virtual funtion call " or "invalid access" or "unhandled exception" etc. 
    // This is critically important for old C++ standard. Without it, the termination of main thread will terminate all other running threads. 
	return 0;	
	
}

