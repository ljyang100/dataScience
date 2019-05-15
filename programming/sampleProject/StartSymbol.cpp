#include "startSymbol.h"


void StartSymbol::StartEntryPoint()
{
	AllData a;			//****huge amount of data stored in heap. 
	a.m_mostRecentDate = true; //This sentence must be before a.initializeTraderParameters(); 
	a.initializeTraderParameters();     //****Be careful of the ording of the member functions of allData instance.
	a.m_backTest = false; 
	CommonFunctions cfs(a);
	exchangeNameVector.push_back("NASDAQ"); exchangeNameVector.push_back("NYSE"); exchangeNameVector.push_back("AMEX");

	Symbols symbols(a, inputStructureVector, exchangeNameVector);
	//Handle market index Vector
	std::string string = "AMEX"; // It should be only AMEX
	a.m_dataFolderPrefix = "C:/Data/" + string + "/" + string + "_";
	symbols.indexVector(); //Need only once
	for (unsigned int i = 0; i < exchangeNameVector.size(); i++){
		int counter = 0;
		std::string BREAK;
		std::string inputFileString = "C:/symbols/inputSelectionRule"; inputFileString.append(a.m_accountCode); inputFileString.append(".txt");
		while (1){
			std::ifstream i_inputFile(inputFileString, std::ios_base::in);
			inputStructureVector.clear();
			if (!i_inputFile.is_open())
				printf("Failed in opening the input file %s.\n", inputFileString.c_str());
			else{
				InputStructure inputStructure;
				std::string wholeLineString, subString;
				std::vector<std::string> stringVector;
				while (getline(i_inputFile, wholeLineString, '\n'))	{
					std::stringstream   linestream(wholeLineString);
					stringVector.clear();
					while (getline(linestream, subString, ','))	{
						stringVector.push_back(subString);
					}
					inputStructure.string0 = stringVector[0]; //**** Note if I use string directly rather than indirectly using stringstream, then the empty space from CSV file will not be neglected. 
					inputStructure.string1 = stringVector[1];
					inputStructure.string2 = stringVector[2];

					inputStructureVector.push_back(inputStructure);
				}//while (getline(i_inputFile, wholeLineString, '\n'))
				i_inputFile.close();
			}

			if (counter <= 0){
				//Calculate baseVector for specific exchange. 
				std::string string = exchangeNameVector[i];
				std::string symbolListString = "C:/Data/" + string + "/" + string + ".txt";
				a.m_dataFolderPrefix = "C:/Data/" + string + "/" + string + "_";
				std::ifstream i_symbolList(symbolListString, std::ios_base::in); if (!i_symbolList.is_open()) printf("Failed in opening the symbolList file.\n");
				std::string symbolString;
				unsigned int symbolReqId = 0;
				a.m_newtxtMap.clear();
				while (getline(i_symbolList, symbolString, '\n')){

					if (symbolString != "Symbol"){
						a.m_newtxtMap.insert({ symbolString, symbolReqId }); //**** Note the "Symbol" in the first line of a file is not the real symbol name.
						symbolReqId = symbolReqId + 1;
					}
				}
				i_symbolList.close();

				symbols.historyVector();
				a.initializeBaseVector("SYMBOL");
				cfs.calculateBaseVector_symbol();
			}

			//setup_L.findSymbolsOTC(); //**** When turned on OTC case, turn off the lines below for normal stocks. 
			symbols.findSymbols(exchangeNameVector[i]);

			counter = counter + 1;
			std::cout << "Type BREAK to quit symbol choosing any other key + ENTER to continue. \n";
			std::cin >> BREAK;
			if (BREAK == "BREAK") break;
		} // end of while (1)
	} //End of for loop for different exchanges.
	symbols.combineSymbols();
}
