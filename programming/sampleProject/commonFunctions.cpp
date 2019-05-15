#include "commonFunctions.h"

void CommonFunctions::initialization()
{
	m_lineVector = *new(std::vector<LineStructure>); 
	m_lineVector.clear(); 
	m_lineStructure.numNonZeroData = 0;
	m_lineStructure.sumX = 0;
	m_lineStructure.averageX = 0;
	m_lineStructure.sumY = 0;
	m_lineStructure.averageY = 0;
	m_lineStructure.sumSquaresX = 0;
	m_lineStructure.sumSquaresXY = 0;
	m_lineStructure.slope = 12345;
	m_lineStructure.intercept = 12345;
	m_lineStructure.firstCalculation = true; //default should be true;
	m_lineStructure.originX = 0;
	m_lineStructure.originY = 0;
	for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++)  m_lineVector.push_back(m_lineStructure);
}
bool CommonFunctions::commission(unsigned int &reqId, unsigned int &index, unsigned int &relevantShares, double &expectedGain)
{
	double moneyPerOrder = a.m_traderParameters.minMoneyPerOrderPercentage *  (a.m_baseVector)[index][reqId].averageMoneyVolume;
	moneyPerOrder = (moneyPerOrder > a.m_traderParameters.standardMoneyPerOrder) ? a.m_traderParameters.standardMoneyPerOrder : moneyPerOrder;
	moneyPerOrder = (moneyPerOrder < a.m_traderParameters.minMoneyPerOrder) ? a.m_traderParameters.minMoneyPerOrder : moneyPerOrder;
	double commissionRatio = 1000;
	double commission = min(relevantShares * 0.005, moneyPerOrder * 0.005);
	commission = max(1.0, commission);
	if (relevantShares > 0 && expectedGain > 0) commissionRatio = commission / (expectedGain * relevantShares);
	bool commissionState = false;
	commissionState = (commissionRatio <= a.m_traderParameters.acceptableCommissionRatio);
	return commissionState;
	//**** This part should mainly done in testFeed? And should be first consider in the calculation here, in order to waste time if it is not satisfied? 
}

double CommonFunctions::commissionValue(unsigned int &reqId, unsigned int &index, unsigned int &relevantShares)
{
	double moneyPerOrder = a.m_traderParameters.minMoneyPerOrderPercentage *  (a.m_baseVector)[index][reqId].averageMoneyVolume;
	moneyPerOrder = (moneyPerOrder > a.m_traderParameters.standardMoneyPerOrder) ? a.m_traderParameters.standardMoneyPerOrder : moneyPerOrder;
	moneyPerOrder = (moneyPerOrder < a.m_traderParameters.minMoneyPerOrder) ? a.m_traderParameters.minMoneyPerOrder : moneyPerOrder;
	double commissionRatio = 1000;
	double commission = min(relevantShares * 0.005, moneyPerOrder * 0.005);
	commission = max(1.0, commission);
	return commission;
	//**** This part should mainly done in testFeed? And should be first consider in the calculation here, in order to waste time if it is not satisfied? 
}

bool CommonFunctions::commonCode(unsigned int &reqId, unsigned int &index, CurrentDataStructure &currentDataStructure, std::string &algoString, std::string openOrClose)
{
	//*****Part I:  Hedging: This part should not be run when closing. The following if structure should be extended when more algorithms are added. 
	if (openOrClose == "OPEN")
	{
		double sumValue = 0;
		for (unsigned int reqIdTemp = 0; reqIdTemp < a.m_newtxtVector.size(); reqIdTemp++) //**** The whole loop should be within the critical section. I need the total sum of whole loop before judging whether it is hedged or not., 
		{
			if (a.m_traderVector.longBuy[reqIdTemp].totalPosition > 0 && a.m_traderVector.longBuy[reqIdTemp].stopLossLong > 0 && a.m_traderVector.longBuy[reqIdTemp].stopLossLong != a.m_noStopOrder)
				sumValue = sumValue + a.m_traderVector.longBuy[reqIdTemp].stopLossLong * a.m_traderVector.longBuy[reqIdTemp].totalPosition;
		}
		a.m_traderVector.longBuy[0].totalLongValue = sumValue;
		if ((a.m_traderVector.shortSell[0].totalShortValue + a.m_traderVector.longBuy[0].totalLongValue - a.m_traderParameters.exposureVol) < a.m_traderParameters.exposureMean)
			a.m_traderVector.longBuy[reqId].longHedgeOK = true;
		else
			a.m_traderVector.longBuy[reqId].longHedgeOK = false;

		sumValue = 0; //**** The following loop will significantly increase the calculation burden. I may use other ways to calculate the sumValue. For example, whenevern I submit a opening order or when an opening order is acknowledge, then I add a value to the sum. If cancelled, subtract it. I may use the following loop for accurate only every minute. This is for future. 
		for (unsigned int reqIdTemp = 0; reqIdTemp < a.m_newtxtVector.size(); reqIdTemp++) //**** The whole loop should be within the critical section. I need the total sum of whole loop before judging whether it is hedged or not., 
		{
			if (a.m_traderVector.shortSell[reqIdTemp].totalPosition < 0 && a.m_traderVector.shortSell[reqIdTemp].stopLossShort > 0 && a.m_traderVector.shortSell[reqIdTemp].stopLossShort != a.m_noStopOrder)
				sumValue = sumValue + abs(a.m_traderVector.shortSell[reqIdTemp].stopLossShort * a.m_traderVector.shortSell[reqIdTemp].totalPosition); //****For slow trading, only real position is counted.
		}
		a.m_traderVector.shortSell[0].totalShortValue = -sumValue;
		if ((a.m_traderVector.shortSell[0].totalShortValue + a.m_traderVector.longBuy[0].totalLongValue + a.m_traderParameters.exposureVol) > a.m_traderParameters.exposureMean)
			a.m_traderVector.shortSell[reqId].shortHedgeOK = true;
		else
			a.m_traderVector.shortSell[reqId].shortHedgeOK = false;
	}


	//**** Part III
	unsigned int size, x;
	double realBid = min(currentDataStructure.ask, currentDataStructure.bid), realAsk = max(currentDataStructure.ask, currentDataStructure.bid); //**** Sometimes the ASK and BID can be wrong from IB data.
	double buyPrice, sellPrice;
	double spreadPercentage = (realAsk - realBid) / realBid;
	if (spreadPercentage <= 0.0012){ //**** Take $1000 for example, taking liquidity will cost around extra of $1 to $1.2 but can fill the order quicker. 
		buyPrice = realAsk; 
		sellPrice = realBid;
	}
	else if (spreadPercentage > 0.0012 && spreadPercentage <= 0.01){ //**** If these affect the filling of order, modify it. 
		buyPrice = realBid + (realAsk - realBid) * 0.5;
		sellPrice = realAsk - (realAsk - realBid) * 0.5;
	}
	else if (spreadPercentage > 0.01){
		buyPrice = realBid; 
		sellPrice = realAsk;
	}

	if (a.m_backTest) size = a.m_baseVectorLastDayIndex + 1; else size = a.m_VectorL[reqId].size(); // From elsewhere: if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1; 
	x = size - 1;
	double upVolatility = (a.m_baseVector)[index][reqId].upTradingLineIntercept - (a.m_baseVector)[index][reqId].intercept; //**** always >= 0.
	double downVolatility = (a.m_baseVector)[index][reqId].intercept - (a.m_baseVector)[index][reqId].downTradingLineIntercept; //**** always >= 0.
	double totalTrendingAmount = (a.m_baseVector)[index][reqId].slope * (a.m_baseNum[index] - 1); //**** Note the trendingUpAmount can be negative if slope is negative.
	double numUpVolatilityTrending = 0, numDownVolatilityTrending = 0; //**** Can be positive, zero, or negative
	if (upVolatility > 0) numUpVolatilityTrending = totalTrendingAmount / upVolatility; //**** if positive, really trending up.  
	if (downVolatility > 0) numDownVolatilityTrending = totalTrendingAmount / downVolatility;

	if (a.m_backTest) size = a.m_baseVectorLastDayIndex + 1; else size = a.m_VectorL[reqId].size(); // From elsewhere: if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1; 
	x = size + a.m_baseNum[index] * a.m_traderParameters.closingDateFactor;  //**** expected closing date. Need some time to realize gain. The closingDateFactor can be optmized using backtesting.
	double expectedMiddlePrice = (a.m_baseVector)[index][reqId].slope * (x - (a.m_baseVector)[index][reqId].originX) + (a.m_baseVector)[index][reqId].intercept + (a.m_baseVector)[index][reqId].originY;
	double expectedMiddlePrice_ex = (a.m_baseVector)[index + 1][reqId].slope * (x - (a.m_baseVector)[index + 1][reqId].originX) + (a.m_baseVector)[index + 1][reqId].intercept + (a.m_baseVector)[index + 1][reqId].originY;
	double expectedMiddlePrice_na = (a.m_baseVector)[index - 1][reqId].slope * (x - (a.m_baseVector)[index - 1][reqId].originX) + (a.m_baseVector)[index - 1][reqId].intercept + (a.m_baseVector)[index - 1][reqId].originY;
	x = size; //****today's index
	double expectedMiddlePriceToday = (a.m_baseVector)[index][reqId].slope * (x - (a.m_baseVector)[index][reqId].originX) + (a.m_baseVector)[index][reqId].intercept + (a.m_baseVector)[index][reqId].originY;
	double expectedMiddlePriceToday_ex = (a.m_baseVector)[index + 1][reqId].slope * (x - (a.m_baseVector)[index + 1][reqId].originX) + (a.m_baseVector)[index + 1][reqId].intercept + (a.m_baseVector)[index + 1][reqId].originY;
	double expectedMiddlePriceToday_na = (a.m_baseVector)[index - 1][reqId].slope * (x - (a.m_baseVector)[index - 1][reqId].originX) + (a.m_baseVector)[index - 1][reqId].intercept + (a.m_baseVector)[index - 1][reqId].originY;
	double todayLowBoundary, todayHighBoundary;
	int minIndex = (a.m_baseVector)[0][reqId].minIndex, maxIndex = (a.m_baseVector)[0][reqId].maxIndex; //**** IMPORTANT: Here I only consider short-range min/max, normally 1/4 of the total range considered. 

	if (a.m_traderVector.longBuy[reqId].algoString == a.m_algoString[0].LONG)
	{
		//**** Add codes that compare today's price with high of a few days ago. This can increase the opportunity.
		//**** longer term indicator cannot be "STRONGLY" diffferent from that of one-month indicator.
		if (openOrClose == "OPEN"){
			if (numDownVolatilityTrending >= (-a.m_traderParameters.lowTrendingLimit) && numDownVolatilityTrending <= a.m_traderParameters.highTrendingLimit) {
				double expectedGain = downVolatility * a.m_volatilityFactorLong;
				a.m_traderVector.longBuy[reqId].calculatedBid = buyPrice;

				double calculatedGain = expectedMiddlePrice - a.m_traderVector.longBuy[reqId].calculatedBid;
				a.m_traderVector.longBuy[reqId].conditions[0] = (calculatedGain >= expectedGain) && (expectedGain > 0); //**** This is the fundamental condition. If not satisfied, do not go on to next step. Save calculaiton time.
				if (a.m_traderVector.longBuy[reqId].conditions[0]){
					a.m_traderVector.longBuy[reqId].relevantShares = generalSharesLong(index, reqId, "sharesToBuy", a.m_traderVector.longBuy[reqId].calculatedBid); //**** IMPORTANT. This must be here, otherwise commission()'s parameter is not right.
					a.m_traderVector.longBuy[reqId].conditions[1] = commission(reqId, index, a.m_traderVector.longBuy[reqId].relevantShares, calculatedGain);
					if (a.m_traderVector.longBuy[reqId].conditions[1]){ //**** nested logical structure to save calculation time.
						a.m_traderVector.longBuy[reqId].stopLossLong = calculatedGain * a.m_traderParameters.stopLossFactor; //**** This cannot put into the end. 
						todayLowBoundary = expectedMiddlePriceToday - expectedGain;
						a.m_traderVector.longBuy[reqId].conditions[2] = (a.m_traderVector.longBuy[reqId].calculatedBid <= todayLowBoundary);
						if (a.m_traderVector.longBuy[reqId].conditions[2]){
							bool startToMoveUp = false, notUpLot = false;
							double virtualLow = 0;
							if (minIndex >= 0 ){
								if (a.m_baseVector[0][minIndex].min > 0) //**** Be careful, I am using the [0]th element of the baseVector. 
									virtualLow = min(currentDataStructure.low, a.m_baseVector[0][minIndex].min);
							}
							if (virtualLow > 0) {
								if (a.m_traderVector.longBuy[reqId].opening == false){
									startToMoveUp = (a.m_traderVector.longBuy[reqId].calculatedBid >= virtualLow + calculatedGain * (a.m_traderParameters).reversingRequiredFactor);
									notUpLot = (a.m_traderVector.longBuy[reqId].calculatedBid <= virtualLow + calculatedGain * (a.m_traderParameters).reversingRequiredFactorMax);
								}
								else{ //**** leaving a room for not modifying order
									startToMoveUp = (a.m_traderVector.longBuy[reqId].calculatedBid >= virtualLow + calculatedGain * (a.m_traderParameters).reversingRequiredFactorEX);
									notUpLot = (a.m_traderVector.longBuy[reqId].calculatedBid <= virtualLow + calculatedGain * (a.m_traderParameters).reversingRequiredFactorMaxEX);
									if ((!startToMoveUp) || (!notUpLot)) a.m_traderVector.longBuy[reqId].priceChanged = true;
								}
								a.m_traderVector.longBuy[reqId].conditions[3] = startToMoveUp && notUpLot;
							}
							a.m_traderVector.longBuy[reqId].basicOpen = a.m_traderVector.longBuy[reqId].conditions[0] && a.m_traderVector.longBuy[reqId].conditions[1]
								&& a.m_traderVector.longBuy[reqId].conditions[2] && a.m_traderVector.longBuy[reqId].conditions[3];

						}
					}
				}
			}
			return a.m_traderVector.longBuy[reqId].basicOpen;
		}
		else if (openOrClose == "CLOSE"){
			if (a.m_traderVector.longBuy[reqId].stopLossLong > 0 && a.m_traderVector.longBuy[reqId].averageCost > 0){
				double expectedGain = a.m_traderVector.longBuy[reqId].stopLossLong / a.m_traderParameters.stopLossFactor;
				a.m_traderVector.longSell[reqId].calculatedAsk = sellPrice;
				bool normalGainExpected = ((a.m_traderVector.longSell[reqId].calculatedAsk - a.m_traderVector.longBuy[reqId].averageCost) >= expectedGain);
				bool bigGainExpected = ((currentDataStructure.bid - a.m_traderVector.longBuy[reqId].averageCost) >= (expectedGain * a.m_traderParameters.bigProfitFactor));
				if (bigGainExpected) a.m_traderVector.longSell[reqId].calculatedAsk = currentDataStructure.bid; //**** This is to guarantee a big profit is realized in big moving moment such as session opening.
				a.m_traderVector.longSell[reqId].conditions[0] = normalGainExpected || bigGainExpected;

				//Stopped out
				if (a.m_traderVector.longSell[reqId].calculatedAsk <= (a.m_traderVector.longBuy[reqId].averageCost - a.m_traderVector.longBuy[reqId].stopLossLong)){
					a.m_traderVector.longSell[reqId].calculatedAsk = realAsk - (realAsk - realBid) * 0.5;
					a.m_traderVector.longSell[reqId].conditions[1] = true;
				}

				//**** Due to earning release etc. 
				if (a.m_traderVector.longSell[reqId].needToClose) a.m_traderVector.longSell[reqId].conditions[2] = true;

				//*** Close on emergent situation
				if (a.m_closeLongPositions){ 
					a.m_traderVector.longSell[reqId].calculatedAsk = (realAsk + realBid) / 2;
					a.m_traderVector.longSell[reqId].conditions[3] = true;
				}

				a.m_traderVector.longSell[reqId].basicClose = a.m_traderVector.longSell[reqId].conditions[0] || a.m_traderVector.longSell[reqId].conditions[1] || a.m_traderVector.longSell[reqId].conditions[2]
					|| a.m_traderVector.longSell[reqId].conditions[3];
				a.m_traderVector.longSell[reqId].relevantShares = generalSharesLong(index, reqId, "sharesToSell", a.m_traderVector.longSell[reqId].calculatedAsk); //***** Unlike in opening case where shares are calculated in the beginning, here in closing case we calculate finally because we need calculatedAsk.

				if (a.m_traderVector.longSell[reqId].basicClose == true){
					bool readyToOpen = commonCode(reqId, index, currentDataStructure, algoString, "OPEN"); //**** This is recursive calling. Put it in the very end in order to save unnecessary calling
					if (readyToOpen) a.m_traderVector.longSell[reqId].basicClose = false; //**** Prevent flip-floping
				}

				a.m_traderVector.longSell[reqId].priceChanged = (abs(a.m_traderVector.longSell[reqId].myAsk - a.m_traderVector.longSell[reqId].calculatedAsk) >= a.m_traderVector.longSell[reqId].stopLossLong * a.m_traderParameters.priceUncertainty);
				if (a.m_traderVector.longSell[reqId].numModifiedSell > a.m_traderParameters.maxModificationsAllowed) a.m_traderVector.longSell[reqId].priceChanged = false;
			}
			return a.m_traderVector.longSell[reqId].basicClose;
		}
	}
	else if (a.m_traderVector.shortSell[reqId].algoString == a.m_algoString[0].SHORT){
		if (openOrClose == "OPEN"){
			if (numUpVolatilityTrending >= (-a.m_traderParameters.highTrendingLimit) && numUpVolatilityTrending <= a.m_traderParameters.lowTrendingLimit){
				double expectedGain = upVolatility * a.m_volatilityFactorShort;
				a.m_traderVector.shortSell[reqId].calculatedAsk = sellPrice;

				double calculatedGain = a.m_traderVector.shortSell[reqId].calculatedAsk - expectedMiddlePrice;
				a.m_traderVector.shortSell[reqId].conditions[0] = (calculatedGain >= expectedGain) && (expectedGain > 0); //**** This is the fundamental condition. If not satisfied, do not go on to next step. Save calculaiton time.
				if (a.m_traderVector.shortSell[reqId].conditions[0]){
					a.m_traderVector.shortSell[reqId].relevantShares = generalSharesShort(index, reqId, "sharesToSell", a.m_traderVector.shortSell[reqId].calculatedAsk); //**** IMPORTANT. This must be here, otherwise commission()'s parameter is not right.
					a.m_traderVector.shortSell[reqId].conditions[1] = commission(reqId, index, a.m_traderVector.shortSell[reqId].relevantShares, calculatedGain);
					if (a.m_traderVector.shortSell[reqId].conditions[1]){ //**** nested logical structure to save calculation time.
						a.m_traderVector.shortSell[reqId].stopLossShort = calculatedGain * a.m_traderParameters.stopLossFactor; //**** This cannot put into the end. 
						todayHighBoundary = expectedMiddlePriceToday + expectedGain;
						a.m_traderVector.shortSell[reqId].conditions[2] = (a.m_traderVector.shortSell[reqId].calculatedAsk >= todayHighBoundary);
						if (a.m_traderVector.shortSell[reqId].conditions[2]){

							bool startToMoveDown = false, notDownLot = false;
							double virtualHigh = 0;
							if (maxIndex >= 0){
								if (a.m_baseVector[0][maxIndex].max > 0){
									virtualHigh = max(a.m_baseVector[0][maxIndex].max, currentDataStructure.high);
								}
							}
							if (virtualHigh > 0){
								if (a.m_traderVector.shortSell[reqId].opening == false){
									startToMoveDown = (a.m_traderVector.shortSell[reqId].calculatedAsk <= (virtualHigh - calculatedGain * (a.m_traderParameters).reversingRequiredFactor));
									notDownLot = (a.m_traderVector.shortSell[reqId].calculatedAsk >= (virtualHigh - calculatedGain * (a.m_traderParameters).reversingRequiredFactorMax));
								}
								else{
									startToMoveDown = (a.m_traderVector.shortSell[reqId].calculatedAsk <= (virtualHigh - calculatedGain * (a.m_traderParameters).reversingRequiredFactorEX));
									notDownLot = (a.m_traderVector.shortSell[reqId].calculatedAsk >= (virtualHigh - calculatedGain * (a.m_traderParameters).reversingRequiredFactorMaxEX));
									if ((!startToMoveDown) || (!notDownLot)) a.m_traderVector.shortSell[reqId].priceChanged = true;
								}
								a.m_traderVector.shortSell[reqId].conditions[3] = startToMoveDown && notDownLot;
							}
						}
						a.m_traderVector.shortSell[reqId].basicOpen = a.m_traderVector.shortSell[reqId].conditions[0] && a.m_traderVector.shortSell[reqId].conditions[1]
							&& a.m_traderVector.shortSell[reqId].conditions[2] && a.m_traderVector.shortSell[reqId].conditions[3];
					}
				}
			}
			return a.m_traderVector.shortSell[reqId].basicOpen;
		}
		else if (openOrClose == "CLOSE"){
			if (a.m_traderVector.shortSell[reqId].stopLossShort > 0 && a.m_traderVector.shortSell[reqId].averageCost > 0){
				double expectedGain = a.m_traderVector.shortSell[reqId].stopLossShort / a.m_traderParameters.stopLossFactor;
				a.m_traderVector.shortBuy[reqId].calculatedBid = buyPrice;
				//if ((currentDataStructure.ask - currentDataStructure.last) / currentDataStructure.last > a.m_traderParameters.deviatedSpreadFactor)

				bool normalGainExpected = ((a.m_traderVector.shortSell[reqId].averageCost - a.m_traderVector.shortBuy[reqId].calculatedBid) >= expectedGain);
				bool bigGainExpected = ((a.m_traderVector.shortSell[reqId].averageCost - currentDataStructure.ask) >= (expectedGain * a.m_traderParameters.bigProfitFactor));
				if (bigGainExpected) a.m_traderVector.shortBuy[reqId].calculatedBid = currentDataStructure.ask;
				a.m_traderVector.shortBuy[reqId].conditions[0] = (normalGainExpected || bigGainExpected);

				if (a.m_traderVector.shortBuy[reqId].calculatedBid >= (a.m_traderVector.shortSell[reqId].averageCost + a.m_traderVector.shortSell[reqId].stopLossShort)){
					a.m_traderVector.shortBuy[reqId].calculatedBid = realBid + (realAsk - realBid) * 0.5;
					a.m_traderVector.shortBuy[reqId].conditions[1] = true;
				}

				if (a.m_traderVector.shortSell[reqId].needToClose) a.m_traderVector.shortBuy[reqId].conditions[2] = true; //**** Due to earning release etc. 

				if (a.m_closeShortPositions){
					a.m_traderVector.shortBuy[reqId].calculatedBid = (realAsk + realBid) / 2;
					a.m_traderVector.shortBuy[reqId].conditions[3] = true;
				}

				a.m_traderVector.shortBuy[reqId].basicClose = a.m_traderVector.shortBuy[reqId].conditions[0] || a.m_traderVector.shortBuy[reqId].conditions[1] || a.m_traderVector.shortBuy[reqId].conditions[2]
					|| a.m_traderVector.shortBuy[reqId].conditions[3];

				a.m_traderVector.shortBuy[reqId].relevantShares = generalSharesShort(index, reqId, "sharesToBuy", a.m_traderVector.shortBuy[reqId].calculatedBid);

				if (a.m_traderVector.shortBuy[reqId].basicClose == true){
					bool readyToOpen = commonCode(reqId, index, currentDataStructure, algoString, "OPEN"); //**** This is recursive calling. Put it in the very end in order to save unnecessary calling
					if (readyToOpen)a.m_traderVector.shortBuy[reqId].basicClose = false; //**** Prevent flip-floping
				}

				a.m_traderVector.shortBuy[reqId].priceChanged = (abs(a.m_traderVector.shortBuy[reqId].myBid - a.m_traderVector.shortBuy[reqId].calculatedBid) >= a.m_traderVector.shortBuy[reqId].stopLossShort * a.m_traderParameters.priceUncertainty);
				if (a.m_traderVector.shortBuy[reqId].numModifiedBuy > a.m_traderParameters.maxModificationsAllowed)	a.m_traderVector.shortBuy[reqId].priceChanged = false;
			}
			return a.m_traderVector.shortBuy[reqId].basicClose;
		}
	}
}

unsigned int CommonFunctions::generalSharesLong(unsigned int &i, unsigned int &reqId, std::string String, double &currentPrice)
{
	int regularShares = 0, tempShares = 0, shares = 0, criticalSharesToBuy = 0; //**** return shares = -1 only for the similar meaning. Buy/Sell more than necessary. Don't return -1 for other usage. 
	if (currentPrice > 0)
	{
		double moneyPerOrder = a.m_traderParameters.minMoneyPerOrderPercentage *  (a.m_baseVector)[i][reqId].averageMoneyVolume;
		moneyPerOrder = (moneyPerOrder > a.m_traderParameters.standardMoneyPerOrder) ? a.m_traderParameters.standardMoneyPerOrder : moneyPerOrder;
		moneyPerOrder = (moneyPerOrder < a.m_traderParameters.minMoneyPerOrder) ? a.m_traderParameters.minMoneyPerOrder : moneyPerOrder;
		regularShares = moneyPerOrder / currentPrice;
		if (String == "sharesToBuy")
		{
			//**** If I want to buy more positions but don’t want to buy a lot with one order, I may use the same method as shown in the generalShares() in OTC. 
			criticalSharesToBuy = regularShares * a.m_traderParameters.criticalFactorToBuy;
			if (a.m_traderVector.longBuy[reqId].totalPosition >= 0) //***** If .totalPosition is < 0, then something must be wrong (e.g. an accident short position). In this negative position case, shares will be assinged a default 0 value. Thus there will be not more long orders to open.
				if (a.m_traderVector.longBuy[reqId].totalPosition < criticalSharesToBuy)
					shares = regularShares;
		}
		else if (String == "sharesToSell")
		{
			int maxNormalShares = regularShares *  (1 + a.m_traderParameters.sharesUncertainty);
			if (a.m_traderVector.longBuy[reqId].totalPosition < maxNormalShares)
			{
				if (a.m_traderVector.longBuy[reqId].totalPosition >= 0) shares = a.m_traderVector.longBuy[reqId].totalPosition; //***** Note if .totalPosition is < 0 (like accidently assigned a short position)
			}
			else
				shares = maxNormalShares;
		}
	}

	return shares;
}

unsigned int CommonFunctions::generalSharesShort(unsigned int &i, unsigned int &reqId, std::string String, double &currentPrice)
{
	int regularShares = 0, tempShares = 0, shares = 0, criticalSharesToSell = 0; //**** return shares = -1 only for the similar meaning. Buy/Sell more than necessary. Don't return -1 for other usage. 
	if (currentPrice > 0)
	{
		double moneyPerOrder = a.m_traderParameters.minMoneyPerOrderPercentage *  (a.m_baseVector)[i][reqId].averageMoneyVolume;
		moneyPerOrder = (moneyPerOrder > a.m_traderParameters.standardMoneyPerOrder) ? a.m_traderParameters.standardMoneyPerOrder : moneyPerOrder;
		moneyPerOrder = (moneyPerOrder < a.m_traderParameters.minMoneyPerOrder) ? a.m_traderParameters.minMoneyPerOrder : moneyPerOrder;
		regularShares = moneyPerOrder / currentPrice;
		criticalSharesToSell = regularShares * a.m_traderParameters.criticalFactorToBuy;
		if (String == "sharesToSell")
		{
			if (a.m_traderVector.shortSell[reqId].totalPosition <= 0)
				if (abs(a.m_traderVector.shortSell[reqId].totalPosition) < criticalSharesToSell)
					shares = regularShares;
		}
		else if (String == "sharesToBuy")
		{
			int maxNormalShares = regularShares *  (1 + a.m_traderParameters.sharesUncertainty);
			if (abs(a.m_traderVector.shortSell[reqId].totalPosition) < maxNormalShares)
			{
				if (a.m_traderVector.shortSell[reqId].totalPosition <= 0) shares = abs(a.m_traderVector.shortSell[reqId].totalPosition); //***** Note if .totalPosition is < 0 (like accidently assigned a short position)
			}
			else
				shares = maxNormalShares;
		}
	}
	return shares;
}

void CommonFunctions::calculateBaseVector(unsigned int &reqId)
{
	unsigned int i = a.m_traderParameters.i_periodIndex; //**** In the future do not do this copying things if performance really matters.
	for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); ++reqId) //Currently m_VectorL is fully occupied and thus I can use a.m_VectorL[i].size().
	{
		misceParameters(i, reqId); //**** This function should be called before lineFittingLocal because the later will use the min_index for each period. 
		lineFittingTested(i, reqId);
		lineFittingLocal(i, reqId, "MAX");
		lineFittingLocal(i, reqId, "MIN");

		ATR_calculation(i, reqId); //Checked once;
		volatilityCalculation(i, reqId); //Checked once;
		tradeLines(i, reqId);

		if (reqId % 20 == 0)
		{
			printf("The (i, reqId) for calculated BaseVector elements is (%d, %d). \n ", i, reqId);
		}
	}
}
void CommonFunctions::calculateBaseVector()
{
	
	for (unsigned int i = 0; i < a.m_baseNum.size(); i++)
	{
		for (unsigned int reqId = 0; reqId < a.m_newtxtVector.size(); reqId++) //Currently m_VectorL is fully occupied and thus I can use a.m_VectorL[i].size().
		{
			misceParameters(i, reqId); //**** This function should be called before lineFittingLocal because the later will use the min_index for each period. 
			lineFittingTested(i, reqId);
			lineFittingLocal(i, reqId, "MAX");
			lineFittingLocal(i, reqId, "MIN");

			ATR_calculation(i, reqId); //Checked once;
			volatilityCalculation(i, reqId); //Checked once;
			tradeLines(i, reqId);

			if (reqId % 20 == 0)
			{
				printf("The (i, reqId) for calculated BaseVector elements is (%d, %d). \n ", i, reqId);
			}
		}
	}
}

void CommonFunctions::calculateBaseVector_symbol()
{
	std::vector<unsigned int> i_vector;
	i_vector.push_back(a.m_traderParameters.i_periodIndex);
	//**** When doing push_back action, be careful the order. Should be the same as baseNum. 

	for (int i = 0; i < i_vector.size(); ++i)
	{
		for (auto it = a.m_newtxtMap.begin(); it != a.m_newtxtMap.end(); ++it)
		{
			misceParameters_Symbol(i_vector[i], it->second); //**** This function should be called before lineFittingLocal because the later will use the min_index for each period. 
			lineFittingTested(i_vector[i], it->second);
			lineFittingLocal(i_vector[i], it->second, "MAX");
			lineFittingLocal(i_vector[i], it->second, "MIN");

			ATR_calculation(i_vector[i], it->second); //Checked once;
			volatilityCalculation(i_vector[i], it->second); //Checked once;
			tradeLines(i_vector[i], it->second);
			tradingIndicators(i_vector[i], it->second); //**** Must be after misceParameters_Symbol

			if (it->second % 20 == 0)
			{
				printf("The (i, reqId) for calculated BaseVector elements is (%d, %d). \n ", i_vector[i], it->second);
			}
		}
	}
}

void CommonFunctions::tradingIndicators(unsigned int &i, unsigned int &reqId)
{
	if (i >= 0 && i < (int) a.m_baseNum.size() )
	{
		int size;
		if (a.m_backTest) size = a.m_baseVectorLastDayIndex + 1; else size = a.m_VectorL[reqId].size(); // From elsewhere: if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1; 
		double upVolatility = (a.m_baseVector)[i][reqId].upTradingLineIntercept - (a.m_baseVector)[i][reqId].intercept; //**** always >= 0.
		double downVolatility = (a.m_baseVector)[i][reqId].intercept - (a.m_baseVector)[i][reqId].downTradingLineIntercept; //**** always >= 0.
		double volatility = min(upVolatility, downVolatility); //**** The up and down volatility can be very different. 
		int x = size + a.m_baseNum[i];  //**** expected closing date. Need some time to realize gain. The closingDateFactor can be optmized using backtesting.
		double expectedMiddlePrice = (a.m_baseVector)[i][reqId].slope * (x - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
		x = size; //****today's index
		double expectedMiddlePriceToday = (a.m_baseVector)[i][reqId].slope * (x - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
		double absFutureTrendingAmount = abs(expectedMiddlePrice - expectedMiddlePriceToday);
		if (volatility > 0) (a.m_baseVector)[i][reqId].futureTrendingAmountVolatilityRatio = absFutureTrendingAmount / volatility;
		x = size - 1;
		if (a.m_VectorL[reqId][x].close > 0)
		{
			(a.m_baseVector)[i][reqId].volatilityPriceRatio = volatility / a.m_VectorL[reqId][x].close;
			(a.m_baseVector)[i][reqId].maxDrawDownPercentage = (upVolatility + downVolatility) / a.m_VectorL[reqId][x].close;
			double totalTrendingAmount = (a.m_baseVector)[i][reqId].slope * (a.m_baseNum[i] - 1); //**** Note the trendingUpAmount can be negative if slope is negative.
			(a.m_baseVector)[i][reqId].trendingPencentage = abs(totalTrendingAmount) / a.m_VectorL[reqId][x].close;

		}
		double middlePriceLastDay = (a.m_baseVector)[i][reqId].slope * (x - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
		if (middlePriceLastDay > 0)
		{
			if (a.m_VectorL[reqId][x].close > middlePriceLastDay)
			{
				(a.m_baseVector)[i][reqId].upDeviationPercentage = (a.m_VectorL[reqId][x].close - middlePriceLastDay) / middlePriceLastDay;
				(a.m_baseVector)[i][reqId].gapToDeviationRatio = (a.m_baseVector)[i][reqId].maxGapPercentage / (a.m_baseVector)[i][reqId].upDeviationPercentage;
			}
			if (middlePriceLastDay > a.m_VectorL[reqId][x].close)
			{
				(a.m_baseVector)[i][reqId].downDeviationPercentage = (middlePriceLastDay - a.m_VectorL[reqId][x].close) / middlePriceLastDay;
				(a.m_baseVector)[i][reqId].gapToDeviationRatio = (a.m_baseVector)[i][reqId].maxGapPercentage / (a.m_baseVector)[i][reqId].downDeviationPercentage;
			}
		}
	}
}

void CommonFunctions::lineFittingTested(unsigned int &i, unsigned int &reqId) // This one has been tested. It can be used as a standard to test the above function. 
{
	//**** For full-data case (no missing or zero data), I have compared the results with the EXCEL generated trend line. They are consistent. So at least in this case, the following code is tested. 
	//**** For missing data case, also tested. When printing out data, I need eliminate the zero or missing data. These data are not included in the calculations below. 
	int barIndex, numShortBarsConsidered;
	int originX = -1; //Note this is the index of origin for the line to be fitted.
	double originY = -1;
	m_lineStructure.originX = -1; //**** Used in other places.
	m_lineStructure.originY = -1;

	numShortBarsConsidered = a.m_baseNum[i];
	if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;
	int firstStartIndex = -1, firstEndIndex = -1, loopFirstEndIndex = -1;
	firstEndIndex = barIndex; //**** This is different from that of other functions.
	loopFirstEndIndex = firstEndIndex + 1;
	firstStartIndex = firstEndIndex - numShortBarsConsidered + 1;

	if (firstStartIndex >= 0) //****Sometimes the total elements of data array might be smaller than the numShortBarsConsidered, which is usually 120 trading days within half a year. 
	{
		if (a.m_VectorL[reqId][firstStartIndex].close > 0)
		{
			originX = firstStartIndex;
			originY = a.m_VectorL[reqId][firstStartIndex].close;
			m_lineStructure.originX = firstStartIndex; //**** Used in other places.
			m_lineStructure.originY = a.m_VectorL[reqId][firstStartIndex].close;
		}
		else
		{
			for (unsigned int j = firstStartIndex + 1; j < loopFirstEndIndex; j++)
			{
				if (a.m_VectorL[reqId][j].close > 0)
				{
					originX = j;
					originY = a.m_VectorL[reqId][j].close;
					m_lineStructure.originX = j; //**** Used in other places.
					m_lineStructure.originY = a.m_VectorL[reqId][j].close;
					break;
				}
			}
		}
	}

	if (originX > 0 && originY > 0)
	{
		m_lineStructure.numNonZeroData = 0;
		m_lineStructure.sumX = 0;
		m_lineStructure.averageX = 12345;
		m_lineStructure.sumY = 0;
		m_lineStructure.averageY = 12345;
		m_lineStructure.sumSquaresX = 0;
		m_lineStructure.sumSquaresXY = 0;
		m_lineStructure.slope = 12345;
		m_lineStructure.intercept = 12345;
		//m_lineStructure.firstCalculation = true; //default should be true;

		firstStartIndex = originX; //**** originX might be updated in the earlier non-zero data searching process. Also note originX varies as different i. 
		for (unsigned int index = firstStartIndex; index < loopFirstEndIndex; index++)
		{
			if (index >= 0 && index < a.m_VectorL[0].size())
			{
				if (a.m_VectorL[reqId][index].close > 0 && originY > 0) //**** No need to check originX because it is always >= 0, actually > 0 in most cases. 
				{
					//****The following arrangement take the first bar coordinates as origin. Thus the calculated intercept should be zero.
					m_lineStructure.sumX = m_lineStructure.sumX + index - originX;
					m_lineStructure.sumY = m_lineStructure.sumY + a.m_VectorL[reqId][index].close - originY;
					m_lineStructure.numNonZeroData = m_lineStructure.numNonZeroData + 1;
				}
			}
		}

		if (m_lineStructure.numNonZeroData > 0)
		{
			m_lineStructure.averageX = m_lineStructure.sumX / m_lineStructure.numNonZeroData;
			m_lineStructure.averageY = m_lineStructure.sumY / m_lineStructure.numNonZeroData; //**** averageY can be negative because sumY can be negative due to a big positive originY.
		}

		firstStartIndex = originX; //**** The originX might be updated in the earlier non-zero data searching process.
		for (unsigned int index = firstStartIndex; index < loopFirstEndIndex; index++)
		{
			if (index >= 0 && index < a.m_VectorL[0].size())
			{
				if (a.m_VectorL[reqId][index].close > 0 && originY > 0 && m_lineStructure.averageX != 12345 && m_lineStructure.averageY != 12345)
				{
					m_lineStructure.sumSquaresX = m_lineStructure.sumSquaresX + (index - originX - m_lineStructure.averageX) * (index - originX - m_lineStructure.averageX); //***** This sentence must be within the if branch. Otherwise we may take the x-value even y-value is zero.
					m_lineStructure.sumSquaresXY = m_lineStructure.sumSquaresXY + (index - originX - m_lineStructure.averageX) * (a.m_VectorL[reqId][index].close - originY - m_lineStructure.averageY);
				}
			}
		}

		if (m_lineStructure.numNonZeroData >= numShortBarsConsidered * a.m_traderParameters.minPercentageVectorOccupation) //**** Normally I calculate only when there are 60 percent occcupied, particularly for fast trading.
		{
			if (m_lineStructure.numNonZeroData > 0 && m_lineStructure.averageX != 12345 && m_lineStructure.averageY != 12345)
			{
				if (m_lineStructure.sumSquaresX > 0) m_lineStructure.slope = m_lineStructure.sumSquaresXY / m_lineStructure.sumSquaresX;
				if (m_lineStructure.slope != 12345)
					m_lineStructure.intercept = m_lineStructure.averageY - m_lineStructure.slope * m_lineStructure.averageX;
			}
			else
			{
				m_lineStructure.slope = 12345;
				m_lineStructure.intercept = 12345;
			}
		}
		else
		{
			m_lineStructure.slope = 12345;
			m_lineStructure.intercept = 12345;
		}

		(a.m_baseVector)[i][reqId].slope = m_lineStructure.slope;
		(a.m_baseVector)[i][reqId].intercept = m_lineStructure.intercept;
		(a.m_baseVector)[i][reqId].originX = m_lineStructure.originX;
		(a.m_baseVector)[i][reqId].originY = m_lineStructure.originY;

		//if (reqId == 1 && i == 2)
		//{
		//	if (remove("tempData2.txt") != 0) perror("Error deleting file: tempData1.txt");
		//	else puts("File successfully deleted");
		//	std::ofstream o_tempData1("tempData2.txt", std::ios::app);
		//	double upValue, downValue, middleValue;
		//	for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
		//	{
		//		if (a.m_VectorL[reqId][index].close > 0)
		//		{
		//			middleValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
		//			o_tempData1 << (index - firstStartIndex) << '\t' << middleValue << '\t' << a.m_VectorL[reqId][index].close << std::endl;
		//		}
		//	}

		//}

	}
}

void CommonFunctions::lineFittingLocal(unsigned int &i, unsigned int &reqId, std::string minOrMax) //**** Can make it good for day Trading? How to find the local minOrMax quickly
{
	unsigned int barIndex, numShortBarsConsidered;
	int originX = -1; //Note this is the index of origin for the line to be fitted.
	double originY = -1;
	m_lineStructure.originX = -1; //**** Used in other places.
	m_lineStructure.originY = -1;

	if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

	int firstStartIndex = -1, firstEndIndex = -1, loopFirstEndIndex = -1;
	if (minOrMax == "MAX")
		firstStartIndex = (a.m_baseVector)[i][reqId].maxIndex;
	else if (minOrMax == "MIN")
		firstStartIndex = (a.m_baseVector)[i][reqId].minIndex;

	if (firstStartIndex >= 0) //-1 must be the default value
	{
		firstEndIndex = barIndex; //**** This is different from that of other functions.
		loopFirstEndIndex = firstEndIndex + 1;
		numShortBarsConsidered = firstEndIndex - firstStartIndex + 1;


		if (firstStartIndex >= 0 && numShortBarsConsidered >= a.m_traderParameters.minNumBarsForLocalLine) //****Sometimes the total elements of data array might be smaller than the numShortBarsConsidered, which is usually 120 trading days within half a year. 
		{
			if (a.m_VectorL[reqId][firstStartIndex].close > 0)
			{
				originX = firstStartIndex;
				originY = a.m_VectorL[reqId][firstStartIndex].close;
				m_lineStructure.originX = firstStartIndex; //**** Used in other places.
				m_lineStructure.originY = a.m_VectorL[reqId][firstStartIndex].close;
			}
			else
			{
				for (unsigned int j = firstStartIndex + 1; j < loopFirstEndIndex; j++)
				{
					if (a.m_VectorL[reqId][j].close > 0)
					{
						originX = j;
						originY = a.m_VectorL[reqId][j].close;
						m_lineStructure.originX = j; //**** Used in other places.
						m_lineStructure.originY = a.m_VectorL[reqId][j].close;
						break;
					}
				}
			}
		}

		if (originX > 0 && originY > 0)
		{
			m_lineStructure.numNonZeroData = 0;
			m_lineStructure.sumX = 0;
			m_lineStructure.averageX = 12345;
			m_lineStructure.sumY = 0;
			m_lineStructure.averageY = 12345;
			m_lineStructure.sumSquaresX = 0;
			m_lineStructure.sumSquaresXY = 0;
			m_lineStructure.slope = 12345;
			m_lineStructure.intercept = 12345;
			//m_lineStructure.firstCalculation = true; //default should be true;

			firstStartIndex = originX; //**** The firstStartIndex might be updated in the earlier non-zero data searching process.
			for (unsigned int index = firstStartIndex; index < loopFirstEndIndex; index++)
			{
				if (index >= 0 && index < a.m_VectorL[0].size())
				{
					if (a.m_VectorL[reqId][index].close > 0 && originY > 0) //**** No need to check originX because it is always >= 0, actually > 0 in most cases. 
					{
						//****The following arrangement take the first bar coordinates as origin. Thus the calculated intercept should be zero.
						m_lineStructure.sumX = m_lineStructure.sumX + index - originX;
						m_lineStructure.sumY = m_lineStructure.sumY + a.m_VectorL[reqId][index].close - originY;
						m_lineStructure.numNonZeroData = m_lineStructure.numNonZeroData + 1;
					}
				}
			}

			if (m_lineStructure.numNonZeroData > 0)
			{
				m_lineStructure.averageX = m_lineStructure.sumX / m_lineStructure.numNonZeroData;
				m_lineStructure.averageY = m_lineStructure.sumY / m_lineStructure.numNonZeroData; //**** averageY can be negative because sumY can be negative due to a big positive originY.
			}

			firstStartIndex = originX; //**** The firstStartIndex might be updated in the earlier non-zero data searching process.
			for (unsigned int index = firstStartIndex; index < loopFirstEndIndex; index++)
			{
				if (index >= 0 && index < a.m_VectorL[0].size())
				{
					if (a.m_VectorL[reqId][index].close > 0 && originY > 0 && m_lineStructure.averageX != 12345 && m_lineStructure.averageY != 12345)
					{
						m_lineStructure.sumSquaresX = m_lineStructure.sumSquaresX + (index - originX - m_lineStructure.averageX) * (index - originX - m_lineStructure.averageX); //***** This sentence must be within the if branch. Otherwise we may take the x-value even y-value is zero.
						m_lineStructure.sumSquaresXY = m_lineStructure.sumSquaresXY + (index - originX - m_lineStructure.averageX) * (a.m_VectorL[reqId][index].close - originY - m_lineStructure.averageY);
					}
				}
			}

			if (m_lineStructure.numNonZeroData >= numShortBarsConsidered * a.m_traderParameters.minPercentageVectorOccupation) //**** Normally I calculate only when there are 60 percent occcupied, particularly for fast trading.
			{
				if (m_lineStructure.numNonZeroData > 0 && m_lineStructure.averageX != 12345 && m_lineStructure.averageY != 12345)
				{
					if (m_lineStructure.sumSquaresX > 0) m_lineStructure.slope = m_lineStructure.sumSquaresXY / m_lineStructure.sumSquaresX;
					if (m_lineStructure.slope != 12345)
						m_lineStructure.intercept = m_lineStructure.averageY - m_lineStructure.slope * m_lineStructure.averageX;
				}
				else
				{
					m_lineStructure.slope = 12345;
					m_lineStructure.intercept = 12345;
				}
			}
			else
			{
				m_lineStructure.slope = 12345;
				m_lineStructure.intercept = 12345;
			}

			if (minOrMax == "MAX")
			{
				(a.m_baseVector)[i][reqId].slopeLocalMax = m_lineStructure.slope;
				(a.m_baseVector)[i][reqId].interceptLocalMax = m_lineStructure.intercept;
				(a.m_baseVector)[i][reqId].originXLocalMax = m_lineStructure.originX;
				(a.m_baseVector)[i][reqId].originYLocalMax = m_lineStructure.originY;
			}
			else if (minOrMax == "MIN")
			{
				(a.m_baseVector)[i][reqId].slopeLocalMin = m_lineStructure.slope;
				(a.m_baseVector)[i][reqId].interceptLocalMin = m_lineStructure.intercept;
				(a.m_baseVector)[i][reqId].originXLocalMin = m_lineStructure.originX;
				(a.m_baseVector)[i][reqId].originYLocalMin = m_lineStructure.originY;
			}
		}
	}
}


void CommonFunctions::ATR_calculation(unsigned int &i, unsigned int &reqId)
{
	int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
	numShortBarsConsidered = a.m_baseNum[i];
	if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

	firstStartIndex = barIndex - numShortBarsConsidered + 1;
	loopFirstEndIndex = barIndex + 1;

	if (i >= 0 && i < a.m_baseNum.size() )
	{
		if (firstStartIndex >= 0)
		{
			int outBegIdx, outNBElement, retCode, endIdx, optInTimePeriod, outputSize;
			ATR_H.clear(); ATR_L.clear(); ATR_C.clear();

			for (unsigned int m = firstStartIndex; m < loopFirstEndIndex; m++)
			{
				if (m >= 0 && m < a.m_VectorL[reqId].size())
				{
					if (a.m_VectorL[reqId][m].high > 0 && a.m_VectorL[reqId][m].low > 0 && a.m_VectorL[reqId][m].close > 0)
					{
						ATR_H.push_back(a.m_VectorL[reqId][m].high);
						ATR_L.push_back(a.m_VectorL[reqId][m].low);
						ATR_C.push_back(a.m_VectorL[reqId][m].close);
					}
				}
			}

			if (ATR_H.size() >= (numShortBarsConsidered *a.m_traderParameters.percentageOfBarsToCalculateATR))
			{
				//**** Some notes on the understanding of calculating ATR. [1] If there are 10 raw data, the TA_Lib will obtain 9 true-range data for further averaging. However, for the normal moving average of 10 raw data has 10 data for further averaging. This is might be the reason that the outputSize is different for two cases. 
				endIdx = ATR_H.size() - 1; //***** Note here I use the real vector size for calculating endIdx but not the numBarsConsideredToCalculateATr. This is because there might be zero (no-data) bars.
				optInTimePeriod = ATR_H.size() - 1; //**** The number of TR to calculate average TR, i.e. ATR.
				outputSize = ATR_H.size() - optInTimePeriod; //**** For other TA indicators such as MA, outputSize = totalSize - optInTimePeriod + 1, due to the different lookback value (see the comments earlier).
				ATR_output = new double[outputSize]; //***** Only one ATR will be output from the above parameters. Note I must delete the allocated array later. Also note, With or without a () after the key word "new" does not make difference. 
				retCode = TA_ATR(0, endIdx, &ATR_H[0], &ATR_L[0], &ATR_C[0], optInTimePeriod, &outBegIdx, &outNBElement, ATR_output); //*** the type for parameter ATR_H is constant pointer. Array pointer is a constant pointer, and so is a vector. The pointer for output, however, is not a constant pointer, and hence cannot use &output[0] like form.
				//TA_RetCode TA_ATR(int startIdx, int endIdx, const double inHigh[], const double inLow[], const double inClose[], int optInTimePeriod, int *outBegIdx, int *outNBElement, double  outReal[]);
				int lastIndex = ATR_H.size() - optInTimePeriod - 1;
				(a.m_baseVector)[i][reqId].ATR = ATR_output[lastIndex]; //**** Here the optInTimePeriod is chosen to have only one ATR output. If not, then I need use the final element of ATR_output[0]. 
				delete[] ATR_output;
			}
		}
	}
}

void CommonFunctions::volatilityCalculation(unsigned int &i, unsigned int &reqId)
{
	int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
	numShortBarsConsidered = a.m_baseNum[i];
	if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

	firstStartIndex = barIndex - numShortBarsConsidered + 1;
	loopFirstEndIndex = barIndex + 1;
	std::vector<double> inputVector = *new(std::vector<double>);

	inputVector.clear();
	if (firstStartIndex >= 0)
	{
		for (int k = firstStartIndex; k < loopFirstEndIndex; k++)
		{
			if (a.m_VectorL[reqId][k].close > 0) inputVector.push_back(a.m_VectorL[reqId][k].close);
		}

		if (inputVector.size() >= (numShortBarsConsidered * a.m_traderParameters.percentageOfBarsToCalculateATR))
		{
			int startIndex = 0, endIndex = inputVector.size() - 1, optInTimePeriod = inputVector.size(), optInNbDev = 1, outBegIdx, outNBElement;
			double output = -1;
			TA_STDDEV(startIndex, endIndex, &inputVector[0], optInTimePeriod, optInNbDev, &outBegIdx, &outNBElement, &output); //**** For smaller period, stddev has big error. 
			(a.m_baseVector)[i][reqId].volatility = output; //**** Here the optInTimePeriod is chosen to have only one ATR output. If not, then I need use the final element of ATR_output[0]. 
			//if (a.m_VectorL[reqId][a.m_VectorL[reqId].size() - 1].close > 0) (a.m_baseVector)[i][reqId].volatilityPriceRatio = output / a.m_VectorL[reqId][a.m_VectorL[reqId].size() - 1].close;
		}
	}
}

void CommonFunctions::misceParameters(unsigned int &i, unsigned int &reqId)
{
	if (i >= 0 && i < a.m_baseNum.size() )
	{
		int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
		numShortBarsConsidered = a.m_baseNum[i];
		if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

		firstStartIndex = barIndex - numShortBarsConsidered + 1;
		loopFirstEndIndex = barIndex + 1;

		if (firstStartIndex >= 0)
		{
			double valueMax = 0.0, valueMin = 123456; //**** Should be same as the default values used in m_traderCommonVector.
			int indexForMax = -1, indexForMin = -1;
			double currentShareSum = 0, currentMoneySum = 0, validNumBars = 0; //**** validNumBars is intentionally a double variable.
			std::vector<double> SPY_Vector = *new(std::vector<double>), arbSymbol_Vector = *new(std::vector<double>);
			SPY_Vector.clear(); arbSymbol_Vector.clear();

			for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
			{
				if (a.m_VectorL[reqId][index].high > 0 && a.m_VectorL[reqId][index].high > valueMax) //**** returnValue should be assigned to a special value before calling. 
				{
					valueMax = a.m_VectorL[reqId][index].high;
					indexForMax = index;
				}

				if (a.m_VectorL[reqId][index].low > 0 && a.m_VectorL[reqId][index].low < valueMin) //**** returnValue should be assigned to a special value before calling. 
				{
					valueMin = a.m_VectorL[reqId][index].low;
					indexForMin = index;
				}

				if (a.m_VectorL[reqId][index].close > 0)
				{
					currentShareSum = currentShareSum + a.m_VectorL[reqId][index].volume;
					currentMoneySum = currentMoneySum + a.m_VectorL[reqId][index].volume * a.m_VectorL[reqId][index].close * 100;
					validNumBars = validNumBars + 1;
				}

				if (a.m_VectorL[0][index].symbol == "SPY" && a.m_VectorL[0][index].close > 0 && a.m_VectorL[reqId][index].close > 0)
				{
					SPY_Vector.push_back(a.m_VectorL[0][index].close);
					arbSymbol_Vector.push_back(a.m_VectorL[reqId][index].close);
				}
			} // end of loop
			(a.m_baseVector)[i][reqId].max = valueMax;
			(a.m_baseVector)[i][reqId].maxIndex = indexForMax;
			if (valueMin != 123456) (a.m_baseVector)[i][reqId].min = valueMin; else (a.m_baseVector)[i][reqId].min = 0;
			(a.m_baseVector)[i][reqId].minIndex = indexForMin;
			if (validNumBars > 0)
			{
				(a.m_baseVector)[i][reqId].averageShareVolume = currentShareSum / validNumBars; //**** NOTE this is in units of 100 shares. 
				(a.m_baseVector)[i][reqId].averageMoneyVolume = currentMoneySum / validNumBars;
			}
			if (numShortBarsConsidered > 0) (a.m_baseVector)[i][reqId].frequency = validNumBars / numShortBarsConsidered;

			if (SPY_Vector.size() >= (numShortBarsConsidered *a.m_traderParameters.percentageOfBarsToCalculateATR))
			{
				//**** No need to calculate pair-correlation among all the symbols for the following reasons: 
				//[1] I may need several different account for differne people. Each person can have one of the strongly correlated symbol. 
				//[2] Even there are two symbols in a single trading account. This is OK because buying twice will not be a disaster, like in OTC case. 
				//[3] If I found two strongly correlated symbols (for example with correlation coefficient one or minus one), then I can manually mark them take one of them out. 
				int outBeg, outNbElement;
				double output;
				TA_CORREL(0, SPY_Vector.size() - 1, &SPY_Vector[0], &arbSymbol_Vector[0], SPY_Vector.size(), &outBeg, &outNbElement, &output);
				(a.m_baseVector)[i][reqId].correlation = output;
			}
		} // if (firstStartIndex >= 0)
	}
}

void CommonFunctions::misceParameters_Symbol(unsigned int &i, unsigned int &reqId)
{
	if (i >= 0 && i < a.m_baseNum.size())
	{
		int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
		numShortBarsConsidered = (a.m_baseNum)[i];
		if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

		firstStartIndex = barIndex - numShortBarsConsidered + 1;
		loopFirstEndIndex = barIndex + 1;

		if (firstStartIndex >= 0)
		{
			double valueMax = 0.0, valueMin = 123456; //**** Should be same as the default values used in m_traderCommonVector.
			int indexForMax = -1, indexForMin = -1;
			double currentShareSum = 0, currentMoneySum = 0, validNumBars = 0, sumClosingPrice = 0; //**** validNumBars is intentionally a double variable.
			std::vector<double> index_Vector = *new(std::vector<double>), arbSymbol_Vector = *new(std::vector<double>);
			index_Vector.clear(); arbSymbol_Vector.clear();
			double maxGapPercentage = 0;
			for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
			{
				if (index < (loopFirstEndIndex - 1))
				{
					if (a.m_VectorL[reqId][index].close > 0 && a.m_VectorL[reqId][index+1].open > 0)
					{
						double gapPercentage = abs(a.m_VectorL[reqId][index + 1].open - a.m_VectorL[reqId][index].close) / a.m_VectorL[reqId][index].close;
						if (gapPercentage > maxGapPercentage) maxGapPercentage = gapPercentage;
					}
				}
				if (a.m_VectorL[reqId][index].high > 0 && a.m_VectorL[reqId][index].high > valueMax) //**** returnValue should be assigned to a special value before calling. 
				{
					valueMax = a.m_VectorL[reqId][index].high;
					indexForMax = index;
				}

				if (a.m_VectorL[reqId][index].low > 0 && a.m_VectorL[reqId][index].low < valueMin) //**** returnValue should be assigned to a special value before calling. 
				{
					valueMin = a.m_VectorL[reqId][index].low;
					indexForMin = index;
				}

				if (a.m_VectorL[reqId][index].close > 0)
				{
					currentShareSum = currentShareSum + a.m_VectorL[reqId][index].volume;
					currentMoneySum = currentMoneySum + a.m_VectorL[reqId][index].volume * a.m_VectorL[reqId][index].close * 100;
					sumClosingPrice = sumClosingPrice + a.m_VectorL[reqId][index].close;
					validNumBars = validNumBars + 1;
				}

				if (a.m_index_deque[index].close > 0 && a.m_VectorL[reqId][index].close > 0) //***** out of range here. 
				{
					index_Vector.push_back(a.m_index_deque[index].close);
					arbSymbol_Vector.push_back(a.m_VectorL[reqId][index].close);
				}
			} // end of loop
			(a.m_baseVector)[i][reqId].max = valueMax;
			(a.m_baseVector)[i][reqId].maxIndex = indexForMax;
			if (valueMin != 123456) (a.m_baseVector)[i][reqId].min = valueMin; else (a.m_baseVector)[i][reqId].min = 0;
			(a.m_baseVector)[i][reqId].minIndex = indexForMin;
			(a.m_baseVector)[i][reqId].maxGapPercentage = maxGapPercentage;
			if (validNumBars > 0)
			{
				(a.m_baseVector)[i][reqId].averageShareVolume = currentShareSum / validNumBars; //**** NOTE this is in units of 100 shares. 
				(a.m_baseVector)[i][reqId].averageMoneyVolume = currentMoneySum / validNumBars;
			}
			if (numShortBarsConsidered > 0) (a.m_baseVector)[i][reqId].frequency = validNumBars / numShortBarsConsidered;

			if (index_Vector.size() >= (numShortBarsConsidered *a.m_traderParameters.percentageOfBarsToCalculateATR))
			{
				int outBeg, outNbElement;
				double output;
				TA_CORREL(0, index_Vector.size() - 1, &index_Vector[0], &arbSymbol_Vector[0], index_Vector.size(), &outBeg, &outNbElement, &output);
				(a.m_baseVector)[i][reqId].correlation = abs(output); //**** For now I only consider the absolute values of correlation coefficients.
			}
		} // if (firstStartIndex >= 0)
	}
}

void CommonFunctions::tradeLines(unsigned int &i, unsigned int &reqId)
{
	if (i >= 0 && i < a.m_baseNum.size())
	{
		int barIndex, numShortBarsConsidered, firstStartIndex = -1, loopFirstEndIndex = -1;
		numShortBarsConsidered = a.m_baseNum[i];
		if (a.m_backTest) barIndex = a.m_baseVectorLastDayIndex; else barIndex = a.m_VectorL[reqId].size() - 1;

		firstStartIndex = barIndex - numShortBarsConsidered + 1;
		loopFirstEndIndex = barIndex + 1;

		if (firstStartIndex >= 0)
		{
			std::vector<double> interceptVectorHigh, interceptVectorLow;
			interceptVectorHigh.clear(), interceptVectorLow.clear();
			int oscillationCounter = 0, oscillationLeftCounter = 0, x0, x = 0;
			double linePrice = 0, c0 = 0, y0, y;
			double aboveArea = 0, belowArea = 0;
			for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
			{
				if (a.m_VectorL[reqId][index].high > 0 && a.m_VectorL[reqId][index].low > 0 && a.m_VectorL[reqId][index].close > 0)
				{
					linePrice = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
					if (linePrice > a.m_VectorL[reqId][index].low && linePrice < a.m_VectorL[reqId][index].high)
					{
						oscillationCounter = oscillationCounter + 1;
						if (index <= (index + numShortBarsConsidered / 2))  oscillationLeftCounter = oscillationLeftCounter + 1;
					}

					//**** When relative to origin (x0,y0), we subtract x0 and y0 respectively from all points(x,y). And we obtain slope and intercept relative to (x0,y0) as (y-y0) = b*(x-x0) + c. Note slope is same in any frame, but intercept is different.
					//**** The c above is the intercept when (x0, y0) is taken as origin. If we rewrite the equation as y = b*x + c + y0 - b*x0. This is just (y-0) = b*(x-0)+ c + y0-b*x0. Therefore, y0+c-b*x0 is the intercetp when we take (0,0) as origin. 
					//**** As in the slope and intercept calculation, here we only use the intercept relative to (x0,y0) origin. That is, from (y-y0) = b*(x-x0) + c, we have c = (y-y0)-b(x-x0). 
					x0 = (a.m_baseVector)[i][reqId].originX; x = index;
					y0 = (a.m_baseVector)[i][reqId].originY;
					y = a.m_VectorL[reqId][index].high; c0 = (y - y0) - (x - x0) * (a.m_baseVector)[i][reqId].slope; interceptVectorHigh.push_back(c0);
					y = a.m_VectorL[reqId][index].low;  c0 = (y - y0) - (x - x0) * (a.m_baseVector)[i][reqId].slope; interceptVectorLow.push_back(c0);

					if (a.m_VectorL[reqId][index].high > linePrice) aboveArea = aboveArea + a.m_VectorL[reqId][index].high - linePrice;
					if (a.m_VectorL[reqId][index].low < linePrice)  belowArea = belowArea + linePrice - a.m_VectorL[reqId][index].low;

				}
			} // end of loop
			(a.m_baseVector)[i][reqId].oscillationRate = (double)oscillationCounter / (double)numShortBarsConsidered;
			(a.m_baseVector)[i][reqId].leftToTotalOscillation = (double)oscillationLeftCounter / (double)numShortBarsConsidered;

			double valueMax = 0.000001, valueMin = 123456; //**** Should be same as the default values used in m_traderCommonVector.
			for (unsigned int index = 0; index < interceptVectorHigh.size(); index++)
			{
				if (interceptVectorHigh[index] > valueMax) valueMax = interceptVectorHigh[index]; //**** Note I cannot request interceptVectorHigh/Low to be > 0 because they can be either positive or negative. 
				if (interceptVectorLow[index] < valueMin) valueMin = interceptVectorLow[index];   //**** The intercepts stored in the vector must be valid numbers because they are calculated from valid .high, .low, numbers. 
			}
			if (valueMax != 0.000001 && valueMin != 123456)
			{
				(a.m_baseVector)[i][reqId].upBoundaryIntercept = valueMax;
				(a.m_baseVector)[i][reqId].downBoundaryIntercept = valueMin;
			}

			double upLoopingStep = 0, downLoopingStep = 0;
			upLoopingStep = ((a.m_baseVector)[i][reqId].upBoundaryIntercept - (a.m_baseVector)[i][reqId].intercept) / a.m_traderParameters.maxNumStepsLooping;
			if (upLoopingStep < 0.01) upLoopingStep = 0.01;
			downLoopingStep = ((a.m_baseVector)[i][reqId].intercept - (a.m_baseVector)[i][reqId].downBoundaryIntercept) / a.m_traderParameters.maxNumStepsLooping;
			if (downLoopingStep < 0.01)	downLoopingStep = 0.01;

			//**** up trading line calculation. Note the upper line intercept can be very different from the  down line intercept. 
			double intercept = 0;
			linePrice = 0;
			int J = 1;
			while (upLoopingStep >= 0.01)
			{
				int upTradingLineCounter = 0, leftUpTradingLineCounter = 0;
				intercept = (a.m_baseVector)[i][reqId].intercept + J * upLoopingStep; //**** Note this intercept is in the X-Y plane where origins are originX and originY. 
				double tempArea = 0;
				for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
				{
					linePrice = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + intercept + (a.m_baseVector)[i][reqId].originY; //**** This lineprice is not for regression line.
					if (a.m_VectorL[reqId][index].high > 0 && linePrice > 0 && a.m_VectorL[reqId][index].high >= linePrice)
					{
						tempArea = tempArea + a.m_VectorL[reqId][index].high - linePrice; //**** high > linePrice.
						upTradingLineCounter = upTradingLineCounter + 1; // Currently not used. 
						if (index <= (index + numShortBarsConsidered / 2))  leftUpTradingLineCounter = leftUpTradingLineCounter + 1;
					}
				} // end of loop

				double areaRatio = 0;
				if (aboveArea > 0) areaRatio = (aboveArea - tempArea) / aboveArea;
				if (areaRatio > a.m_traderParameters.tradingLineCriticalValue || intercept >= (a.m_baseVector)[i][reqId].upBoundaryIntercept)
				{
					(a.m_baseVector)[i][reqId].upTradingLineIntercept = intercept;
					if (numShortBarsConsidered > 0) (a.m_baseVector)[i][reqId].upperLeftToTotalCrossing = (double)leftUpTradingLineCounter / (double)numShortBarsConsidered;
					break;
				}
				J = J + 1;
			}

			//**** down trading line calculation. 
			J = 1;
			while (downLoopingStep >= 0.01)
			{
				int downTradingLineCounter = 0, leftDownTradingLineCounter = 0;
				intercept = (a.m_baseVector)[i][reqId].intercept - J * downLoopingStep;
				double tempArea = 0;
				for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
				{
					linePrice = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + intercept + (a.m_baseVector)[i][reqId].originY;
					if (a.m_VectorL[reqId][index].low > 0 && linePrice > 0 && a.m_VectorL[reqId][index].low <= linePrice)
					{
						tempArea = tempArea + linePrice - a.m_VectorL[reqId][index].low; 
						downTradingLineCounter = downTradingLineCounter + 1; // Currently not used. 
						if (index <= (index + numShortBarsConsidered / 2))  leftDownTradingLineCounter = leftDownTradingLineCounter + 1;
					}
				} // end of loop

				double areaRatio = 0;
				if (aboveArea > 0) areaRatio = (belowArea - tempArea) / belowArea;
				if (areaRatio > a.m_traderParameters.tradingLineCriticalValue || intercept <= (a.m_baseVector)[i][reqId].downBoundaryIntercept)
				{
					(a.m_baseVector)[i][reqId].downTradingLineIntercept = intercept;
					if (numShortBarsConsidered > 0)(a.m_baseVector)[i][reqId].downLeftToTotalCrosing = (double)leftDownTradingLineCounter / (double)numShortBarsConsidered;
					break;
				}
				J = J + 1;
			}

			//if (reqId == 6 && i == 2)
			//{
			//	if (remove("tempData1.txt") != 0) perror("Error deleting file: tempData1.txt");
			//	else puts("File successfully deleted");
			//	std::ofstream o_tempData1("tempData1.txt", std::ios::app);
			//	double upValue, downValue, middleValue; 
			//	for (int index = firstStartIndex; index < loopFirstEndIndex; index++)
			//	{
			//		if (a.m_VectorL[reqId][index].close > 0)
			//		{
			//			middleValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].intercept + (a.m_baseVector)[i][reqId].originY;
			//			upValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].upTradingLineIntercept + (a.m_baseVector)[i][reqId].originY;
			//			downValue = (a.m_baseVector)[i][reqId].slope * (index - (a.m_baseVector)[i][reqId].originX) + (a.m_baseVector)[i][reqId].downTradingLineIntercept + (a.m_baseVector)[i][reqId].originY;
			//			o_tempData1 << (index - firstStartIndex) << '\t' << a.m_VectorL[reqId][index].high << '\t' << middleValue << '\t' << a.m_VectorL[reqId][index].low << '\t' << upValue << '\t' << downValue << '\t' << a.m_VectorL[reqId][index].close << std::endl;
			//		}
			//	}
			//	std::cout << " oscillator counter : numData: oscillationRate " << oscillationCounter << ":" << numShortBarsConsidered << ":" << (a.m_baseVector)[i][reqId].oscillationRate << std::endl; 
			//}
		} // if (firstStartIndex >= 0)
	}
}
