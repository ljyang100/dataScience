#ifndef startSymbol_h_
#define startSymbol_h_

#include <time.h>
#include<iostream> //My code, for MyWrapper::historicalData to use std:cin, std:out, and std::std::endl,etc. 
#include<fstream>  //My code, for file manipulation.
#include<sstream>  //My code for using  ss<<int_reqId, etc;	
#include<math.h>   //My code, for mathematical operations. 
#include<assert.h> //My code for useing assert. 
#include <process.h> // for _beginthread()
#include <windows.h>
#include "commonFunctions.h"
#include "allData.h"
#include "symbolSelection.h"
#include <ctime>
#include <chrono>

class  StartSymbol
{


public:
	std::vector<std::string> exchangeNameVector;
	std::vector<InputStructure> inputStructureVector;

	static void __cdecl StartStaticEntryPoint(void * pThis)
	{
		StartSymbol * pStart = (StartSymbol*)pThis;
		pStart->StartEntryPoint(); //call the true entry-point-function
	}
	void StartEntryPoint();

	~StartSymbol()
	{
	}
};
#endif
