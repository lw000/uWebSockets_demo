//============================================================================
// Name        : uWebSockets_demo.cpp
// Author      : lw
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <string>
#include <iostream>

#include "main.h"

#include "log4z.h"
using namespace zsummer::log4z;

int main(int argc, char**argv) {
	if (argc < 2)
		return -1;

	std::string s(argv[1]);

	ILog4zManager::getInstance()->start();

	if (s.compare("s") == 0) {
		test_wb_server(argc, argv);
	} else if (s.compare("c") == 0) {
		test_wb_client(argc, argv);
	} else {

	}
}
