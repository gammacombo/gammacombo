#ifndef FitResultDump_h
#define FitResultDump_h

#include <iostream>
#include <fstream>
#include "MethodAbsScan.h"

class FitResultDump {

	public:
		FitResultDump();
		~FitResultDump();

		void dumpResult(std::string ofname, MethodAbsScan *scanner);
		ofstream outf;
};

#endif
