
#ifndef sharestructure_h_
#define sharestructure_h_

#include "TwsApiL0.h"
#include <time.h>
#include "CriticalSection.h"
#include <deque>

struct My_BarDataNormal {
	std::string date;
	std::string yearMonthDateString;
	double open;
	double high;
	double low;
	double close;
	unsigned int volume;
	//double average; //do not uncomment it if no using it. 
	double currentMoneySum;
	unsigned int	 currentShareSum;
	std::string lastDownloadTime; 
	std::string symbol;

};

struct LineStructure{
	unsigned int barIndexDone;
	unsigned int numNonZeroData;
	double sumX;
	double averageX;
	double sumY;
	double averageY;
	double sumSquaresX;
	double sumSquaresXY;
	double slope;
	double intercept;
	bool firstCalculation; //default should be true;
	double originX;
	double originY;
};

struct CurrentDataStructure{   
	std::string symbol; //for testing 
	unsigned int reqId;
	double ask;
	unsigned int askSize;
	double bid;
	unsigned int bidSize;
	double last;
	unsigned int lastSize;
	double open;
	double high;
	double low;
	double close;
	time_t timeStamp;
};

//The structure below handle only the ticks from tickPrice and tickSize. If in the future I need ticks from tickGeneric and tickString, which can be obtained 
//from modifying the genericTicklist in reqMktData, then I need extend this structure.
struct DequeStructure{ 
 time_t timestamp;
 TickerId tickerId;
 TickType field;
 double price;
 int canAutoExecute;
 unsigned int size;
};

struct BaseStructure{
	unsigned int			i; //**** range index
	unsigned int			numDaysConsidered;
	//std::string symbol;
	double		frequency;
	double		finalLow; 
	double		finalHigh; 
	double		min;
	unsigned int			minIndex;
	double		max;
	unsigned int			maxIndex;
	double		ATR; 
	double		volatility; 
	double		averageMoneyVolume;
	unsigned int			averageShareVolume;
	double		correlation;
	double		slope;
	double		intercept;
	double		originX;
	double		originY;
	double		upperOscillaitonTimesVolatility;
	double		downOscillaitonTimesVolatility;
	double		leftToTotalOscillation;
	double		upperLeftToTotalCrossing;
	double		downLeftToTotalCrosing;
	double		upTradingLineIntercept;
	double		downTradingLineIntercept;
	double		upBoundaryIntercept;
	double		downBoundaryIntercept;
	double		slopeLocalMin;
	double		interceptLocalMin;
	double		originXLocalMin;
	double		originYLocalMin;
	double		slopeLocalMax;
	double		interceptLocalMax;
	double		originXLocalMax;
	double		originYLocalMax;
	double		oscillationRate;
	double		upDeviationPercentage;
	double		downDeviationPercentage;
	double		trendingPencentage;
	double      maxDrawDownPercentage;
	double		volatilityPriceRatio;
	double		futureTrendingAmountVolatilityRatio;
	double		maxGapPercentage;
	double		gapToDeviationRatio;


};

struct TraderStructure{

	//***** Unless I have to, don't use elements that I cannot restore from IB only. So for traderStructureVector, actually I don't need save it to database at all. 
	//***** Use database only when I cannot restore very fast, e.g the historical data. Or when I need a backup. 
	double		myBid;
	int			totalPosition; //**** Position can be negative for short
	unsigned int			buyingShares;
	unsigned int			currentTotalBuy;
	unsigned int			currentTotalSell;
	double		averageCost; //****This is set in orderStatus() and thus should be the average cost of particular jth cost. The general cost for the total position might be in the updatePortfolio. 
	bool		closing;  
	bool		opening;
	bool		cancelling;
	OrderId		orderId;
	std::string symbol; 
	std::string orderType; 
	time_t		orderUpdatedTime;
	time_t		lastModifiedTimeBuy;
	time_t		lastModifiedTimeSell;
	unsigned int			sellingShares;
	std::string action;
	double		myAsk;
	unsigned int			totalCancellationsBuy;
	unsigned int			totalCancellationsSell;
	unsigned int			totalDuplicateOrderBuy;
	unsigned int			totalDuplicateOrderSell;
	bool		modifiedBuy;
	bool		modifiedSell;
	unsigned int			filledBuy;
	unsigned int			filledSell;
	unsigned int			numModifiedBuy;
	unsigned int			numModifiedSell;
	bool		sessionClosing; 
	double		totalLongValue;
	double		totalShortValue;
	time_t  positionUpdatedTime;
	time_t	 lastFilledLongTime;
	time_t	 lastFilledShortTime;
	double filledPrice;
	bool   needToClose;
	double minPriceVariation;
	bool manualOrder;

	double calculatedBid;
	double calculatedAsk;
	unsigned int   relevantShares;
	bool	longHedgeOK;
	bool	shortHedgeOK;
	double stopLossLong;
	double stopLossShort;
	bool   basicOpen;
	bool   basicClose;
	std::string algoString;
	std::vector<bool> conditions;
	bool priceChanged;
	bool noBuyLong;
	bool noSellLong;
	bool noBuyShort;
	bool noSellShort;
	bool stoppedOut;
	

};

struct AlgoLiteral{
	std::string LONG;
	std::string SHORT;
};


struct TraderVector{
	std::vector<TraderStructure> longBuy;
	std::vector<TraderStructure> longSell;
	std::vector<TraderStructure> shortBuy;
	std::vector<TraderStructure> shortSell;
};

const struct TraderParameters{ 
	//Part I trader parameters
	const double criticalFactorToBuy = 0.5;
	const double bigSpreadBias = 0.5;
	const double smallSpreadBias = 0.25;
	const time_t maxTimeToWaitSendingMessage = 20; //20 means 20*100=2000ms=2s; 
	const double percentageOfBarsToCalculateATR = 0.7; //***** Note this is not minPercentageVectorOccupation.
	const double minPercentageVectorOccupation = 0.7;
	const time_t timeBetweenReqOpenOrders = 20; //****Note this is in the unit of second.
	const time_t historicalDataWaitingTime = 3; // Set this to be 5 minutes. Otherwise whenever internet disruppted a little time, it will set all .ready = true. Normally we don't need wait. Also when internet is disrrupted for more than 5 minutes, app will not sent historical requests and will not set to ready. 
	const time_t orderPositionWaitingTime = 60; //**** Check whether this is enough.
	const unsigned int maxModificationsAllowed = 100; //**** Should be multiple itmes the max number of modification in one round
	const unsigned int cancellationsAllowed = 10;
	const unsigned int minSmallBarsConsidered = 5; //  8; //****    // in units of one minute, or 5 minutes, etc. 40 bars is the whole range, where we use 20 bars to calculate deviation.
	const unsigned int maxSmallBarsConsidered = 10; // 40; //  in units of one or five minutes.
	const unsigned int stepSmallBars = 5;
	const unsigned int optInNbDevUp = 2;
	const unsigned int optInNbDevDn = 2;
	const bool	dayTrading = false; //**** Set this to be false for overnight trading. Otherwise the app will not create order much earlier before session close. 
	const time_t timeShiftToOpenOrder = 6 * 60 * 60; //**** can open order half-hour before closing at the lastest. 
	const time_t timeShiftToCloseOrder = 6.5 * 60 * 60 - 5 * 60; //**** must close order five minutes before closing. 
	const double sharesUncertainty = 0.2;
	const unsigned int badTickFilterParameter = 2;
	const double exposureMean = 0; //**** default is zero. If I need protect OTC long position, I may let it to be -1000
	const double exposureVol = 500; //**** default is 500 or more. If I want to no hedge at all, set this number very high
	const double acceptableCommissionRatio = 0.1; //**** !!!!! meaning commission to gain ratio.
	const unsigned int minNumBarsToCalculateSlope = 5; //**** Normally this means 5 1-min bars.
	const double standardMoneyPerOrder = 1000; //***** Depend on the total money available and on the average daily volum of each stock.
	const double minMoneyPerOrder = 500;
	const double minMoneyPerOrderPercentage = 0.05;
	const double reversingRequiredFactor = 0.16; //**** Note this number should be given according to volatilityFactorLong/Short
	const double reversingRequiredFactorMax = 0.25;
	const double reversingRequiredFactorEX = 0.05; //**** Note this number should be given according to volatilityFactorLong/Short
	const double reversingRequiredFactorMaxEX = 0.333;
	const double priceUncertainty = 0.1;
	const unsigned int maxNumDaysConsidered = 120;
	const unsigned int numNoTradingDays = 60;
	const time_t timeToContinue = 120;
	const time_t timeForOneRoundModifications = 60; // 60 seconds.
	const time_t timeDurationBetweenRoundOfMofications = 30 * 60;
	const unsigned int maxNumStepsLooping = 500;
	const double remainingProfitFactor = 0.5;
	const double stopLossFactor = 1.0; //Better keep it as 0.5.
	const double tradingLineCriticalValue = 0.9;	//***** Change SIMULTANEOUSLY in two projects.
	const unsigned int i_periodIndex = 2;
	const unsigned int minNumBarsForLocalLine = 2;
	const double lowTrendingLimit = 0.6; //**** numVol trended during the whole period considered.  
	const double highTrendingLimit = 2.0;
	const bool	fullDataLinesSubscribed = false;
	const unsigned int numDataRequested = 100; //***** Note this should be above the datalines I have. The default is 100. However, if my commission is a lot, then this might be bigger than 100.
	const double closingDateFactor = 0.5; //****If period is one month, then we may close a position within 5 days. 
	const unsigned int maxNumConditions = 10;
	const double remainingGainCriticalFactor = 0.33; // Normally even for downtrending, I still expected a gain of volatility. However, if this expectation is below 1/3 of volatility, I should close the position.
	const time_t maxWaitingTimeForMarketData = 3;
	const unsigned int earliestTradingDayFactor = 2; 
	const double bigProfitFactor = 3.0;
	const unsigned int numTradingDays = 120;
	
};

struct InputStructure{
	std::string string0;
	std::string string1;
	std::string string2;
};

struct OPEN_CLOSE{
	std::vector<std::string> open;
	std::vector<std::string> close;
};

struct MR_TF{
	OPEN_CLOSE MR;
	OPEN_CLOSE TF;
};

struct AlgoStringLiteral{
	MR_TF LONG;
	MR_TF SHORT;
};

struct FinishedTradeStructure{
	unsigned int reqId;
	std::string symbol;
	std::string longOrShort;
	unsigned int OpenedDateIndex;
	unsigned int OpenedSmallBarIndex;
	unsigned int closedDateIndex;
	unsigned int closedSmallBarIndex;
	bool opened;
	bool closed;
	std::string algoString;
	std::vector<bool> conditions;
	unsigned int sharesOpened;
	double gain_loss;
	double commissionOpened;
	double commissionClosed;
	double averageCost;
};

struct TradeStructureTest{
	bool longOpened;
	bool longClosed;
	bool shortOpened;
	bool shortClosed;
	unsigned int OpenedDateIndex;
	unsigned int smallBarIndex;
};

#endif 