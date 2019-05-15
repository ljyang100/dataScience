// For TA-lib
#include "ta_common.h"
#include "ta_defs.h"
#include "ta_func.h"
#include "ta_global.h"
#include "ta_memory.h"
#include <windows.h>
//End of TA-lib
#include "symbolSelection.h"

void  Symbols::indexVector()
{
	unsigned int daysCounter = 0;
	time_t dateInSeconds = a.m_backTest_symbolSection_EndTime; //***** The selected end date
	a.m_index_deque.clear();
	while (daysCounter <  a.m_traderParameters.numTradingDays)
	{
		struct tm *ts;
		ts = localtime(&dateInSeconds);
		std::string monthString, dayString, dateString, dataFileString;	
		if ((ts->tm_mon + 1) < 10) monthString = "0" + std::to_string(ts->tm_mon + 1); else monthString = std::to_string(ts->tm_mon + 1);
		if (ts->tm_mday < 10) dayString = "0" + std::to_string(ts->tm_mday); else dayString = std::to_string(ts->tm_mday);
		dateString = std::to_string(ts->tm_year + 1900) + monthString + dayString;
		dataFileString = a.m_dataFolderPrefix + dateString + ".txt";
		std::ifstream i_dataFile(dataFileString, std::ios_base::in);


		if (!i_dataFile.is_open())
			printf("Failed in opening the file %s.\n", dateString.c_str());
		else
		{
			printf("I am reading the data for the day of %s.\n", dataFileString.c_str());
			My_BarDataNormal myBarData;
			std::string dataString, String;
			std::vector<std::string> stringVector; stringVector.clear();
			while (getline(i_dataFile, dataString, '\n'))
			{
				std::stringstream   linestream(dataString);
				stringVector.clear();
				while (getline(linestream, String, ',')) stringVector.push_back(String);
				myBarData.symbol = stringVector[0];
				myBarData.date = stringVector[1];
				myBarData.open = std::stod(stringVector[2]);
				myBarData.high = std::stod(stringVector[3]);
				myBarData.low = std::stod(stringVector[4]);
				myBarData.close = std::stod(stringVector[5]);
				myBarData.volume = std::stoi(stringVector[6]);

				if (myBarData.symbol == "SPY") //**** Note this is for the symbol SPY.
				{
					a.m_index_deque.push_front(myBarData); //**** Make sure why I use deque here. 
					daysCounter = daysCounter + 1;
				}
			}
		} //if (i_dataFile.is_open())
		dateInSeconds = dateInSeconds - 86400;
	}
}

void  Symbols::historyVector()
{
	My_BarDataNormal barData;
	std::stringstream ss;
	ss << time(0);
	barData.symbol = "EMPTY";
	barData.date = "EMPTY";
	barData.open = 0;
	barData.high = 0;
	barData.low = 0;
	barData.close = 0;
	barData.volume = 0;

	a.m_VectorL.clear();//**** I need clear it because I will loop through the different exchanges. 
	std::vector<My_BarDataNormal> *tempVector = new(std::vector<My_BarDataNormal>);
	tempVector->clear();
	for (unsigned int k = 0; k < a.m_traderParameters.numTradingDays; k++) tempVector->push_back(barData);
	for (auto it = a.m_newtxtMap.begin(); it != a.m_newtxtMap.end(); ++it)	a.m_VectorL.push_back(*tempVector);

	unsigned int daysCounter = 0;
	time_t dateInSeconds = a.m_backTest_symbolSection_EndTime;
	while (daysCounter <  a.m_traderParameters.numTradingDays)
	{

		struct tm *ts;
		ts = localtime(&dateInSeconds);
		std::string monthString, dayString, dateString, dataFileString;	
		if ((ts->tm_mon + 1) < 10) monthString = "0" + std::to_string(ts->tm_mon + 1); else monthString = std::to_string(ts->tm_mon + 1);
		if (ts->tm_mday < 10) dayString = "0" + std::to_string(ts->tm_mday); else dayString = std::to_string(ts->tm_mday);
		dateString = std::to_string(ts->tm_year + 1900) + monthString + dayString;
		dataFileString = a.m_dataFolderPrefix + dateString + ".txt";
		std::ifstream i_dataFile(dataFileString, std::ios_base::in);

		if (!i_dataFile.is_open())	printf("Failed in opening the file %s.\n", dateString.c_str());
		else
		{
			daysCounter = daysCounter + 1;
			unsigned int index = a.m_traderParameters.numTradingDays - daysCounter; //**** Note for 120 days of data, the final index is 119.
			printf("I am reading the data for the day of %s.\n", dataFileString.c_str());
			My_BarDataNormal myBarData;
			std::string dataString, String;
			std::vector<std::string> stringVector; stringVector.clear();
			while (getline(i_dataFile, dataString, '\n'))
			{
				std::stringstream   linestream(dataString);
				stringVector.clear();
				while (getline(linestream, String, ',')) stringVector.push_back(String); //**** Note if I use string directly rather than indirectly using stringstream, then the empty space from CSV file will not be neglected. 

				//**** For more important application, I should not use hard-coding style as below but need other ways as elsewhere. 
				myBarData.symbol = stringVector[0];
				myBarData.date = stringVector[1];
				myBarData.open = std::stod(stringVector[2]);
				myBarData.high = std::stod(stringVector[3]);
				myBarData.low = std::stod(stringVector[4]);
				myBarData.close = std::stod(stringVector[5]);
				myBarData.volume = std::stoi(stringVector[6]);

				auto it = a.m_newtxtMap.find(myBarData.symbol);
				if (it != a.m_newtxtMap.end()) 	a.m_VectorL[it->second][index] = myBarData;

			}//while (getline(i_dataFile, dataString, '\n'))
		} //if (i_dataFile.is_open())
		dateInSeconds = dateInSeconds - 86400;
	}
}

void  Symbols::findSymbols(std::string exchangeName)
{
	std::vector<std::vector<int>> *originalReqIdVector = new(std::vector<std::vector<int>>); originalReqIdVector->clear();
	std::vector<int> tempVector; tempVector.clear();
	for (unsigned int k = 0; k < (m_inputStructureVector.size() + 1); k++) originalReqIdVector->push_back(tempVector); //**** 1 is added. 

	for (auto it = a.m_newtxtMap.begin(); it != a.m_newtxtMap.end(); ++it)	(*originalReqIdVector)[0].push_back(it->second); //**** Note that within the vector, the reqId is not ordered. 
	printf(" %d symbols are in the exchange %s. \n", (*originalReqIdVector)[0].size(), exchangeName.c_str());

	int i_index = a.m_traderParameters.i_periodIndex;
	for (unsigned int index = 0; index < m_inputStructureVector.size(); index++)
	{
		//*****IMPORTANT: Add most priority filters first. For example, if filter oscillation first and get samples with very big oscillation but very small amplitude. Then later you will get only tiny sample with big amplitude filter.
		if ( m_inputStructureVector[index].string0 == "frequency")
		{
			std::string string0 =  m_inputStructureVector[index].string0, string1 =  m_inputStructureVector[index].string1;
			std::istringstream instream( m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].frequency >= value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].frequency < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if ( m_inputStructureVector[index].string0 == "correlation")
		{
			std::string string0 =  m_inputStructureVector[index].string0, string1 =  m_inputStructureVector[index].string1;
			std::istringstream instream( m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if (abs((a.m_baseVector)[i_index][reqId].correlation) >= value) (*originalReqIdVector)[index + 1].push_back(reqId); //****Note that absolute sign is added to create non-negative correlations.
				}
				else if (string1 == "SMALLERTHAN")
				{
					if (abs((a.m_baseVector)[i_index][reqId].correlation) < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);

		}
		else if ( m_inputStructureVector[index].string0 == "averageMoneyVolume")
		{
			std::string string0 =  m_inputStructureVector[index].string0, string1 =  m_inputStructureVector[index].string1;
			std::istringstream instream( m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].averageMoneyVolume >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].averageMoneyVolume < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if ( m_inputStructureVector[index].string0 == "closedPrice")
		{
			std::string string0 =  m_inputStructureVector[index].string0, string1 =  m_inputStructureVector[index].string1;
			std::istringstream instream( m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j];
				if (string1 == "BIGGERTHAN")
				{
				
					int size = a.m_VectorL[reqId].size();
					if (a.m_VectorL[reqId][size - 1].close >= value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					int size = a.m_VectorL[reqId].size();
					if (a.m_VectorL[reqId][size - 1].close < value) (*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if ( m_inputStructureVector[index].string0 == "oscillationRate") //****OscillationRate must be combined with other volatitlity/amplitude indicator. Otherwise, there migth be big oscillations with very small amplitude.
		{
			std::string string0 =  m_inputStructureVector[index].string0, string1 =  m_inputStructureVector[index].string1;
			std::istringstream instream( m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].oscillationRate >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].oscillationRate < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if (m_inputStructureVector[index].string0 == "VOLATILITYPRICERATIO") //**** Might be good for day trading even the percentage is not big as it
		{
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].volatilityPriceRatio >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].volatilityPriceRatio < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if (m_inputStructureVector[index].string0 == "TRENDINGVOLATILITYRATIO") //**** Might be good for day trading even the percentage is not big as it
		{
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].futureTrendingAmountVolatilityRatio >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].futureTrendingAmountVolatilityRatio < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if (m_inputStructureVector[index].string0 == "RECENTNUMVOLDIFFERENCE") //**** Might be good for day trading even the percentage is not big as it
		{
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j];
				double numVolatilityRecentDifference = 0;
				double difference = (a.m_baseVector)[0][reqId].max - (a.m_baseVector)[0][reqId].min;//**** Most recent range drawdown. Note I need calculate the 0th element if I want this.
				double volatility = (a.m_baseVector)[i_index][reqId].upTradingLineIntercept - (a.m_baseVector)[i_index][reqId].intercept; //**** always >= 0.
				if (volatility > 0) numVolatilityRecentDifference = difference / volatility;
				if (string1 == "BIGGERTHAN")
				{
					if (numVolatilityRecentDifference >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if (numVolatilityRecentDifference < value) 	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if (m_inputStructureVector[index].string0 == "deviationPercentage") //****OscillationRate must be combined with other volatitlity/amplitude indicator. Otherwise, there migth be big oscillations with very small amplitude.
		{
			//for testing
			std::vector<std::string> m_helloUp, m_helloDown;

			std::string tempString;
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			instream >> tempString;
			std::string stringUp, stringDown;
			stringUp = tempString.substr(0, 4);//**** first parameter start position. Second parameter is the length of string.
			stringDown = tempString.substr(5, 4);
			double valueUp, valueDown;
			valueUp = std::stod(stringUp);
			valueDown = std::stod(stringDown);
			std::vector<int> tempVectorUp, tempVectorDown; tempVectorUp.clear(); tempVectorDown.clear();
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; 
				if ((a.m_baseVector)[i_index][reqId].upDeviationPercentage >= valueUp) tempVectorUp.push_back(reqId);
				if ((a.m_baseVector)[i_index][reqId].downDeviationPercentage >= valueDown) tempVectorDown.push_back(reqId);
			}
			printf(" %d symbols are selected due to upDeviationPercentage is above %f. \n", tempVectorUp.size(), valueUp);
			printf(" %d symbols are selected due to downupDeviationPercentage is above %f. \n", tempVectorDown.size(), valueDown);


			if (tempVectorUp.size() > 0)
			{
				for (unsigned int reqId = 0; reqId < tempVectorUp.size(); ++reqId)
				{
					(*originalReqIdVector)[index + 1].push_back(tempVectorUp[reqId]); //**** Be very careful that if I use push_back(reqId), then it will be completely wrong.
				}
			}
			if (tempVectorDown.size() > 0)
			{
				for (unsigned int reqId = 0; reqId < tempVectorDown.size(); ++reqId)
				{
					(*originalReqIdVector)[index + 1].push_back(tempVectorDown[reqId]);
				}
			}
			printf(" %d symbols are selected due to DeviationPercentage selection. \n", (*originalReqIdVector)[index + 1].size());
		}
		else if (m_inputStructureVector[index].string0 == "gapToDeviationRatio") //**** Might be good for day trading even the percentage is not big as it
		{
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].gapToDeviationRatio >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].gapToDeviationRatio < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}
		else if (m_inputStructureVector[index].string0 == "maxDrawDownPercentage") //**** Might be good for day trading even the percentage is not big as it
		{
			std::string string0 = m_inputStructureVector[index].string0, string1 = m_inputStructureVector[index].string1;
			std::istringstream instream(m_inputStructureVector[index].string2);
			double value;
			instream >> value;
			for (unsigned int j = 0; j < (*originalReqIdVector)[index].size(); j++)
			{
				int reqId = (*originalReqIdVector)[index][j]; //*******
				if (string1 == "BIGGERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].maxDrawDownPercentage >= value)  (*originalReqIdVector)[index + 1].push_back(reqId);
				}
				else if (string1 == "SMALLERTHAN")
				{
					if ((a.m_baseVector)[i_index][reqId].maxDrawDownPercentage < value)	(*originalReqIdVector)[index + 1].push_back(reqId);
				}
			}
			printf(" %d symbols are selected due to %s %s %f. \n", (*originalReqIdVector)[index + 1].size(), string0.c_str(), string1.c_str(), value);
		}


	} // for (int index = 1; index < a.m_inputStructureVector.size(); index++) 

	std::string outputFileString = "C:/symbols/symbols" + exchangeName + a.m_accountCode + ".txt";
	std::ofstream o_new(outputFileString.c_str());
	int size = originalReqIdVector->size();
	if (size >= 1)
	{
		if ((*originalReqIdVector)[size - 1].size() > 0)
		{
			for (unsigned int i = 0; i < (*originalReqIdVector)[size - 1].size(); i++)
			{
				for (auto it = a.m_newtxtMap.begin(); it != a.m_newtxtMap.end(); ++it)
					if (it->second == (*originalReqIdVector)[size - 1][i]) o_new << it->first << std::endl;
			}
			o_new.close();
			printf("There are %d new symbols have been saved to the file %s.\n", (*originalReqIdVector)[size - 1].size(), outputFileString.c_str());
		}
	}
}

void  Symbols::combineSymbols()
{
	//**** Part I combine all newly-obtained symbols for today. 
	std::vector<std::string> oldTradingSymbolVector, newTradingSymbolVector;
	oldTradingSymbolVector.clear(); newTradingSymbolVector.clear();
	std::string oldTradingSymbols = "C:/symbols/symbolsOLD_Prev"; oldTradingSymbols.append(a.m_accountCode); oldTradingSymbols.append(".txt");
	std::string newTradingSymbols = "C:/symbols/symbolsNEW_Prev"; newTradingSymbols.append(a.m_accountCode); newTradingSymbols.append(".txt");
	remove(oldTradingSymbols.c_str());
	rename(newTradingSymbols.c_str(), oldTradingSymbols.c_str()); //**** rename new.txt to old.txt, but not the opposite.
	std::string symbol;
	for (unsigned int i = 0; i < m_exchangeNameVector.size(); i++)
	{
		std::string symbolFile = "C:/symbols/symbols" + m_exchangeNameVector[i] + a.m_accountCode + ".txt";
		std::ifstream i_inputFile1(symbolFile, std::ios_base::in);
		std::istringstream instream;
		std::string wholeLineString;
		while (getline(i_inputFile1, wholeLineString, '\n'))
		{
			instream.clear();
			instream.str(wholeLineString);
			instream >> symbol;
			newTradingSymbolVector.push_back(symbol);
		}
		i_inputFile1.close();
	}

	//**** Part II Eliminate symbtols with "No Security" warning in IB. 
	std::string noSecuritySymbols = "C:/symbols/noSecuritySymbols"; noSecuritySymbols.append(a.m_accountCode); noSecuritySymbols.append(".txt");
	std::ifstream i_noSecurityFile(noSecuritySymbols, std::ios_base::in); if (!i_noSecurityFile.is_open()) printf("Failed in opening the file noSecuritySymbols.txt.\n");
	std::string wholeLineString;
	std::stringstream linestream; linestream.clear();
	while (getline(i_noSecurityFile, wholeLineString, '\n'))
	{
		linestream.clear();
		linestream.str(wholeLineString);
		linestream >> symbol;

		for (auto it = newTradingSymbolVector.begin(); it != newTradingSymbolVector.end(); ++it){
			if ((*it) == symbol){
				it = newTradingSymbolVector.erase(it); //**** For vectors, if I need insert or erase, better use iterator but not subscript? 
			}
		}
	}
	i_noSecurityFile.close();


	//**** Part III Print out symbols. 
	std::sort(newTradingSymbolVector.begin(), newTradingSymbolVector.end()); //**** Must run std::sort() before finding the unique value. Otherwise it won't work. unique only elimimate consecutitive same elements.
	auto last = std::unique(newTradingSymbolVector.begin(), newTradingSymbolVector.end()); // This replace:	std::vector<std::string>::iterator newEnd;
	int originalSize = newTradingSymbolVector.size();
	newTradingSymbolVector.erase(last, newTradingSymbolVector.end());
	int finalSize = newTradingSymbolVector.size();

	std::ofstream o_new(newTradingSymbols.c_str());
	for (unsigned int i = 0; i < newTradingSymbolVector.size(); i++) 
		o_new << newTradingSymbolVector[i] << std::endl;

	o_new.close();
	printf("There are %d new symbols have been saved to the file %s. %d duplicated symbols are eliminated. \n", newTradingSymbolVector.size(), newTradingSymbols.c_str(), originalSize - finalSize);


	//**** Part IV  Compare how many symbols are different from the old ones.
	std::string m_oldSymbol = "C:/symbols/symbolsNEW"; m_oldSymbol.append(a.m_accountCode);   m_oldSymbol.append(".txt"); //**** The symbolsNEW file contains the "old" symbols.
	std::ifstream i_inputFile(m_oldSymbol, std::ios_base::in); if (!i_inputFile.is_open()) printf("Failed in opening the file new.txt.\n");
	std::vector<std::string> oldSymbolsVector; oldSymbolsVector.clear();
	while (getline(i_inputFile, wholeLineString, '\n'))
	{
		linestream.clear();
		linestream.str(wholeLineString);
		linestream >> symbol;
		oldSymbolsVector.push_back(symbol);
	}//while (getline(i_inputFile, wholeLineString, '\n'))
	i_inputFile.close();

	std::sort(newTradingSymbolVector.begin(), newTradingSymbolVector.end());
	std::sort(oldSymbolsVector.begin(), oldSymbolsVector.end());
	std::vector<std::string> resultVector;  resultVector.clear();
	std::set_difference(newTradingSymbolVector.begin(), newTradingSymbolVector.end(), oldSymbolsVector.begin(), oldSymbolsVector.end(), std::inserter(resultVector, resultVector.begin()));

	std::cout << "The total number of different symbols from old trading symbols are: " << std::endl;
	for (auto string : resultVector) std::cout << string << std::endl;
	std::this_thread::sleep_for(std::chrono::hours(100));
}

