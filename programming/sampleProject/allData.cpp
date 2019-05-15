#include "allData.h"
//#include <chrono> //*** don't need this header even I use std::chrono::high_resolution_clock::time_point, because I have included std::chrono...
void AllData::initializeTraderParameters()
{
	m_volatilityFactorLong = 1.0;
	m_volatilityFactorShort = 1.0;
	m_closeLongPositions = false;
	m_closeShortPositions = false;
	//**** Always use {} for pairs in uniform initialization of a map. This is easy for initialzing a constant map with not-so-many element.
	m_manualMap.insert({ "symbol", 0 }); m_manualMap.insert({ "stopLossLong", 1 }); m_manualMap.insert({ "stopLossShort", 2 }); m_manualMap.insert({ "algoStringLong", 3 }); m_manualMap.insert({ "algoStringShort", 4 });
	m_manualMap.insert({ "totalPosition", 5 });

	m_historyMap.insert({ "date", 0 }); m_historyMap.insert({ "yearMonthDateString", 1 }); m_historyMap.insert({ "open", 2 }); m_historyMap.insert({ "high", 3 }); m_historyMap.insert({ "low", 4 });
	m_historyMap.insert({ "close", 5 }); m_historyMap.insert({ "volume", 6 });	m_historyMap.insert({ "lastDownloadTime", 7 }); m_historyMap.insert({ "symbol", 8 }); 
	AlgoLiteral algoLiteral; 
	algoLiteral.LONG = "whiteSwanLong"; algoLiteral.SHORT = "whiteSwanShort"; m_algoString.push_back(algoLiteral);
	algoLiteral.LONG = "blackSwanLong"; algoLiteral.SHORT = "blackSwanShort"; m_algoString.push_back(algoLiteral);
	
	////Part I trader parameters
	//****The following need to be done here because symbol selection code need.

	m_baseVector.clear();
	m_baseNum.clear();
	m_baseNum.push_back(5);  m_baseNum.push_back(10); m_baseNum.push_back(20); m_baseNum.push_back(40);  m_baseNum.push_back(80);   //The number of bar may never be determined before downloading.
	m_VectorL.clear(); m_VectorS.clear();
	m_VectorS_Test.clear();

	///////////////////////////////////////////////////
	//****** The following is for symbol selection only.
	m_index_deque.clear();

	//***** The following is used for back testing and symbol selection
	std::string yearString = m_endDateIndex.substr(0, 4); //****IMPORTANT NOTES: start with first character (with index 0 but not 1). 4 is the length of the string, but not the index of the final character (like Python)
	std::string monthString;
	if (m_endDateIndex.at(4) == '0') monthString = m_endDateIndex.substr(5, 1); else  monthString = m_endDateIndex.substr(4, 2); //**** This might be not necessary. atoi might have the ability of neglect zero before non-zero numbers. This is for 100% sure.
	std::string dayString;
	if (m_endDateIndex.at(6) == '0') dayString = m_endDateIndex.substr(7, 1); else  dayString = m_endDateIndex.substr(6, 2);
	time_t now = time(0);
	struct tm *ts = localtime(&now); //**** I need initialize it before modify it later.
	ts->tm_year = atoi(yearString.c_str()) - 1900;
	ts->tm_mon = atoi(monthString.c_str()) - 1; //***** IMPORTANT. Without -1 there will be big problem.See the range of time variables below. mon runs from 0 to 11
	ts->tm_mday = atoi(dayString.c_str());		//***** IMPORTANT  With a -1 there will be big problem. See the range of time variables below. mday runs from 1 to 31
	ts->tm_hour = 20; //**** This will include the historical data for the date shown in then m_endDateIndex, either I download in eastern or pacific time. 
	//******Be careful of this setting. It might be have a day difference when downloading from IB, and get data from EOD download.
	ts->tm_min = 0;
	ts->tm_sec = 0;
	m_backTest_symbolSection_EndTime = mktime(ts); // Set a zero point makes things easier. 
	if (m_mostRecentDate == true) m_backTest_symbolSection_EndTime = time(0); //**** Note m_mostRecentDate
}

void AllData::initializeOtherParameters()
{
	//**** need figure out what is this. It should be same in all places?
	m_iSize = (m_traderParameters.maxSmallBarsConsidered - m_traderParameters.minSmallBarsConsidered) / m_traderParameters.stepSmallBars + 1; //**** This must be same as that of traderNormalLong.cpp

	m_orderIdLong = -1; m_orderIdShort = -1;
	m_orderIdReadyLong = false; m_orderIdReadyShort = false;
	m_positionUpdateDoneLong = false; m_accountSummaryDoneLong = false; m_executionDoneLong = false; m_positionUpdateDoneShort = false; m_accountSummaryDoneShort = false; m_executionDoneShort = false;
	m_openOrderUpdateDoneLong = false; m_updatePorfolioUpdateDoneLong = false; m_openOrderUpdateDoneShort = false; m_updatePorfolioUpdateDoneShort = false;
	m_myfileLong.open(m_errorOutputLong, std::ios::app), m_myfileShort.open(m_errorOutputShort, std::ios::app);



	m_historicalDataDone = false;

	time_t now;
	struct tm *ts;
	now = time(0);
	ts = localtime(&now);
	//I need only modify the following three fields.
	ts->tm_hour = 6;
	ts->tm_min = 30;
	ts->tm_sec = 0;
	m_initime = mktime(ts);
	m_tradingEndTime = m_initime + 6.5 * 60 * 60; //**** This must be after the initialization of m_initime;
}

void AllData::initializeBackTestVectors()
{
	m_finishedTradesList.clear();
	m_finishedTradeStructure_default.reqId = -1;
	m_finishedTradeStructure_default.symbol = "EMPTY";
	m_finishedTradeStructure_default.longOrShort = "EMPTY";
	m_finishedTradeStructure_default.OpenedDateIndex = 0;
	m_finishedTradeStructure_default.OpenedSmallBarIndex = 0;
	m_finishedTradeStructure_default.closedDateIndex = 0;
	m_finishedTradeStructure_default.closedSmallBarIndex = 0;
	m_finishedTradeStructure_default.opened = false;
	m_finishedTradeStructure_default.closed = false;
	m_finishedTradeStructure_default.algoString = "EMPTY";
	m_finishedTradeStructure_default.sharesOpened = 0;
	m_finishedTradeStructure_default.gain_loss = 0;
	m_finishedTradeStructure_default.commissionOpened = 0;
	m_finishedTradeStructure_default.commissionClosed = 0;
	m_finishedTradeStructure_default.averageCost = 0;

	m_finishedTradeStructure_default.conditions.clear();
	bool tempBool = false;
	for (unsigned int i = 0; i < m_traderParameters.maxNumConditions; i++)	m_finishedTradeStructure_default.conditions.push_back(tempBool);

	m_traderVectorTest.clear();
	TradeStructureTest tradeStructureTest;
	tradeStructureTest.longOpened = false;
	tradeStructureTest.longClosed = false;
	tradeStructureTest.shortOpened = false;
	tradeStructureTest.shortClosed = false;
	tradeStructureTest.OpenedDateIndex = 0;
	tradeStructureTest.smallBarIndex = 0;

	for (unsigned int reqId = 0; reqId < m_newtxtVector.size(); reqId++)
		m_traderVectorTest.push_back(tradeStructureTest);

	m_lastDayClose.clear();
	for (unsigned int reqId = 0; reqId < m_newtxtVector.size(); reqId++) m_lastDayClose.push_back(0);
}

void AllData::initializeSymbolVectors_Prev()
{
	//**** Part I Read in newly selected Symbols 
	std::string newTradingSymbols = "C:/symbols/symbolsNEW_Prev"; newTradingSymbols.append(m_accountCode); newTradingSymbols.append(".txt");
	std::ifstream i_inputFile(newTradingSymbols, std::ios_base::in); if (!i_inputFile.is_open()) printf("Failed in opening the file new.txt.\n");
	std::string wholeLineString;
	std::string symbol;
	std::list<std::string> newSymbolsList; newSymbolsList.clear();
	std::stringstream   linestream;
	while (getline(i_inputFile, wholeLineString, '\n'))
	{
		linestream.clear();
		linestream.str(wholeLineString);
		linestream >> symbol;
		newSymbolsList.push_back(symbol);
	}//while (getline(i_inputFile, wholeLineString, '\n'))
	i_inputFile.close();

	//**** Part II Add old symbols that have position
	std::ifstream i_dataFile(m_manuals_NEW, std::ios_base::in);
	if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", m_manuals_NEW.c_str());
	else
	{
		TraderStructure manuals;
		std::string lineString, columnString;
		std::vector<std::string> columnStringVector;
		while (getline(i_dataFile, lineString, '\n'))
		{
			std::stringstream   lineStream(lineString);
			columnStringVector.clear();
			while (getline(lineStream, columnString, ','))	columnStringVector.push_back(columnString);

			if (m_manualMap.size() > 0 && m_manualMap.size() == columnStringVector.size()){
				for (auto i = m_manualMap.begin(); i != m_manualMap.end(); ++i){
					if ((*i).first == "symbol") manuals.symbol = columnStringVector[(*i).second];
					else if ((*i).first == "totalPosition") manuals.totalPosition = std::stoi(columnStringVector[(*i).second]);
				}
			}
			else{
				printf("something is wrong in update_historyVector().\n");
			}

			if (manuals.totalPosition != 0)
			{
				for (auto it = newSymbolsList.begin(); it != std::prev(newSymbolsList.end(), 1); ++it)
				{
					if ((*it) < manuals.symbol && manuals.symbol < (*std::next(it, 1))) //****Note > and < cannot be >= or <=. The second term might be *std::next(it,1)
						newSymbolsList.insert(std::next(it, 1), manuals.symbol);
				}
			}
		}//while (getline(i_dataFile, dataString, '\n'))
		i_dataFile.close();
	}

	//**** Part IV output the final symbol list. 
	std::string oldSymbol = "C:/symbols/symbolsOLD"; oldSymbol.append(m_accountCode); oldSymbol.append(".txt");
	std::string newSymbol = "C:/symbols/symbolsNEW"; newSymbol.append(m_accountCode); newSymbol.append(".txt");
	remove(oldSymbol.c_str());
	rename(newSymbol.c_str(), oldSymbol.c_str()); //**** rename new.txt to old.txt, but not the opposite.
	std::ofstream o_new(newSymbol, std::ios::app);
	for (auto element : newSymbolsList) if(element != "SPY") o_new << element << std::endl;
	o_new.close();
}

void AllData::initializeSymbolVectors()
{
	std::ifstream i_inputFile(m_newSymbols, std::ios_base::in); if (!i_inputFile.is_open()) printf("Failed in opening the file new.txt.\n");
	std::istringstream instream;
	unsigned int reqId = 0;
	m_newtxtVector.clear();
	std::string symbol = "SPY"; //******Cannot delete this symbol because of historical download etc. 
	bool b = true;
	try { m_csTraderVector.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		//**** Use the same critical section of trader Vector. The symbol list might change in the future.
		m_newtxtVector.push_back(symbol);
		m_newtxtMap.insert({ symbol, reqId });
		m_csTraderVector.LeaveCriticalSection();
	}

	reqId = 1; //**** SPY take the first place already.
	std::string wholeLineString;
	while (getline(i_inputFile, wholeLineString, '\n'))
	{
		std::stringstream   linestream; linestream.clear();
		linestream.str(wholeLineString);
		linestream >> symbol;
		bool b = true;
		try { m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			//**** Use the same critical section of trader Vector. The symbol list might change in the future.
			m_newtxtVector.push_back(symbol);
			m_newtxtMap.insert({symbol, reqId});
			reqId = reqId + 1;
			m_csTraderVector.LeaveCriticalSection();
		}


	}//while (getline(i_inputFile, wholeLineString, '\n'))
	i_inputFile.close();


}

void AllData::unhandledSymbols()
{
	std::string excludedSymbols = "C:/symbols/excludedSymbols"; excludedSymbols.append(m_accountCode); excludedSymbols.append(".txt");
	std::ifstream i_excludeFile(excludedSymbols, std::ios_base::in); if (!i_excludeFile.is_open()) printf("Failed in opening the file excludedSymbols.txt.\n");
	std::stringstream linestream;
	std::string wholeLineString, symbol;
	while (getline(i_excludeFile, wholeLineString, '\n'))
	{
		linestream.clear();
		linestream.str(wholeLineString);
		linestream >> symbol;

		//***** Remember the ordering of following two critical sections. The one for traderVector should always be in the first place.
		bool b = true; 
		try { m_csTraderVector.EnterCriticalSection(); }catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			auto it = m_newtxtMap.find(symbol);
			if (it != m_newtxtMap.end())
			{
				m_traderVector.longBuy[it->second].manualOrder = true;
				m_traderVector.longSell[it->second].manualOrder = true;
				m_traderVector.shortSell[it->second].manualOrder = true;
				m_traderVector.shortBuy[it->second].manualOrder = true;
				printf("Symbol %s has been set to manual order.\n", m_newtxtVector[it->second].c_str()); //**** Here m_newtxtVector is used for comparing with the symbol input in file.
				m_csTraderVector.LeaveCriticalSection();
			}
		}
	}
	i_excludeFile.close();
}

void AllData::initializeBaseVector(std::string string)
{
	BaseStructure baseStructure;
	baseStructure.i = -1; //**** range index
	baseStructure.numDaysConsidered = -1;
	//baseStructure.symbol = "EMPTY";
	baseStructure.frequency = 0;
	baseStructure.finalLow = 0;
	baseStructure.finalHigh = 0;
	baseStructure.min = -1;
	baseStructure.minIndex = -1;
	baseStructure.max = -1;
	baseStructure.maxIndex = -1;
	baseStructure.ATR = 0;
	baseStructure.volatility = 0;
	baseStructure.averageMoneyVolume = 0;
	baseStructure.averageShareVolume = 0;
	baseStructure.correlation = 0;
	baseStructure.slope = 0;
	baseStructure.intercept = 0;
	baseStructure.originX = 0;
	baseStructure.originY = 0;
	baseStructure.upperOscillaitonTimesVolatility = 0;
	baseStructure.downOscillaitonTimesVolatility = 0;
	baseStructure.leftToTotalOscillation = 0;
	baseStructure.upperLeftToTotalCrossing = 0;
	baseStructure.downLeftToTotalCrosing = 0;
	baseStructure.upTradingLineIntercept = 0;
	baseStructure.downTradingLineIntercept = 0;
	baseStructure.upBoundaryIntercept = 0;
	baseStructure.downBoundaryIntercept = 0;
	baseStructure.slopeLocalMin = 0;
	baseStructure.interceptLocalMin = 0;
	baseStructure.originXLocalMin = 0;
	baseStructure.originYLocalMin = 0;
	baseStructure.slopeLocalMax = 0;
	baseStructure.interceptLocalMax = 0;
	baseStructure.originXLocalMax = 0;
	baseStructure.originYLocalMax = 0;
	baseStructure.oscillationRate = 0;
	baseStructure.upDeviationPercentage = 0;
	baseStructure.downDeviationPercentage = 0;
	baseStructure.trendingPencentage = 0;
	baseStructure.maxDrawDownPercentage = 0;
	baseStructure.volatilityPriceRatio = 0;
	baseStructure.futureTrendingAmountVolatilityRatio = 0;
	baseStructure.maxGapPercentage = 0;
	baseStructure.gapToDeviationRatio = 0;


	m_baseVector.clear(); //**** 
	std::vector<BaseStructure> tempVectorBase = *new(std::vector<BaseStructure>);
	tempVectorBase.clear();
	if (string == "SYMBOL")
	{
		for (auto it = m_newtxtMap.begin(); it != m_newtxtMap.end(); ++it)
			tempVectorBase.push_back(baseStructure);
	}
	else
	{
		for (unsigned int j = 0; j < m_newtxtVector.size(); j++)
			tempVectorBase.push_back(baseStructure);
	}
	for (unsigned int i = 0; i < m_baseNum.size(); i++) m_baseVector.push_back(tempVectorBase);
}

void AllData::initializeCurrentDataVector()
{
	//Part I: m_currentDataStructureVector
	CurrentDataStructure m_currentDataStructure;
	m_currentDataStructure.symbol = "EMPTY";
	m_currentDataStructure.ask = -1;
	m_currentDataStructure.askSize = -1;
	m_currentDataStructure.bid = -1;
	m_currentDataStructure.bidSize = -1;
	m_currentDataStructure.last = -1;
	m_currentDataStructure.lastSize = -1;
	m_currentDataStructure.open = -1;
	m_currentDataStructure.high = -1;
	m_currentDataStructure.low = -1;
	m_currentDataStructure.close = -1;
	m_currentDataStructure.timeStamp = 0;

	m_currentDataStructureVector.clear();

	for (unsigned int i = 0; i < m_newtxtVector.size(); i++)
	{
		m_currentDataStructure.symbol = m_newtxtVector[i];
		m_currentDataStructure.reqId = i;
		m_currentDataStructureVector.push_back(m_currentDataStructure);
	}

}

void AllData::initializeOtherVectors()
{
	// Part II: m_traderVector
	m_traderStructureDefault.myBid = 0;
	m_traderStructureDefault.totalPosition = 0;
	m_traderStructureDefault.buyingShares = 0;
	m_traderStructureDefault.currentTotalBuy = 0;
	m_traderStructureDefault.currentTotalSell = 0;
	m_traderStructureDefault.averageCost = 0;
	m_traderStructureDefault.closing = false;
	m_traderStructureDefault.opening = false;
	m_traderStructureDefault.cancelling = false;
	m_traderStructureDefault.orderId = 0;

	m_traderStructureDefault.symbol = "EMPTY";
	m_traderStructureDefault.orderType = "EMPTY";
	m_traderStructureDefault.orderUpdatedTime = time(0) * 2; //This value is good for hundreds of years starting from the year 2013.
	m_traderStructureDefault.lastModifiedTimeBuy = time(0);
	m_traderStructureDefault.lastModifiedTimeSell = time(0);
	m_traderStructureDefault.sellingShares = 0;
	m_traderStructureDefault.action = "EMPTY";
	m_traderStructureDefault.myAsk = 0;
	m_traderStructureDefault.totalCancellationsBuy = 0;
	m_traderStructureDefault.totalCancellationsSell = 0;
	m_traderStructureDefault.totalDuplicateOrderBuy = 0;
	m_traderStructureDefault.totalDuplicateOrderSell = 0;
	m_traderStructureDefault.modifiedBuy = true; //This indicates we are not modifying, and thus ready to modify. 
	m_traderStructureDefault.modifiedSell = true;
	m_traderStructureDefault.filledBuy = 0;
	m_traderStructureDefault.filledSell = 0;
	m_traderStructureDefault.numModifiedBuy = 0;
	m_traderStructureDefault.numModifiedSell = 0;
	m_traderStructureDefault.sessionClosing = false;
	m_traderStructureDefault.totalLongValue = 0;
	m_traderStructureDefault.totalShortValue = 0;
	m_traderStructureDefault.positionUpdatedTime = time(0) * 2;

	//**** The following is for backtesting.
	m_traderStructureDefault.lastFilledLongTime = 0;
	m_traderStructureDefault.lastFilledShortTime = 0;
	m_traderStructureDefault.filledPrice = 0;
	m_traderStructureDefault.needToClose = 0;
	m_traderStructureDefault.minPriceVariation = 0.01;
	m_traderStructureDefault.manualOrder = false;

	m_traderStructureDefault.calculatedBid = 0;
	m_traderStructureDefault.calculatedAsk = 0;
	m_traderStructureDefault.relevantShares = 0;
	m_traderStructureDefault.longHedgeOK = false;
	m_traderStructureDefault.shortHedgeOK = false;
	m_traderStructureDefault.stopLossLong = false;
	m_traderStructureDefault.stopLossShort = false;
	m_traderStructureDefault.basicOpen = false;
	m_traderStructureDefault.basicClose = false;
	m_traderStructureDefault.algoString = "EMPTY";
	m_traderStructureDefault.conditions.clear();
	for (unsigned int i = 0; i < m_traderParameters.maxNumConditions; i++)	m_traderStructureDefault.conditions.push_back(false); //***** push_back(false) but not a push_back(var), where var = false. An extra copy will be occured.
	m_traderStructureDefault.priceChanged = false;
	m_traderStructureDefault.noBuyLong = false;
	m_traderStructureDefault.noSellLong = false;
	m_traderStructureDefault.noBuyShort = false;
	m_traderStructureDefault.noSellShort = false;
	m_traderStructureDefault.stoppedOut = false;

	m_traderVector.longBuy.clear();
	m_traderVector.longSell.clear();
	m_traderVector.shortBuy.clear();
	m_traderVector.shortSell.clear();

	for (unsigned int i = 0; i < m_newtxtVector.size(); i++)
	{
		m_traderStructureDefault.symbol = m_newtxtVector[i];
		bool b = true;
		try{ m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_traderVector.longBuy.push_back(m_traderStructureDefault);
			m_traderVector.longSell.push_back(m_traderStructureDefault);
			m_traderVector.shortBuy.push_back(m_traderStructureDefault);
			m_traderVector.shortSell.push_back(m_traderStructureDefault);

			m_csTraderVector.LeaveCriticalSection();
		}
	}


}

void AllData::L_BarVectorDefaul()
{
	My_BarDataNormal barData;
	std::stringstream ss;
	ss << time(0);
	barData.date = "EMPTY";
	barData.yearMonthDateString = "EMPTY";
	barData.open = 0;
	barData.high = 0;
	barData.low = 0;
	barData.close = 0;
	barData.volume = 0;
	barData.lastDownloadTime = "0";
	barData.symbol = "EMPTY";
	//barData.reachToMaxTime = 0; //The reachToMax/MinTime is not used in smaller-than-day bars. 
	//barData.reachToMinTime = 0;

	std::vector<My_BarDataNormal> m_tempVector;
	m_tempVector.clear();
	for (unsigned int k = 0; k < m_numDataOfSingleSymbolL; k++) m_tempVector.push_back(barData);
	m_VectorL.clear();
	for (unsigned int reqId = 0; reqId < m_newtxtVector.size(); reqId++)
	{
		bool b = true;//This is for the zeroth element of the vector m_VectorS
		try{ m_csVectorL.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_VectorL.push_back(m_tempVector);
			 m_csVectorL.LeaveCriticalSection();
		}
	}
}

void AllData::S_BarVectorDefaul()
{
	My_BarDataNormal barData;
	std::stringstream ss;
	ss << time(0);
	barData.date = "EMPTY";
	barData.open = 0;
	barData.high = 0;
	barData.low = 0;
	barData.close = 0;
	barData.volume = 0;
	barData.lastDownloadTime = "0";
	barData.symbol = "EMPTY";

	std::vector<My_BarDataNormal> m_tempVector;
	std::vector<std::vector<My_BarDataNormal>> m_tempVectorVector;

	m_tempVector.clear(); m_tempVectorVector.clear();
	for (unsigned int k = 0; k < m_numDataOfSingleSymbolS; k++) m_tempVector.push_back(barData);
	for (unsigned int reqId = 0; reqId < m_newtxtVector.size(); reqId++) m_tempVectorVector.push_back(m_tempVector);

	m_VectorS.clear();
	for (unsigned int i = 0; i < m_iSize; i++) //The zeroth element has been populated earlier.
	{
		bool b = true;//This is for the zeroth element of the vector m_VectorS
		try{ m_csVectorS.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			m_VectorS.push_back(m_tempVectorVector);
			m_csVectorS.LeaveCriticalSection();
		}
	}
}

void AllData::update_manualsVector()
{
	std::ifstream i_dataFile(m_manuals_NEW, std::ios_base::in);
	if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", m_manuals_NEW.c_str());
	else
	{
		TraderStructure manuals;
		std::string algoStringLong, algoStringShort;
		std::string lineString, columnString;
		std::vector<std::string> columnStringVector;
		while (getline(i_dataFile, lineString, '\n'))
		{
			std::stringstream   lineStream(lineString);
			columnStringVector.clear();
			while (getline(lineStream, columnString, ','))
			{
				columnStringVector.push_back(columnString);
			}

			if (m_manualMap.size() > 0 && m_manualMap.size() == columnStringVector.size()){
				for (auto i = m_manualMap.begin(); i != m_manualMap.end(); ++i){
					if ((*i).first == "symbol") manuals.symbol = columnStringVector[(*i).second];
					else if ((*i).first == "stopLossLong") manuals.stopLossLong = std::stod(columnStringVector[(*i).second]);
					else if ((*i).first == "stopLossShort") manuals.stopLossShort = std::stod(columnStringVector[(*i).second]);
					else if ((*i).first == "algoStringLong") algoStringLong = columnStringVector[(*i).second];
					else if ((*i).first == "algoStringShort") algoStringShort = columnStringVector[(*i).second];
					else if ((*i).first == "totalPosition") manuals.totalPosition = std::stoi(columnStringVector[(*i).second]);
					//**** The above can become more efficient if we can put myBarData.open/high...to a hetergeneous container. Anyway the above way already has the following advantages:
					//**** [1] Avoid hard-coding of vector index, and hence eliminate of possible index shuffling when new element is added or removed. [2] Even though == "open" etc is still hard coded, it can avoid mistakes of, e.g., mismatch open and high etc.
				}
			}
			else{
				printf("something is wrong in update_historyVector().\n");
			}

			int reqId = -1;
			for (unsigned int i = 0; i < m_newtxtVector.size(); i++) //*****Note for a symbol I need search only once but not (*numDataOfSingleSymbol) times. However, searching so many times is safe by verify one by one. If this does not consume too much time as compared other process. Just keep it as is. 
			{
				if (manuals.symbol == m_newtxtVector[i])
				{
					reqId = i;
					break;
				}
			} //end of for loop.

			if (reqId >= 0 && reqId < m_newtxtVector.size())
			{
				bool b = true;
				try{ m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					m_traderVector.longBuy[reqId].symbol = manuals.symbol;
					m_traderVector.longBuy[reqId].stopLossLong = manuals.stopLossLong;
					m_traderVector.shortSell[reqId].stopLossShort = manuals.stopLossShort;
					m_traderVector.longBuy[reqId].algoString = algoStringLong;
					m_traderVector.shortSell[reqId].algoString = algoStringShort;
					//**** Note that the algoStringVector use the same critical section of traderVector. So whenever the appearance of this vector, it should be within the traderVector critical section. 

					if (manuals.totalPosition > 0) m_traderVector.longBuy[reqId].totalPosition = manuals.totalPosition;
					else if (manuals.totalPosition < 0)m_traderVector.shortSell[reqId].totalPosition = manuals.totalPosition;

					m_csTraderVector.LeaveCriticalSection();
				}
			}
		}//while (getline(i_dataFile, dataString, '\n'))
		i_dataFile.close();

		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		if (remove(m_manuals_OLD.c_str()) != 0)
		{
			printf("Error deleting file %s.\n", m_manuals_OLD.c_str());
		}
		else
		{
			printf("File %s successfully deleted", m_manuals_OLD.c_str());
		}

		int file_result;
		file_result = rename(m_manuals_NEW.c_str(), m_manuals_OLD.c_str()); //**** rename new.txt to old.txt, but not the opposite.
		if (file_result == 0)
		{
			printf("File name successfully changed from %s to %s.\n", m_manuals_NEW.c_str(), m_manuals_OLD.c_str());
		}
		else
		{
			printf("Error occured when changing file name from %s to %s.\n", m_manuals_NEW.c_str(), m_manuals_OLD.c_str());
		}

		std::ofstream o_new(m_manuals_NEW.c_str(), std::ios::app);
		for (unsigned int reqId = 0; reqId < m_newtxtVector.size(); reqId++)
		{
			bool b = true;
			try{ m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				int totalPosition = 0;
				if (m_traderVector.longBuy[reqId].totalPosition > 0) totalPosition = m_traderVector.longBuy[reqId].totalPosition;
				else if (m_traderVector.shortSell[reqId].totalPosition < 0) totalPosition = m_traderVector.shortSell[reqId].totalPosition; // feeding back poision should be in the same client?
				if (totalPosition != 0)
				{
					o_new << m_newtxtVector[reqId] << ',' << m_traderVector.longBuy[reqId].stopLossLong << ',' << m_traderVector.shortSell[reqId].stopLossShort << ',' << m_traderVector.longBuy[reqId].algoString << ',' << m_traderVector.shortSell[reqId].algoString << ','
						<< totalPosition << std::endl;
				}

				m_csTraderVector.LeaveCriticalSection();
			}
		}
		o_new.close();
		std::cout << "At " << tempString << ":" << " manuals successfully written to a new file " << m_manuals_NEW << std::endl;
		
	} //if (i_dataFile.is_open())

}

void  AllData::earningReportDate()
{
	//Part I Calculate zero time points for several critical days.
	time_t todayTimeZeroPoint, nextTradingDayZeroPoint, ealiestTradingDayZeroPoint, latestTradingDayZeroPoint;
	time_t now = time(0);// If today is not trading day due to running the app during week ends, then I may specify in a wrong way. However, this will not make any harm. It will be corrected next trading day. 
	struct tm *ts;
	ts = localtime(&now);
	ts->tm_hour = 0;
	ts->tm_min = 0;
	ts->tm_sec = 0;
	todayTimeZeroPoint = mktime(ts); // If today is not trading day due to running the app during week ends, then I may specify in a wrong way. However, this will not make any harm. It will be corrected next trading day.  

	//**** Calculate nextTradingDayZeroPoint
	int counter = 0;
	while (1)
	{
		time_t newTime = todayTimeZeroPoint + (counter + 1) * 86400;
		ts = localtime(&newTime);
		if (ts->tm_wday != 0 && ts->tm_wday != 6) //**** If I want to be more precise, I should include a list of market holidays, which is different year after year. 
		{
			nextTradingDayZeroPoint = newTime; //***** Here only for next business day, so I don't need a separate counter as in other places. 
			break;
		}
		counter = counter + 1;
	}

	//**** Calculate latestTradingDayZeroPoint;
	unsigned int index = m_traderParameters.i_periodIndex; //**** Note I cannot use zeroth element because that is special for SPY and the .i = -1 in default.
	unsigned int expectedHoldingTimeUnits = m_baseNum[index] * m_traderParameters.closingDateFactor; // I will hold stocks for at least these days. 
	unsigned int tradingDayCounter = 0;
	counter = 0;
	while (1)
	{
		time_t newTime = todayTimeZeroPoint + (counter + 1) * 86400;
		ts = localtime(&newTime);
		if (ts->tm_wday != 0 && ts->tm_wday != 6)
		{
			latestTradingDayZeroPoint = newTime;
			tradingDayCounter = tradingDayCounter + 1;
		}
		counter = counter + 1;
		if (tradingDayCounter >= expectedHoldingTimeUnits) break;
	}

	//**** Calculate ealiestTradingDayZeroPoint
	tradingDayCounter = 0;
	counter = 0;
	while (1)
	{
		time_t newTime = todayTimeZeroPoint - (counter + 1) * 86400; //**** Note minus sign
		ts = localtime(&newTime);
		if (ts->tm_wday != 0 && ts->tm_wday != 6)
		{
			ealiestTradingDayZeroPoint = newTime;
			tradingDayCounter = tradingDayCounter + 1;
		}
		counter = counter + 1;
		if (tradingDayCounter >= m_traderParameters.earliestTradingDayFactor) break;
	}

	//Part II  Specify which symbol is not suitable to open position today, and which position need close today. 
	std::ifstream i_dataFile(m_earningReportDate, std::ios_base::in);
	if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", m_earningReportDate.c_str());
	else
	{
		std::string dateString, symbolString;
		std::string lineString, columnString;
		std::vector<std::string> columnStringVector;
		std::stringstream date_ss, symbol_ss;
		while (getline(i_dataFile, lineString, '\n'))
		{
			std::stringstream   lineStream(lineString);
			columnStringVector.clear();
			while (getline(lineStream, columnString, ',')) // empty space is the delimir
			{
				columnStringVector.push_back(columnString);
			}

			time_t earningReportEpochTimeZeroPoint;
			if (columnStringVector.size() == 2) //***** Note here is hard-coded.
			{
				date_ss.clear(); symbol_ss.clear();
				date_ss.str(columnStringVector[0]); symbol_ss.str(columnStringVector[1]);
				date_ss >> dateString;
				symbol_ss >> symbolString;

				std::string yearString = dateString.substr(0, 4); //****IMPORTANT NOTES: start with first character (with index 0 but not 1). 4 is the length of the string, but not the index of the final character (like Python)
				std::string monthString;
				if (dateString.at(4) == '0') monthString = dateString.substr(5, 1); else  monthString = dateString.substr(4, 2); //**** This might be not necessary. atoi might have the ability of neglect zero before non-zero numbers. This is for 100% sure.
				std::string dayString;
				if (dateString.at(6) == '0') dayString = dateString.substr(7, 1); else  dayString = dateString.substr(6, 2);

				time_t now = time(0);
				struct tm *ts = localtime(&now); //**** I need initialize it before modify it later.
				ts->tm_year = atoi(yearString.c_str()) - 1900;
				ts->tm_mon = atoi(monthString.c_str()) - 1; //***** IMPORTANT. Without -1 there will be big problem.See the range of time variables below. mon runs from 0 to 11
				ts->tm_mday = atoi(dayString.c_str());		//***** IMPORTANT  With a -1 there will be big problem. See the range of time variables below. mday runs from 1 to 31
				ts->tm_hour = 0;
				ts->tm_min = 0;
				ts->tm_sec = 0;
				earningReportEpochTimeZeroPoint = mktime(ts); // Set a zero point makes things easier. 
				//**** Note that using a give date, we can calculated the week day from this method. However, there are many other ways from web. The current way is very good, as long as I notice the month range (0-11), etc.
			}


			int reqId = -1;
			for (unsigned int i = 0; i < m_newtxtVector.size(); i++) //*****Note for a symbol I need search only once but not (*numDataOfSingleSymbol) times. However, searching so many times is safe by verify one by one. If this does not consume too much time as compared other process. Just keep it as is. 
			{
				if (symbolString == m_newtxtVector[i])
				{
					reqId = i;
					break;
				}
			} //end of for loop.


			if (reqId >= 0 && reqId < m_newtxtVector.size())
			{
				bool b = true;
				try{ m_csTraderVector.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
				{
					if (earningReportEpochTimeZeroPoint >= ealiestTradingDayZeroPoint && earningReportEpochTimeZeroPoint <= latestTradingDayZeroPoint)
					{
						m_traderVector.shortSell[reqId].noSellShort = true;
						m_traderVector.longBuy[reqId].noBuyLong = true;

						time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						printf(" At %s, noBuyLong and noSellShort is set for symbol %s. \n", tempString.c_str(), m_newtxtVector[reqId].c_str());
					}

					if ((earningReportEpochTimeZeroPoint == (todayTimeZeroPoint + 86400) ) //**** Note sometimes earning is out before trading start that day.
						|| (earningReportEpochTimeZeroPoint > (todayTimeZeroPoint +86400) && earningReportEpochTimeZeroPoint < nextTradingDayZeroPoint) //**** This is for the case of, e.g., today is Friday, and report date is Saturday.
						|| (earningReportEpochTimeZeroPoint == nextTradingDayZeroPoint))
					{
						m_traderVector.longSell[reqId].needToClose = true;
						m_traderVector.shortBuy[reqId].needToClose = true;
						time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
						printf(" At %s, needToClose is set for symbol is set for symbol %s. \n", tempString.c_str(), m_newtxtVector[reqId].c_str());

					}

					m_csTraderVector.LeaveCriticalSection();
				}
			}
		}//while (getline(i_dataFile, dataString, '\n'))
		i_dataFile.close();
	} //if (i_dataFile.is_open())
}

void  AllData::update_historyVector()   //***** This is for re-start during trading hours. 
{
	int fileVectorSize = 0;
	std::ifstream i_dataFile(m_vectorL_NEW, std::ios_base::in);
	if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", m_vectorL_NEW.c_str());
	else
	{
		My_BarDataNormal myBarData;
		std::string lineString, columnString;
		std::vector<std::string> columnStringVector;  //***** Where will they be cleared again ??????

		while (getline(i_dataFile, lineString, '\n')){
			std::stringstream   lineStream(lineString);
			columnStringVector.clear();
			while (getline(lineStream, columnString, ','))
				columnStringVector.push_back(columnString);
 
			if (m_historyMap.size() > 0 && m_historyMap.size() == columnStringVector.size()){
				for (auto i = m_historyMap.begin(); i != m_historyMap.end(); ++i){
					if ((*i).first == "symbol"){
						myBarData.symbol = columnStringVector[(*i).second];
						break; //**** Here I only find symbol of SPY to calculate the length of data range.
					}
				}
			}
			else{
				printf("something is wrong in update_historyVector().\n");
			}

			if (myBarData.symbol == "SPY") //**** Note this arrangement need the first symbol in the file is SPY. Otherwise the databaseVector
				fileVectorSize = fileVectorSize + 1;
			else
				break;
		}//while (getline(i_dataFile, dataString, '\n'))
		i_dataFile.close();

		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		printf(" At %s, the number of days for SPY is calculated as %d. \n", tempString.c_str(), fileVectorSize);

	} //if (i_dataFile.is_open())

	bool b = true; //*****Note the critical section below is for absolutely thread safety for concurrent accessing of a traderStructureVector table, although this member function might not be run concurrently with other threads.
	try{ m_csVectorL.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
	{
		if (fileVectorSize > 0)
		{
			std::ifstream i_dataFile(m_vectorL_NEW, std::ios_base::in);
			if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", m_vectorL_NEW.c_str());
			else
			{
				My_BarDataNormal myBarData;
				std::string lineString, columnString;
				std::vector<std::string> columnStringVector;

				int lineNumber = -1;
				while (getline(i_dataFile, lineString, '\n'))  //***** i_dataFile is of std::ifstream type, which is different from the lineStream below.
				{
					lineNumber = lineNumber + 1;

					std::stringstream  lineStream(lineString);
					columnStringVector.clear();
					while (getline(lineStream, columnString, ','))
						columnStringVector.push_back(columnString);

					if (m_historyMap.size() > 0 && m_historyMap.size() == columnStringVector.size()){
						for (auto i = m_historyMap.begin(); i != m_historyMap.end(); ++i){
							if ((*i).first == "date") myBarData.date = columnStringVector[(*i).second];
							else if ((*i).first == "yearMonthDateString") myBarData.yearMonthDateString = columnStringVector[(*i).second];
							else if ((*i).first == "open") myBarData.open = std::stod(columnStringVector[(*i).second]);
							else if ((*i).first == "high") myBarData.high = std::stod(columnStringVector[(*i).second]);
							else if ((*i).first == "low") myBarData.low = std::stod(columnStringVector[(*i).second]);
							else if ((*i).first == "close") myBarData.close = std::stod(columnStringVector[(*i).second]);
							else if ((*i).first == "volume") myBarData.volume = std::stoi(columnStringVector[(*i).second]);
							else if ((*i).first == "lastDownloadTime") myBarData.lastDownloadTime = columnStringVector[(*i).second];
							else if ((*i).first == "symbol") myBarData.symbol = columnStringVector[(*i).second];
							//**** The above can become more efficient if we can put myBarData.open/high...to a hetergeneous container. Anyway the above way already has the following advantages:
							//**** [1] Avoid hard-coding of vector index, and hence eliminate of possible index shuffling when new element is added or removed. [2] Even though == "open" etc is still hard coded, it can avoid mistakes of, e.g., mismatch open and high etc.
						}
					}
					else{
						printf("something is wrong in update_historyVector().\n");
					}
					
					int reqId = -1;
					for (unsigned int i = 0; i < m_newtxtVector.size(); i++) //*****Note for a symbol I need search only once but not (*numDataOfSingleSymbol) times. However, searching so many times is safe by verify one by one. If this does not consume too much time as compared other process. Just keep it as is. 
					{
						if (myBarData.symbol == m_newtxtVector[i])
						{
							reqId = i;
							break;
						}
					} //end of for loop.

					int j;
					if (reqId >= 0 && reqId < m_newtxtVector.size()) //**** This is a double safety. It is redundant because the range of i above already guarantee that reqId has the same range.
					{
						int m_VectorSize = m_VectorL[0].size(); //**** Note the important points below. 
						//**** m_VectorL[0].size() is the number related to today's SPY download. It might not be same as the SPY download of earlier days, when the number of elements is given by the databaseVectorSize. 
						//***** Be very careful of the extra element in m_VectorL for SPY, which is set aside for today's day bar. 
						if (fileVectorSize == m_VectorSize) //**** due to different downloading times or accident from IB these supposed-same variable might be different. 
						{
							j = lineNumber %  fileVectorSize;
							//**** Be very careful of the problem of extra today's bar. 
							//**** A specific example: if in the database we have 0, 1, 2, 3, 4 | 5, 6, 7, 8, 9 |....each with 5-day historical data. However, note that the final day for each stock (e.g. 4, 9, etc.) are for allocated position for today's bar. 
							//**** According to the definition of j = ID% m_VectorSize, j = 0, 1,... m_VectorSize-1 (j cannot be bigger than  m_VectorSizee-1). And the index for today's bar is  m_VectorSize-1. We can never use the contents of  m_VectorSize-1 element in database to overwrite/update that of m_VectorL.
							//***** ***** ***** See comments below.

							//**** Here j cannot be m_VectorSiz-1 because this element is the last element which is set aside for today's bar. So if j = m_VectorSiz-1, then there is problem because this element always indicates a historical data bar not including today.
							//**** If we start app before trading session starts, an overwritten should be fine. But if in the middle of trading, such a overwritten is a disaster. 
							//**** ***** ***** As said before. Because we normally overwrite before session starts, so it is ok to use today's bar data to overwrite because anyway it is a default. Therefore, we can use 
							//**** ***** ***** " if (j >= 0 && j < m_VectorSize) " can cause no problems at all. 
							if (j >= 0 && j < m_VectorSize)  m_VectorL[reqId][j] = myBarData; //**** Note in OTC case, we have if (j >= 0 && j < (m_VectorSize-1) )  m_VectorL[reqId][j] = myBarData; 
						}
						else if (fileVectorSize > m_VectorSize)
						{
							//****Also use a specific example to understand. 
							int bias = fileVectorSize - m_VectorSize;
							j = lineNumber % fileVectorSize;
							if (j == 0)  m_VectorL[reqId][0] = myBarData; //**** This is a special treatment because the first element stores important information such as manualBuy/Sell.
							else if ((j - bias) > 0 && (j - bias) < m_VectorSize)  m_VectorL[reqId][j - bias] = myBarData;
							//**** j-bias > 0 because 0 is already assigned. For example, if bias = 2, then j=0 database element is assigned to zeroth element of m_Vector, j = 1 and 2 elements are discarded because j-bias not > 0. From j = 3, database element is assigned to first, 2nd... element of m_VectorL. 
							//**** also j-bias can also be = m_VectorSize - 1 because there is an extra element in database for today's bar. However we won't assign its value to the m_VectorL here. 
						}
						else if (fileVectorSize < m_VectorSize)
						{

							int bias = m_VectorSize - fileVectorSize;
							j = lineNumber % fileVectorSize;

							if (j == 0)	{ for (int J = 0; J < (bias + 1); J++)	if (J >= 0 && J < m_VectorSize) m_VectorL[reqId][J] = myBarData; } //**** This is a special treatment because the first element stores important information.
							else if (j > 0)
							{
								if ((j + bias) < m_VectorSize) m_VectorL[reqId][j + bias] = myBarData; //**** Use a special databaseVector and m_Vector to arrange the index. 
								else
								{
									time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
									printf("At %s, this is impossible No. 1. \n", tempString.c_str());
								}
							}
						}
					}
				}//while (getline(i_dataFile, dataString, '\n'))
				i_dataFile.close();
			} //if (i_dataFile.is_open())

			time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
			std::ofstream o_new(m_vectorL_NEW.c_str(), std::ios::app);
			bool b = true;
			try{ m_csVectorL.EnterCriticalSection(); }	catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
			{
				for (unsigned int i = 0; i < m_newtxtVector.size(); i++)
				{
					for (unsigned int j = 0; j < m_VectorL[i].size(); j++) //numDataOfSingleSymbol Actually this also include an extra element for today's bar, not just the number of historical data downloaded.
					{
						o_new << m_VectorL[i][j].date << ',' << m_VectorL[i][j].yearMonthDateString<< ',' << m_VectorL[i][j].open << ',' << m_VectorL[i][j].high << ',' << m_VectorL[i][j].low << ',' << m_VectorL[i][j].close << ','
							<< m_VectorL[i][j].volume << ',' << m_VectorL[i][j].lastDownloadTime << ',' << m_VectorL[i][j].symbol << std::endl;
					}
				}
				 m_csVectorL.LeaveCriticalSection();
			}
			o_new.close();

			printf("historical data have successfully been written to a new file after update.\n");
		} //if (fileVectorSize > 0)
		 m_csVectorL.LeaveCriticalSection();
	}
}

void AllData::historyVectorLoad_backTest(std::vector<std::vector<My_BarDataNormal>> &m_vector, std::string &FileName)
{
	int fileVectorSize = 0;
	std::ifstream i_dataFile(FileName, std::ios_base::in);
	if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", FileName.c_str());
	else
	{
		My_BarDataNormal myBarData;
		std::string lineString, columnString;
		std::vector<std::string> columnStringVector;  //***** Where will they be cleared again ??????

		while (getline(i_dataFile, lineString, '\n'))
		{
			std::stringstream   lineStream(lineString);
			columnStringVector.clear();
			while (getline(lineStream, columnString, ','))
				columnStringVector.push_back(columnString);

			if (m_historyMap.size() > 0 && m_historyMap.size() == columnStringVector.size()){
				for (auto i = m_historyMap.begin(); i != m_historyMap.end(); ++i){
					if ((*i).first == "symbol"){
						myBarData.symbol = columnStringVector[(*i).second];
						break; //**** Here I only find symbol of SPY to calculate the length of data range.
					}
				}
			}
			else{
				printf("something is wrong in update_historyVector().\n");
			}

			if (myBarData.symbol == "SPY") //**** Note this arrangement need the first symbol in the file is SPY. Otherwise the databaseVector
				fileVectorSize = fileVectorSize + 1;
			else
				break;
		}//while (getline(i_dataFile, dataString, '\n'))
		i_dataFile.close();

		time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
		printf(" At %s, the number of bars for SPY is calculated as %d. \n", tempString.c_str(), fileVectorSize);

	} //if (i_dataFile.is_open())

	if (fileVectorSize > 0)
	{
		std::vector<My_BarDataNormal> tempVector; 
		tempVector.clear();
		m_vector.clear();
		std::ifstream i_dataFile(FileName, std::ios_base::in);
		if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", FileName.c_str());
		else
		{
			My_BarDataNormal myBarData;
			std::string lineString, columnString;
			std::vector<std::string> columnStringVector;

			while (getline(i_dataFile, lineString, '\n'))  //***** i_dataFile is of std::ifstream type, which is different from the lineStream below.
			{
				std::stringstream  lineStream(lineString);
				columnStringVector.clear();
				while (getline(lineStream, columnString, ','))
					columnStringVector.push_back(columnString);

				if (m_historyMap.size() > 0 && m_historyMap.size() == columnStringVector.size()){
					for (auto i = m_historyMap.begin(); i != m_historyMap.end(); ++i){
						if ((*i).first == "date") myBarData.date = columnStringVector[(*i).second];
						else if ((*i).first == "yearMonthDateString") myBarData.yearMonthDateString = columnStringVector[(*i).second];
						else if ((*i).first == "open") myBarData.open = std::stod(columnStringVector[(*i).second]);
						else if ((*i).first == "high") myBarData.high = std::stod(columnStringVector[(*i).second]);
						else if ((*i).first == "low") myBarData.low = std::stod(columnStringVector[(*i).second]);
						else if ((*i).first == "close") myBarData.close = std::stod(columnStringVector[(*i).second]);
						else if ((*i).first == "volume") myBarData.volume = std::stoi(columnStringVector[(*i).second]);
						else if ((*i).first == "lastDownloadTime") myBarData.lastDownloadTime = columnStringVector[(*i).second];
						else if ((*i).first == "symbol") myBarData.symbol = columnStringVector[(*i).second];
						//**** The above can become more efficient if we can put myBarData.open/high...to a hetergeneous container. Anyway the above way already has the following advantages:
						//**** [1] Avoid hard-coding of vector index, and hence eliminate of possible index shuffling when new element is added or removed. [2] Even though == "open" etc is still hard coded, it can avoid mistakes of, e.g., mismatch open and high etc.
					}
				}
				else{
					printf("something is wrong in update_historyVector().\n");
				}


				tempVector.push_back(myBarData);
				if (tempVector.size() == fileVectorSize){
					m_vector.push_back(tempVector);
					tempVector.clear();
				}

			}//while (getline(i_dataFile, dataString, '\n'))

			i_dataFile.close();
		} //if (i_dataFile.is_open())

	
	} //if (fileVectorSize > 0)
	 m_csVectorL.LeaveCriticalSection();
}

void AllData::MessageLimitation()
{
	std::chrono::high_resolution_clock::time_point m_now, m_earlier; 
	while (1)
	{
		bool b = true;
		try{ m_csTimeDeque.EnterCriticalSection(); } catch (...) { printf("***CS*** %s %d\n", __FILE__, __LINE__);  b = false; } if (b)
		{
			if (!m_timeDeque.empty())
			{
				m_now = std::chrono::high_resolution_clock::now(); //***** now() is a static function and thus I can call without a high_resolution_clock instance.
				m_earlier = m_timeDeque.front();
				if (std::chrono::duration_cast<std::chrono::seconds > (m_now - m_earlier).count() >= 1) m_timeDeque.pop_front();
			}
			m_csTimeDeque.LeaveCriticalSection();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

