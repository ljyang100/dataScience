#include "startTest.h"
void StartTest::StartEntryPoint()
{
	a.m_mostRecentDate = false;
	a.initializeTraderParameters();  a.initializeOtherParameters();
	a.m_backTest = true; //**** IMPORTANT! This must true all the time for back testing.
	a.initializeSymbolVectors();
	a.initializeBaseVector("EMPTY"); a.initializeCurrentDataVector();  a.initializeOtherVectors();
	a.initializeBackTestVectors();
	CommonFunctions cfs(a);
	cfs.initialization();

	std::string tempString;
	std::cout << "Type YES if preparing historical data. " << std::endl;
	std::cin >> tempString;
	if (tempString == "YES"){
		int err = _beginthread(AllData::MessageLimitationStatic, 0, &a);  if (err < 0) { printf("Could not start new threads.\n"); } // This is for handling 50 message limitation. 
		HistoricalDataL historicalDataL(cfs, a);
		historicalDataL.connectToTWS();
		HistoricalDataNormal historicalDataS(cfs, a);
		historicalDataS.connectToTWS();

		a.m_timeNow = a.m_backTest_symbolSection_EndTime; //**** This specifies a particular date to end up with.
		a.m_numSymbolsToDownload = 1; a.m_historicalDataDone = false; historicalDataL.downloadHistoricalData_Single(); //****After this call, numDataOfSingleSymbolL will be set with a value. 
		a.L_BarVectorDefaul();
		time_t startTime = time(0), timePassed, nextHeartBeat = time(0);
		a.m_numSymbolsToDownload = a.m_newtxtVector.size(); a.m_historicalDataDone = false;
		err = _beginthread(HistoricalDataL::HistoricalDataStaticEntryPoint, 0, &historicalDataL); if (err < 0) { printf("Could not start new threads.\n"); }
		while (1){
			timePassed = time(0) - startTime;
			if (a.m_historicalDataDone) break;
			Sleep(100);
			if (time(0) > nextHeartBeat) { printf("I am waiting %d seconds for most historical data download done.\n", a.m_traderParameters.timeToContinue); nextHeartBeat = nextHeartBeat + 60; }
		}

		int lastCharacterIndex = a.m_vectorFileName_S_Test.size() - 1;
		for (unsigned int i = 0; i < a.m_numShortBarsPeriodTest; i++){
			std::string tempString = std::to_string(i);
			if (i == 0) a.m_vectorFileName_S_Test = a.m_vectorFileName_S_Test.insert(lastCharacterIndex - 3, tempString);
			else{
				a.m_vectorFileName_S_Test.erase(lastCharacterIndex - 3, 1);
				a.m_vectorFileName_S_Test = a.m_vectorFileName_S_Test.insert(lastCharacterIndex - 3, tempString);
			}

			a.m_timeNow = a.m_backTest_symbolSection_EndTime - (a.m_numShortBarsPeriodTest - 1 - i) * a.m_daysOfHistoricalDataS * 86000; //**** This is end time of a period. So -1 is necessary. 
			a.m_numSymbolsToDownload = 1; a.m_historicalDataDone = false; historicalDataS.downloadHistoricalData_Single(); //****After this call, numDataOfSingleSymbolL will be set with a value. 
			a.S_BarVectorDefaul();
			a.m_numSymbolsToDownload = a.m_newtxtVector.size(); a.m_historicalDataDone = false;
			historicalDataS.downloadHistoricalData_multiple_beforeTrading();
			while (1)
			{
				timePassed = time(0) - startTime;
				if (a.m_historicalDataDone) break;
				Sleep(100);
				if (time(0) > nextHeartBeat) { printf("I am waiting %d seconds for most historical data download done.\n", a.m_traderParameters.timeToContinue); nextHeartBeat = nextHeartBeat + 60; }
			}
			Sleep(15000);
		} //end of for loop.
	}
	else{
		a.m_VectorL.clear();
		a.historyVectorLoad_backTest(a.m_VectorL, a.m_vectorFileName_L_Test);
		int lastCharacterIndex = a.m_vectorFileName_S_Test.size() - 1;
		int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2; // Not accurate due to int division and no-trading holidays. But it is generally OK. 
		int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;

		for (unsigned int i = 0; i < a.m_numShortBarsPeriodTest; i++){
			std::string tempString = std::to_string(i);
			if (i == 0) a.m_vectorFileName_S_Test = a.m_vectorFileName_S_Test.insert(lastCharacterIndex - 3, tempString);
			else{
				a.m_vectorFileName_S_Test.erase(lastCharacterIndex - 3, 1);
				a.m_vectorFileName_S_Test = a.m_vectorFileName_S_Test.insert(lastCharacterIndex - 3, tempString);
			}
			a.m_VectorS_Test.clear();
			a.historyVectorLoad_backTest(a.m_VectorS_Test, a.m_vectorFileName_S_Test);
			int lastDayIndexToCalculateBaseVector_L = a.m_VectorL[0].size() - 1 - (a.m_numShortBarsPeriodTest - i) * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
			for (int baseVectorLastDayIndex = lastDayIndexToCalculateBaseVector_L; baseVectorLastDayIndex < (lastDayIndexToCalculateBaseVector_L + numTradingDaysInOnePeriod); baseVectorLastDayIndex++)
			{
				a.initializeCurrentDataVector(); //**** Every trading day, a.m_currentDataStructureVector need reset. 
				a.m_baseVectorLastDayIndex = baseVectorLastDayIndex;
				cfs.calculateBaseVector(); //**** for different trading day, baseVector need be recalculated. 
				a.m_tradingDayIndex = a.m_baseVectorLastDayIndex + 1;
				showAllTradingConditions();
				printf("hedge related current exposure is %f. \n", a.m_traderVector.longBuy[0].totalLongValue + a.m_traderVector.shortSell[0].totalShortValue);

				for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); ++reqId)
				{
					for (unsigned int smallBarIndex = 0; smallBarIndex < a.m_VectorS_Test[0].size(); smallBarIndex++) // **** using reqId = 0, a.m_VectorS_Test[reqId] because every reqId has the same number of small bars.
					{
						if (a.m_VectorL[reqId][a.m_tradingDayIndex].yearMonthDateString == a.m_VectorS_Test[reqId][smallBarIndex].yearMonthDateString //**** Make sure smallBars belong to the same trading day, and we are dealing with the same symbol.
							&& a.m_VectorL[reqId][a.m_tradingDayIndex].symbol == a.m_VectorS_Test[reqId][smallBarIndex].symbol) {

							if (a.m_VectorS_Test[reqId][smallBarIndex].open > 0)
								if (a.m_currentDataStructureVector[reqId].open <= 0) a.m_currentDataStructureVector[reqId].open = a.m_VectorS_Test[reqId][smallBarIndex].open;
							if (a.m_VectorS_Test[reqId][smallBarIndex].high > 0){
								if (a.m_currentDataStructureVector[reqId].high <= 0) a.m_currentDataStructureVector[reqId].high = a.m_VectorS_Test[reqId][smallBarIndex].high;
								else {
									if (a.m_VectorS_Test[reqId][smallBarIndex].high > a.m_currentDataStructureVector[reqId].high) a.m_currentDataStructureVector[reqId].high = a.m_VectorS_Test[reqId][smallBarIndex].high;
								}
							}
							if (a.m_VectorS_Test[reqId][smallBarIndex].low > 0){
								if (a.m_currentDataStructureVector[reqId].low <= 0) a.m_currentDataStructureVector[reqId].low = a.m_VectorS_Test[reqId][smallBarIndex].low;
								else {
									if (a.m_VectorS_Test[reqId][smallBarIndex].low < a.m_currentDataStructureVector[reqId].low) a.m_currentDataStructureVector[reqId].low = a.m_VectorS_Test[reqId][smallBarIndex].low;
								}
							}
							if (a.m_VectorS_Test[reqId][smallBarIndex].close > 0) a.m_lastDayClose[reqId] = a.m_VectorS_Test[reqId][smallBarIndex].close; //**** This make sure the close > 0 in case some of small bars has no >0 data. 

							a.m_currentDataStructureVector[reqId].close = a.m_VectorL[reqId][a.m_tradingDayIndex - 1].close; //**** yesterday's close
							a.m_currentDataStructureVector[reqId].ask = a.m_VectorS_Test[reqId][smallBarIndex].high; //**** This is only an approximation
							a.m_currentDataStructureVector[reqId].bid = a.m_VectorS_Test[reqId][smallBarIndex].low;

							if (a.m_currentDataStructureVector[reqId].open > 0 && a.m_currentDataStructureVector[reqId].high > 0 && a.m_currentDataStructureVector[reqId].low > 0
								&& a.m_currentDataStructureVector[reqId].close > 0 && a.m_currentDataStructureVector[reqId].ask > 0 && a.m_currentDataStructureVector[reqId].bid > 0
								&& a.m_VectorL[reqId][a.m_tradingDayIndex].close > 0){ //**** IMPORTANT. I need the final condition because the final close of smallBars is not the a.m_currentDataStructureVector[reqId].close

								//****For long
								if (a.m_traderVectorTest[reqId].shortOpened == false && a.m_traderVectorTest[reqId].longOpened == false){

									//***** No deletion. Note in some days the number of small bars are different due to holiday etc. So I cannot use fixed number to loop. However, also be careful the missing data on some days. 
									if (a.m_traderVector.longBuy[reqId].totalPosition == 0 && a.m_newtxtVector[reqId] != "SPY"){ //For backtest, require totalPosiiont = 0, not same as real trading case. 
										createBuyOrderLong(reqId, a.m_currentDataStructureVector[reqId], smallBarIndex);
									}
								}

								if (a.m_traderVectorTest[reqId].longOpened == true){

									if (a.m_traderVector.longBuy[reqId].averageCost > 0 && a.m_traderVector.longBuy[reqId].totalPosition > 0){
										createSellOrderLong(reqId, a.m_currentDataStructureVector[reqId], smallBarIndex); //**** There must be shares to sell because .totalPosition > 0 is checked above. 
									}
								}

								//**** For short
								if (a.m_traderVectorTest[reqId].shortOpened == false && a.m_traderVectorTest[reqId].longOpened == false){

									//***** No deletion. Note in some days the number of small bars are different due to holiday etc. So I cannot use fixed number to loop. However, also be careful the missing data on some days. 
									if (a.m_traderVector.shortSell[reqId].totalPosition == 0 && a.m_newtxtVector[reqId] != "SPY"){
										createSellOrderShort(reqId, a.m_currentDataStructureVector[reqId], smallBarIndex);
									}
								}

								if (a.m_traderVectorTest[reqId].shortOpened == true){

									if (a.m_traderVector.shortSell[reqId].averageCost > 0 && a.m_traderVector.shortSell[reqId].totalPosition < 0){
										createBuyOrderShort(reqId, a.m_currentDataStructureVector[reqId], smallBarIndex); //**** There must be shares to sell because .totalPosition > 0 is checked above. 
									}
								}
							}
						}
					}
				}
			}
		} //end of for loop.
	}
}

void StartTest::createBuyOrderLong(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex)
{
	//*****Note the i below is different from the usual i-index in m_VectorS. So later I need a different name.
	unsigned int i = a.m_traderParameters.i_periodIndex;
	cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "OPEN");

	if (a.m_traderVector.longBuy[reqId].longHedgeOK)
	{
		if (a.m_traderVector.longBuy[reqId].basicOpen)
		{
			if (a.m_traderVector.longBuy[reqId].relevantShares > 0 && a.m_traderVector.longBuy[reqId].calculatedBid > 0)
			{
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.longBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.longBuy[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5)
				{
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.longBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;

				//**** should not appear MR, TF in the following
				double commission = cfs.commissionValue(reqId, i, a.m_traderVector.longBuy[reqId].relevantShares);
				a.m_traderVector.longBuy[reqId].averageCost = (a.m_traderVector.longBuy[reqId].calculatedBid * a.m_traderVector.longBuy[reqId].relevantShares + commission) / a.m_traderVector.longBuy[reqId].relevantShares;
				a.m_traderVector.longBuy[reqId].totalPosition = a.m_traderVector.longBuy[reqId].relevantShares; //**** this relevantShares has been initialized into longBuy traderVectors. But I must populate totalPosition here in backtest. 
				double commissionClose = min(a.m_traderVector.longBuy[reqId].relevantShares * 0.005, a.m_traderVector.longBuy[reqId].relevantShares * a.m_traderVector.longBuy[reqId].calculatedBid * 0.005); //**** This is approximate! Because I cannot use .calculatedAsk.
				commissionClose = max(1.0, commissionClose);


				FinishedTradeStructure finishedTradeStructure;
				finishedTradeStructure.reqId = reqId;
				finishedTradeStructure.symbol = a.m_newtxtVector[reqId];
				finishedTradeStructure.longOrShort = "LONG";
				finishedTradeStructure.OpenedDateIndex = a.m_tradingDayIndex;
				int numSmallBarsEachDay = a.m_wholeDayTradeDuration / a.m_bardurationS;
				finishedTradeStructure.OpenedSmallBarIndex = smallBarsIndex % numSmallBarsEachDay;
				finishedTradeStructure.closedDateIndex = 0;
				finishedTradeStructure.closedSmallBarIndex = 0;
				finishedTradeStructure.opened = true;
				finishedTradeStructure.closed = false;
				finishedTradeStructure.algoString = a.m_traderVector.longBuy[reqId].algoString;
				finishedTradeStructure.conditions = a.m_traderVector.longBuy[reqId].conditions;
				finishedTradeStructure.sharesOpened = a.m_traderVector.longBuy[reqId].relevantShares;
				finishedTradeStructure.gain_loss = 0;
				finishedTradeStructure.commissionOpened = commission;
				finishedTradeStructure.commissionClosed = commissionClose;
				finishedTradeStructure.averageCost = a.m_traderVector.longBuy[reqId].averageCost;

				a.m_finishedTradesList.push_back(finishedTradeStructure);
				finishedTradeStructure = a.m_finishedTradeStructure_default;

				a.m_traderVectorTest[reqId].longOpened = true;
				a.m_traderVectorTest[reqId].longClosed = false;
				a.m_traderVectorTest[reqId].shortOpened = false;
				a.m_traderVectorTest[reqId].shortClosed = false;
				a.m_traderVectorTest[reqId].OpenedDateIndex = a.m_tradingDayIndex;
				a.m_traderVectorTest[reqId].smallBarIndex = smallBarsIndex % numSmallBarsEachDay;

				int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2;
				int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;
				int firstTradingDayIndex = a.m_VectorL[0].size() - a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
				a.m_myfileLong << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create buy order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.longBuy[reqId].calculatedBid << ":" << a.m_traderVector.longBuy[reqId].relevantShares
					<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
					<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;
				std::cout << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create buy order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.longBuy[reqId].calculatedBid << ":" << a.m_traderVector.longBuy[reqId].relevantShares
					<< " and conditions: " << a.m_traderVector.longBuy[reqId].conditions[0] << ":" << a.m_traderVector.longBuy[reqId].conditions[1] << ":" << a.m_traderVector.longBuy[reqId].conditions[2] << ":" << a.m_traderVector.longBuy[reqId].conditions[3] << ":" << a.m_traderVector.longBuy[reqId].conditions[4] << ":" << a.m_traderVector.longBuy[reqId].conditions[5]
					<< ":" << a.m_traderVector.longBuy[reqId].conditions[6] << ":" << a.m_traderVector.longBuy[reqId].conditions[7] << ":" << a.m_traderVector.longBuy[reqId].conditions[8] << ":" << a.m_traderVector.longBuy[reqId].conditions[9] << std::endl;

				std::string fileName = "C:/symbols/graphs/LongOpen_"; fileName.append(a.m_newtxtVector[reqId]);   fileName.append(".txt");
				int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
				numShortBarsConsidered = a.m_baseNum[i];
				if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;
				firstStartIndex = barIndex - numShortBarsConsidered + 1;
				loopFirstEndIndex = barIndex + 1;
				if (remove(fileName.c_str()) != 0) perror("Error deleting file");
				else puts("File successfully deleted");
				std::ofstream o_tempData1(fileName.c_str(), std::ios::app);
				double upValue, downValue, middleValue; 
				for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
				{
					if (a.m_VectorL[reqId][index].close > 0)
					{
						middleValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
						upValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].upTradingLineIntercept + (a.m_baseVector)[i][reqId].originY;
						downValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].downTradingLineIntercept + (a.m_baseVector)[i][reqId].originY;
						o_tempData1 << (index - firstStartIndex) << '\t' << a.m_VectorL[reqId][index].high << '\t' << middleValue << '\t' << a.m_VectorL[reqId][index].low << '\t' << upValue << '\t' << downValue << '\t' << a.m_VectorL[reqId][index].close << '\t' << a.m_traderVector.longBuy[reqId].calculatedBid << std::endl;
					}
				}

			}
		}
	}
}

void StartTest::createSellOrderLong(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;
	cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.longBuy[reqId].algoString, "CLOSE");

	if (a.m_traderVector.longSell[reqId].basicClose)
	{
		if (a.m_traderVector.longBuy[reqId].myBid > 0) a.m_traderVector.longSell[reqId].calculatedAsk = max(a.m_traderVector.longSell[reqId].calculatedAsk, (a.m_traderVector.longBuy[reqId].myBid + 2 * a.m_traderVector.longSell[reqId].minPriceVariation));
		int lmtPrice100IntNew = int(ceil(a.m_traderVector.longSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
		int priceLastDigit = lmtPrice100IntNew % 10;
		int minPriceVariationLastDigit = (int(a.m_traderVector.longSell[reqId].minPriceVariation * 100)) % 10;
		if (minPriceVariationLastDigit == 5)
		{
			if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
			else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
		}
		a.m_traderVector.longSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100;

		double commission = cfs.commissionValue(reqId, i, a.m_traderVector.longSell[reqId].relevantShares); //**** This is for open commission, but the relevantShares is same as the bot ones in back test. 
		double commissionClose = min(a.m_traderVector.longSell[reqId].relevantShares * 0.005, a.m_traderVector.longSell[reqId].relevantShares * a.m_traderVector.longSell[reqId].calculatedAsk * 0.005); //**** This is better approximate! Because I use .calculatedAsk.
		commissionClose = max(1.0, commissionClose);

		FinishedTradeStructure finishedTradeStructure;
		finishedTradeStructure.reqId = reqId;
		finishedTradeStructure.symbol = a.m_newtxtVector[reqId];
		finishedTradeStructure.longOrShort = "LONG";
		finishedTradeStructure.OpenedDateIndex = a.m_traderVectorTest[reqId].OpenedDateIndex;
		finishedTradeStructure.OpenedSmallBarIndex = a.m_traderVectorTest[reqId].smallBarIndex;
		finishedTradeStructure.closedDateIndex = a.m_tradingDayIndex;
		int numSmallBarsEachDay = a.m_wholeDayTradeDuration / a.m_bardurationS;
		finishedTradeStructure.closedSmallBarIndex = smallBarsIndex % numSmallBarsEachDay;;
		finishedTradeStructure.opened = true;
		finishedTradeStructure.closed = true;
		finishedTradeStructure.algoString = a.m_traderVector.longBuy[reqId].algoString;
		finishedTradeStructure.conditions = a.m_traderVector.longSell[reqId].conditions;
		finishedTradeStructure.sharesOpened = a.m_traderVector.longSell[reqId].relevantShares;
		finishedTradeStructure.gain_loss = a.m_traderVector.longSell[reqId].relevantShares * (a.m_VectorS_Test[reqId][smallBarsIndex].close - a.m_traderVector.longBuy[reqId].averageCost) - commissionClose;
		//**** For short, gainLoss should be calculated in a different way. 
		finishedTradeStructure.commissionOpened = commission;
		finishedTradeStructure.commissionClosed = commissionClose;
		finishedTradeStructure.averageCost = a.m_traderVector.longBuy[reqId].averageCost;
		a.m_finishedTradesList.push_back(finishedTradeStructure);


		int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2;
		int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;
		int firstTradingDayIndex = a.m_VectorL[0].size() - a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
		a.m_myfileLong << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create sell order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.longSell[reqId].calculatedAsk << ":" << a.m_traderVector.longSell[reqId].relevantShares
			<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
			<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;
		std::cout << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.longBuy[reqId].algoString << " Long: Create sell order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.longSell[reqId].calculatedAsk << ":" << a.m_traderVector.longSell[reqId].relevantShares
			<< " and conditions: " << a.m_traderVector.longSell[reqId].conditions[0] << ":" << a.m_traderVector.longSell[reqId].conditions[1] << ":" << a.m_traderVector.longSell[reqId].conditions[2] << ":" << a.m_traderVector.longSell[reqId].conditions[3] << ":" << a.m_traderVector.longSell[reqId].conditions[4] << ":" << a.m_traderVector.longSell[reqId].conditions[5]
			<< ":" << a.m_traderVector.longSell[reqId].conditions[6] << ":" << a.m_traderVector.longSell[reqId].conditions[7] << ":" << a.m_traderVector.longSell[reqId].conditions[8] << ":" << a.m_traderVector.longSell[reqId].conditions[9] << std::endl;

		//The following is also set to default.
		a.m_traderVectorTest[reqId].longOpened = false;
		a.m_traderVectorTest[reqId].longClosed = false;
		a.m_traderVectorTest[reqId].shortOpened = false;
		a.m_traderVectorTest[reqId].shortClosed = false;
		a.m_traderVectorTest[reqId].OpenedDateIndex = 0;
		a.m_traderVectorTest[reqId].smallBarIndex = 0;

		finishedTradeStructure = a.m_finishedTradeStructure_default;
		a.m_traderVector.longSell[reqId] = a.m_traderStructureDefault; 
		a.m_traderVector.longBuy[reqId] = a.m_traderStructureDefault; 
	}
}

void StartTest::createSellOrderShort(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex)
{
	//*****Note the i below is different from the usual i-index in m_VectorS. So later I need a different name.
	unsigned int i = a.m_traderParameters.i_periodIndex;

	cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "OPEN");

	if (a.m_traderVector.shortSell[reqId].shortHedgeOK)
	{
		if (a.m_traderVector.shortSell[reqId].basicOpen)
		{
			if (a.m_traderVector.shortSell[reqId].relevantShares > 0 && a.m_traderVector.shortSell[reqId].calculatedAsk > 2 * a.m_traderVector.shortSell[reqId].minPriceVariation)
			{
				int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortSell[reqId].calculatedAsk * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
				int priceLastDigit = lmtPrice100IntNew % 10;
				int minPriceVariationLastDigit = (int(a.m_traderVector.shortSell[reqId].minPriceVariation * 100)) % 10;
				if (minPriceVariationLastDigit == 5)
				{
					if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew + (10 - priceLastDigit);
					else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
				}
				a.m_traderVector.shortSell[reqId].calculatedAsk = (double)lmtPrice100IntNew / 100;

				//**** should not appear MR, TF in the following
				double commission = cfs.commissionValue(reqId, i, a.m_traderVector.shortSell[reqId].relevantShares);
				a.m_traderVector.shortSell[reqId].averageCost = (a.m_traderVector.shortSell[reqId].calculatedAsk * a.m_traderVector.shortSell[reqId].relevantShares + commission) / a.m_traderVector.shortSell[reqId].relevantShares;
				a.m_traderVector.shortSell[reqId].totalPosition = -((int)a.m_traderVector.shortSell[reqId].relevantShares); //**** this relevantShares has been initialized into longBuy traderVectors. But I must populate totalPosition here in backtest. 
				double commissionClose = min(a.m_traderVector.shortSell[reqId].relevantShares * 0.005, a.m_traderVector.shortSell[reqId].relevantShares * a.m_traderVector.shortSell[reqId].calculatedAsk * 0.005); //**** This is approximate! Because I cannot use .calculatedAsk.
				commissionClose = max(1.0, commissionClose);

				FinishedTradeStructure finishedTradeStructure;
				finishedTradeStructure.reqId = reqId;
				finishedTradeStructure.symbol = a.m_newtxtVector[reqId];
				finishedTradeStructure.longOrShort = "SHORT";
				finishedTradeStructure.OpenedDateIndex = a.m_tradingDayIndex;
				int numSmallBarsEachDay = a.m_wholeDayTradeDuration / a.m_bardurationS;
				finishedTradeStructure.OpenedSmallBarIndex = smallBarsIndex % numSmallBarsEachDay;
				finishedTradeStructure.closedDateIndex = 0;
				finishedTradeStructure.closedSmallBarIndex = 0;
				finishedTradeStructure.opened = true;
				finishedTradeStructure.closed = false;
				finishedTradeStructure.algoString = a.m_traderVector.shortSell[reqId].algoString;
				finishedTradeStructure.conditions = a.m_traderVector.shortSell[reqId].conditions;
				finishedTradeStructure.sharesOpened = a.m_traderVector.shortSell[reqId].relevantShares;
				finishedTradeStructure.gain_loss = 0;
				finishedTradeStructure.commissionOpened = commission;
				finishedTradeStructure.commissionClosed = commissionClose;
				finishedTradeStructure.averageCost = a.m_traderVector.shortSell[reqId].averageCost;

				a.m_finishedTradesList.push_back(finishedTradeStructure);
				finishedTradeStructure = a.m_finishedTradeStructure_default;

				a.m_traderVectorTest[reqId].longOpened = false;
				a.m_traderVectorTest[reqId].longClosed = false;
				a.m_traderVectorTest[reqId].shortOpened = true;
				a.m_traderVectorTest[reqId].shortClosed = false;
				a.m_traderVectorTest[reqId].OpenedDateIndex = a.m_tradingDayIndex;
				a.m_traderVectorTest[reqId].smallBarIndex = smallBarsIndex % numSmallBarsEachDay;
						
				int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2;
				int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;
				int firstTradingDayIndex = a.m_VectorL[0].size() - a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
				a.m_myfileShort << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short sell order for symbol " << a.m_newtxtVector[reqId] << " with price:totalPosition " << a.m_traderVector.shortSell[reqId].calculatedAsk << ":" << a.m_traderVector.shortSell[reqId].relevantShares
					<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
					<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;
				std::cout << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short sell order for symbol " << a.m_newtxtVector[reqId] << " with price:totalPosition " << a.m_traderVector.shortSell[reqId].calculatedAsk << ":" << a.m_traderVector.shortSell[reqId].relevantShares
					<< " and conditions: " << a.m_traderVector.shortSell[reqId].conditions[0] << ":" << a.m_traderVector.shortSell[reqId].conditions[1] << ":" << a.m_traderVector.shortSell[reqId].conditions[2] << ":" << a.m_traderVector.shortSell[reqId].conditions[3] << ":" << a.m_traderVector.shortSell[reqId].conditions[4] << ":" << a.m_traderVector.shortSell[reqId].conditions[5]
					<< ":" << a.m_traderVector.shortSell[reqId].conditions[6] << ":" << a.m_traderVector.shortSell[reqId].conditions[7] << ":" << a.m_traderVector.shortSell[reqId].conditions[8] << ":" << a.m_traderVector.shortSell[reqId].conditions[9] << std::endl;
			}
		}
	}
}


void StartTest::createBuyOrderShort(unsigned int &reqId, CurrentDataStructure &currentDataStructure, unsigned int &smallBarsIndex)
{
	unsigned int i = a.m_traderParameters.i_periodIndex;
	cfs.commonCode(reqId, i, currentDataStructure, a.m_traderVector.shortSell[reqId].algoString, "CLOSE");

	
	if (a.m_traderVector.shortBuy[reqId].basicClose)
	{
		if (a.m_traderVector.shortSell[reqId].myAsk > 0)	a.m_traderVector.shortBuy[reqId].calculatedBid = min(a.m_traderVector.shortBuy[reqId].calculatedBid, (a.m_traderVector.shortSell[reqId].myAsk - 2 * a.m_traderVector.shortBuy[reqId].minPriceVariation));
		int lmtPrice100IntNew = int(ceil(a.m_traderVector.shortBuy[reqId].calculatedBid * 100 - 0.499)); //Note the parameter 0.499 and the ceil function achieve the rounding ( using 0.5 as separating line). Here I also multiply the price by 100 so as to make the mininum 0.01 dollar to be 1 dollar and can be represented by an integer. 
		int priceLastDigit = lmtPrice100IntNew % 10;
		int minPriceVariationLastDigit = (int(a.m_traderVector.shortBuy[reqId].minPriceVariation * 100)) % 10;
		if (minPriceVariationLastDigit == 5)
		{
			if (priceLastDigit >= 5) lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit + 5;
			else lmtPrice100IntNew = lmtPrice100IntNew - priceLastDigit; //**** For buying, always try to lower the calcuatedBid. This is to avoid it will be bigger than .myAsk
		}
		a.m_traderVector.shortBuy[reqId].calculatedBid = (double)lmtPrice100IntNew / 100;
			
		double commission = cfs.commissionValue(reqId, i, a.m_traderVector.shortBuy[reqId].relevantShares);
		double commissionClose = min(a.m_traderVector.shortBuy[reqId].relevantShares * 0.005, a.m_traderVector.shortBuy[reqId].relevantShares * a.m_traderVector.shortBuy[reqId].calculatedAsk * 0.005); //**** This is better approximate! Because I use .calculatedAsk.
		commissionClose = max(1.0, commissionClose);

		FinishedTradeStructure finishedTradeStructure;
		finishedTradeStructure.reqId = reqId;
		finishedTradeStructure.symbol = a.m_newtxtVector[reqId];
		finishedTradeStructure.longOrShort = "SHORT";
		finishedTradeStructure.OpenedDateIndex = a.m_traderVectorTest[reqId].OpenedDateIndex;
		finishedTradeStructure.OpenedSmallBarIndex = a.m_traderVectorTest[reqId].smallBarIndex;
		finishedTradeStructure.closedDateIndex = a.m_tradingDayIndex;
		int numSmallBarsEachDay = a.m_wholeDayTradeDuration / a.m_bardurationS;
		finishedTradeStructure.closedSmallBarIndex = smallBarsIndex % numSmallBarsEachDay;;
		finishedTradeStructure.opened = true;
		finishedTradeStructure.closed = true;
		finishedTradeStructure.algoString = a.m_traderVector.shortSell[reqId].algoString;
		finishedTradeStructure.conditions = a.m_traderVector.shortBuy[reqId].conditions;
		finishedTradeStructure.sharesOpened = a.m_traderVector.shortBuy[reqId].relevantShares;
		finishedTradeStructure.gain_loss = a.m_traderVector.shortBuy[reqId].relevantShares * (a.m_traderVector.shortSell[reqId].averageCost - a.m_VectorS_Test[reqId][smallBarsIndex].close) - commissionClose;
		//**** For short, gainLoss should be calculated in a different way. 
		finishedTradeStructure.commissionOpened = commission;
		finishedTradeStructure.commissionClosed = commissionClose;
		finishedTradeStructure.averageCost = a.m_traderVector.shortSell[reqId].averageCost;
		a.m_finishedTradesList.push_back(finishedTradeStructure);


		int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2;
		int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;
		int firstTradingDayIndex = a.m_VectorL[0].size() - a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
		a.m_myfileShort << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short buy order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.shortBuy[reqId].calculatedBid << ":" << a.m_traderVector.shortBuy[reqId].relevantShares
			<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
			<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;

		std::cout << "Day: " << (a.m_tradingDayIndex - firstTradingDayIndex) << ":" << a.m_traderVector.shortSell[reqId].algoString << " Short: Create short buy order for symbol " << a.m_newtxtVector[reqId] << " with price: totalPosition " << a.m_traderVector.shortBuy[reqId].calculatedBid << ":" << a.m_traderVector.shortBuy[reqId].relevantShares
			<< " and conditions: " << a.m_traderVector.shortBuy[reqId].conditions[0] << ":" << a.m_traderVector.shortBuy[reqId].conditions[1] << ":" << a.m_traderVector.shortBuy[reqId].conditions[2] << ":" << a.m_traderVector.shortBuy[reqId].conditions[3] << ":" << a.m_traderVector.shortBuy[reqId].conditions[4] << ":" << a.m_traderVector.shortBuy[reqId].conditions[5]
			<< ":" << a.m_traderVector.shortBuy[reqId].conditions[6] << ":" << a.m_traderVector.shortBuy[reqId].conditions[7] << ":" << a.m_traderVector.shortBuy[reqId].conditions[8] << ":" << a.m_traderVector.shortBuy[reqId].conditions[9] << std::endl;

		//The following is also set to default.
		a.m_traderVectorTest[reqId].longOpened = false;
		a.m_traderVectorTest[reqId].longClosed = false;
		a.m_traderVectorTest[reqId].shortOpened = false;
		a.m_traderVectorTest[reqId].shortClosed = false;
		a.m_traderVectorTest[reqId].OpenedDateIndex = 0;
		a.m_traderVectorTest[reqId].smallBarIndex = 0;

		finishedTradeStructure = a.m_finishedTradeStructure_default;
		a.m_traderVector.shortSell[reqId] = a.m_traderStructureDefault; 
		a.m_traderVector.shortBuy[reqId] = a.m_traderStructureDefault; 
	}
}

void StartTest::summarize()
{
	std::list<FinishedTradeStructure>::iterator i, j;
	bool break1 = false;
	while (a.m_finishedTradesList.size() >= 2){
		for (i = a.m_finishedTradesList.begin(); i != std::prev(a.m_finishedTradesList.end(), 1); ++i){
			for (j = std::next(i, 1); j != a.m_finishedTradesList.end(); ++j){
				if ((*i).symbol == (*j).symbol && (*i).OpenedDateIndex == (*j).OpenedDateIndex && (*i).OpenedSmallBarIndex == (*j).OpenedSmallBarIndex)	{
					if ((*j).closed == true){
						i = a.m_finishedTradesList.erase(i); //**** Note here i is the next iterator following the one deleted.
						//**** i is always before j. Therefore, the closed = true case can only appear in the j. 
						//**** Also check the entry in details.docx: Can I remove element from list while iterating? There I can find why I use i = a.m_finishedTradesList.erase(i); 
						break;
					}
				}
				//***** Next sentence, (..,2) is for the last but one element, (...,1) is the last element. The following sentence should be within the j-loop (and hence i loop).
				if (i == std::prev(a.m_finishedTradesList.end(), 2) && j == std::prev(a.m_finishedTradesList.end(), 1)){
					break1 = true;
				}
			}
			if (break1) break;
		}
		if (break1)	break;
	}

	if (a.m_finishedTradesList.size() > 0){
		int numTotalTrades = a.m_finishedTradesList.size(), numClosedTrades = 0, numUnClosedTrades = 0;
			double closedGainLoss = 0, unClosedGainLoss = 0, totalGainLoss = 0, totalCommissionPaid = 0, totalLongGainLoss = 0, totalShortGainLoss = 0, totalMoneyTraded = 0;
		for (auto i = a.m_finishedTradesList.begin(); i != a.m_finishedTradesList.end(); ++i){
			if ((*i).closed == true) {
				numClosedTrades = numClosedTrades + 1;
				closedGainLoss = closedGainLoss + (*i).gain_loss;
				if ((*i).longOrShort == "LONG") totalLongGainLoss = totalLongGainLoss + (*i).gain_loss;
				if ((*i).longOrShort == "SHORT") totalShortGainLoss = totalShortGainLoss + (*i).gain_loss;
			}

			if ((*i).closed == false){
				numUnClosedTrades = numUnClosedTrades + 1;
				int reqId = (*i).reqId;
				if ((*i).averageCost > 0){
					if ((*i).longOrShort == "LONG"){
						totalLongGainLoss = totalLongGainLoss + a.m_lastDayClose[reqId] - (*i).averageCost - (*i).commissionClosed;
						unClosedGainLoss = unClosedGainLoss + a.m_lastDayClose[reqId] - (*i).averageCost - (*i).commissionClosed;

					}
					if ((*i).longOrShort == "SHORT"){
						totalShortGainLoss = totalShortGainLoss + (*i).averageCost - a.m_lastDayClose[reqId] - (*i).commissionClosed;
						unClosedGainLoss = unClosedGainLoss + (*i).averageCost - a.m_lastDayClose[reqId] - (*i).commissionClosed;
					}
				}
				else {
					a.m_myfileLong << "averageCost for symbol " << a.m_newtxtVector[reqId] << " is <= 0, which must be wrong. " << std::endl;
					std::cout << "averageCost for symbol " << a.m_newtxtVector[reqId] << " is <= 0, which must be wrong. " << std::endl;
				}
			}
			totalGainLoss = closedGainLoss + unClosedGainLoss;
			totalCommissionPaid = totalCommissionPaid + (*i).commissionOpened + (*i).commissionClosed; //Even not closed, I still add the approximate commissionClosed will be paid in the future.
			totalMoneyTraded = totalMoneyTraded + (*i).sharesOpened * (*i).averageCost;
		}

		double averageClosedGainLoss = 0, averageUnClosedGainLoss = 0, averageTotalGainLoss = 0, averageDailyMoneyTraded = 0, averageDailyGainLoss = 0, averageDailyPercentage = 0;
		if (numClosedTrades > 0) averageClosedGainLoss = closedGainLoss / numClosedTrades;
		if (numUnClosedTrades > 0) averageUnClosedGainLoss = unClosedGainLoss / numUnClosedTrades;
		if (numTotalTrades > 0) averageTotalGainLoss = totalGainLoss / numTotalTrades;
		int numWeekEndDays = (a.m_daysOfHistoricalDataS / 7) * 2; // Not accurate due to int division and no-trading holidays. But it is generally OK. 
		int numTradingDaysInOnePeriod = a.m_daysOfHistoricalDataS - numWeekEndDays;

		int totalTradingDays = a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod;
		if (totalTradingDays > 0){
			averageDailyMoneyTraded = totalMoneyTraded / totalTradingDays;
			averageDailyGainLoss = totalGainLoss / totalTradingDays;
			if (averageDailyMoneyTraded > 0) averageDailyPercentage = averageDailyGainLoss / averageDailyMoneyTraded;
		}
		a.m_myfileLong << " numUnclosedTrades:averageUnclosedGainLoss " << numUnClosedTrades << ":" << averageUnClosedGainLoss
			<< " numclosedTrades:aveageClosedGainLoss " << numClosedTrades << ":" << averageClosedGainLoss
			<< " numTotalTrades:averageTotalGainLoss " << numTotalTrades << ":" << averageTotalGainLoss
			<< " totalCommissionPaid is " << totalCommissionPaid
			<< std::endl;

		std::cout << " numUnclosedTrades:averageUnclosedGainLoss " << numUnClosedTrades << ":" << averageUnClosedGainLoss
			<< " numclosedTrades:aveageClosedGainLoss " << numClosedTrades << ":" << averageClosedGainLoss
			<< " numTotalTrades:averageTotalGainLoss " << numTotalTrades << ":" << averageTotalGainLoss
			<< " totalCommissionPaid is " << totalCommissionPaid
			<< std::endl;


		a.m_myfileLong << " TotalLongGainLoss:TotalShortGailLoss are " << totalLongGainLoss << ":" << totalShortGainLoss << std::endl;
		std::cout << " TotalLongGainLoss:TotalShortGailLoss are " << totalLongGainLoss << ":" << totalShortGainLoss << std::endl;
		a.m_myfileLong << " averageDailyMoneyTraded:averageDailyPercentage are " << averageDailyMoneyTraded << ":" << averageDailyPercentage << std::endl;
		std::cout << " averageDailyMoneyTraded:averageDailyPercentage are " << averageDailyMoneyTraded << ":" << averageDailyPercentage << std::endl;


		std::vector<FinishedTradeStructure> tempVector = *new(std::vector<FinishedTradeStructure>);
		int firstTradingDayIndex = a.m_VectorL[0].size() - a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod; //**** The trading day starts from next day. 
		int lastTradingDayIndex = firstTradingDayIndex + a.m_numShortBarsPeriodTest * numTradingDaysInOnePeriod - 1;
		for (int tradingDayIndex = firstTradingDayIndex; tradingDayIndex < (lastTradingDayIndex + 1); ++tradingDayIndex)
		{
			tempVector.clear();
			for (auto i = a.m_finishedTradesList.begin(); i != a.m_finishedTradesList.end(); ++i){
				if ((*i).OpenedDateIndex == tradingDayIndex) tempVector.push_back((*i));
			}
			numClosedTrades = 0; numUnClosedTrades = 0;
			closedGainLoss = 0; unClosedGainLoss = 0; totalGainLoss = 0;
			for (unsigned int i = 0; i < tempVector.size(); ++i){
				if (tempVector[i].closed == true) {
					numClosedTrades = numClosedTrades + 1;
					closedGainLoss = closedGainLoss + tempVector[i].gain_loss;
				}

				if (tempVector[i].closed == false){
					numUnClosedTrades = numUnClosedTrades + 1;
					int reqId = tempVector[i].reqId;
					if (tempVector[i].averageCost > 0){
						if (tempVector[i].longOrShort == "LONG")
							unClosedGainLoss = unClosedGainLoss + a.m_lastDayClose[reqId] - tempVector[i].averageCost - tempVector[i].commissionClosed;
						else if (tempVector[i].longOrShort == "SHORT")
							unClosedGainLoss = unClosedGainLoss + tempVector[i].averageCost - a.m_lastDayClose[reqId] - tempVector[i].commissionClosed;
					}
					else {
						a.m_myfileLong << "averageCost for symbol " << a.m_newtxtVector[reqId] << " is <= 0, which must be wrong. " << std::endl;
						std::cout << "averageCost for symbol " << a.m_newtxtVector[reqId] << " is <= 0, which must be wrong. " << std::endl;
					}
				}
				totalGainLoss = closedGainLoss + unClosedGainLoss;

			}
			double averageClosedGainLoss = 0, averageUnClosedGainLoss = 0, averageTotalGainLoss = 0;
			if (numClosedTrades > 0) averageClosedGainLoss = closedGainLoss / numClosedTrades;
			if (numUnClosedTrades > 0) averageUnClosedGainLoss = unClosedGainLoss / numUnClosedTrades;
			if (numTotalTrades > 0) averageTotalGainLoss = totalGainLoss / numTotalTrades;
			a.m_myfileLong << " At Day: " << (tradingDayIndex - firstTradingDayIndex + 1) << ", numUnclosedTrades:averageUnclosedGainLoss " << numUnClosedTrades << ":" << averageUnClosedGainLoss
				<< " numclosedTrades:aveageClosedGainLoss " << numClosedTrades << ":" << averageClosedGainLoss
				<< " numTotalTrades:averageTotalGainLoss " << numTotalTrades << ":" << averageTotalGainLoss
				<< std::endl;

			std::cout << " At Day: " << (tradingDayIndex - firstTradingDayIndex + 1) << ", numUnclosedTrades:averageUnclosedGainLoss " << numUnClosedTrades << ":" << averageUnClosedGainLoss
				<< " numclosedTrades:aveageClosedGainLoss " << numClosedTrades << ":" << averageClosedGainLoss
				<< " numTotalTrades:averageTotalGainLoss " << numTotalTrades << ":" << averageTotalGainLoss
				<< std::endl;

		}
	}
}

void StartTest::showAllTradingConditions()
{
	//**** Later if for several types of TFLongs for example, then we can use algoStringVector types to design vectors, but not as below where we use MrLong, TFLong.....
	std::vector<std::string> tempVector; tempVector.clear();
	std::vector<std::vector<std::string> > longSymbolsVectorBuy; longSymbolsVectorBuy.clear();
	std::vector<std::vector<std::string> > longSymbolsVectorSell; longSymbolsVectorSell.clear();
	std::vector<std::vector<std::string> > shortSymbolsVectorSell; shortSymbolsVectorSell.clear();
	std::vector<std::vector<std::string> > shortSymbolsVectorBuy; shortSymbolsVectorBuy.clear();

	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		longSymbolsVectorBuy.push_back(tempVector);
		longSymbolsVectorSell.push_back(tempVector);
		shortSymbolsVectorSell.push_back(tempVector);
		shortSymbolsVectorBuy.push_back(tempVector);
	}

	for (unsigned int j = 0; j < a.m_traderParameters.maxNumConditions; j++)
	{
		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)
		{
			if (a.m_traderVector.longBuy[reqId].conditions[j]) longSymbolsVectorBuy[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.longSell[reqId].conditions[j])longSymbolsVectorSell[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.shortSell[reqId].conditions[j]) shortSymbolsVectorSell[j].push_back(a.m_newtxtVector[reqId]);
			if (a.m_traderVector.shortBuy[reqId].conditions[j]) shortSymbolsVectorBuy[j].push_back(a.m_newtxtVector[reqId]);
		}
	}

	time_t now = time(0); struct tm  *ts; char buf[80];	ts = localtime(&now); strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", ts); std::string tempString = buf; //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);  // "ddd yyyy-mm-dd hh:mm:ss zzz" 
	for (unsigned int i = 0; i < a.m_traderParameters.maxNumConditions; i++)
	{
		if (longSymbolsVectorBuy[i].size() > 0)
		{
			printf(" The number of symbols satisfying longBuy[reqId].conditions.condition %d is %d, e.g. %s.\n", i, longSymbolsVectorBuy[i].size(), longSymbolsVectorBuy[i][0].c_str());
			a.m_myfileLong << tempString << "The number of symbols satisfying longBuy[reqId].conditions.condition " << i << " is " << longSymbolsVectorBuy[i].size() << " , e.g., " << longSymbolsVectorBuy[i][0] << std::endl;
		}

		if (longSymbolsVectorSell[i].size() > 0)
		{
			printf(" The number of symbols satisfying longSell[reqId].conditions.condition %d is %d, e.g. %s.\n", i, longSymbolsVectorSell[i].size(), longSymbolsVectorSell[i][0].c_str());
			a.m_myfileLong << tempString << "The number of symbols satisfying longSell[reqId].conditions.condition " << i << " is " << longSymbolsVectorSell[i].size() << " , e.g., " << longSymbolsVectorSell[i][0] << std::endl;
		}

		if (shortSymbolsVectorSell[i].size() > 0)
		{
			printf(" The number of symbols satisfying shortSell[reqId].conditions.condition %d is %d, e.g. %s.\n", i, shortSymbolsVectorSell[i].size(), shortSymbolsVectorSell[i][0].c_str());
			a.m_myfileLong << tempString << "The number of symbols satisfying shortSell[reqId].conditions.condition " << i << " is " << shortSymbolsVectorSell[i].size() << " , e.g., " << shortSymbolsVectorSell[i][0] << std::endl;
		}

		if (shortSymbolsVectorBuy[i].size() > 0)
		{
			printf(" The number of symbols satisfying shortBuy[reqId].conditions.condition %d is %d, e.g. %s.\n", i, shortSymbolsVectorBuy[i].size(), shortSymbolsVectorBuy[i][0].c_str());
			a.m_myfileLong << tempString << "The number of symbols satisfying shortBuy[reqId].conditions.condition " << i << " is " << shortSymbolsVectorBuy[i].size() << " , e.g., " << shortSymbolsVectorBuy[i][0] << std::endl;
		}
	}

}
