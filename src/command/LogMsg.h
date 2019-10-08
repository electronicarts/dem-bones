///////////////////////////////////////////////////////////////////////////////
//               Dem Bones - Skinning Decomposition Library                  //
//         Copyright (c) 2019, Electronic Arts. All rights reserved.         //
///////////////////////////////////////////////////////////////////////////////



#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

extern int GLOBAL_DBG;
extern ofstream GLOBAL_LOG_FILE_STREAM;

#define msg(level, str) {							\
	if (GLOBAL_DBG>=level) {						\
		cout<<str;							\
		if (GLOBAL_LOG_FILE_STREAM.is_open()) {				\
			GLOBAL_LOG_FILE_STREAM<<str;				\
			GLOBAL_LOG_FILE_STREAM.flush();				\
			fflush(NULL);						\
		}								\
	}									\
}

