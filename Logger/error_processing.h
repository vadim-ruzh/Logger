#pragma once

#include "trace.h"

#define ERROR_RETURN_EX(errorCode, retVal) \
	if(errorCode)\
		for(int16_t __macro_i = 0;;++__macro_i)\
			if(__macro_i)\
				return retVal;\
			else\
				std::cout << "\n" << __FILE__ << ":" << __LINE__ << " ERROR: function " << __FUNCTION__

#define ERROR_RETURN(errorCode) \
	ERROR_RETURN_EX(errorCode, errorCode)

#define USER_ERROR(errorCode)\
	if(errorCode)\
		for(int16_t __macro_i = 0;;++__macro_i)\
			if(__macro_i)\
				return errorCode;\
			else\
				std::cout << "\n"