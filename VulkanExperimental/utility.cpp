#include "utility.h"

#include <iostream>


#define ECV(x) std::make_pair( x, #x )
typedef std::vector< std::pair< VkResult, const char*> > EnumCharVec;
EnumCharVec errorCodes = {
//	ECV(VK_SUCCESS),
	ECV(VK_NOT_READY),
	ECV(VK_TIMEOUT),
	ECV(VK_EVENT_SET),
	ECV(VK_EVENT_RESET),
	ECV(VK_INCOMPLETE),
	ECV(VK_ERROR_OUT_OF_HOST_MEMORY),
	ECV(VK_ERROR_OUT_OF_DEVICE_MEMORY),
	ECV(VK_ERROR_INITIALIZATION_FAILED),
	ECV(VK_ERROR_DEVICE_LOST),
	ECV(VK_ERROR_MEMORY_MAP_FAILED),
	ECV(VK_ERROR_LAYER_NOT_PRESENT),
	ECV(VK_ERROR_EXTENSION_NOT_PRESENT),
	ECV(VK_ERROR_FEATURE_NOT_PRESENT),
	ECV(VK_ERROR_INCOMPATIBLE_DRIVER),
	ECV(VK_ERROR_TOO_MANY_OBJECTS),
	ECV(VK_ERROR_FORMAT_NOT_SUPPORTED),
	ECV(VK_ERROR_SURFACE_LOST_KHR),
	ECV(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR),
	ECV(VK_SUBOPTIMAL_KHR),
	ECV(VK_ERROR_OUT_OF_DATE_KHR),
	ECV(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR),
	ECV(VK_ERROR_VALIDATION_FAILED_EXT),
	ECV(VK_ERROR_INVALID_SHADER_NV),
};

void errOut(VkResult error)
{
	if (error == VK_SUCCESS) { return; }

	const char* pMsg = "uncaught error";
	for (EnumCharVec::value_type& pair : errorCodes)
	{
		if (error == pair.first) { pMsg = pair.second; break; }
	}

	std::cout << "Error " << error << ", " << pMsg << ". Abort." << std::endl;
	std::cin.ignore();
	exit( error );
}

void assert(bool condition, const char* msg)
{
	if (condition) { return; }
	
	std::cout << "Assert Failure " << msg << ". Abort." << std::endl;
	std::cin.ignore();
	exit(-1);
}
