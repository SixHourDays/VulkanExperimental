#pragma once
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR //enables VK_KHR_win32_surface extension
#include <vulkan.h>


void errOut(VkResult error);
void assert(bool condition, const char* msg);

//abstracts out the 'get how many, get that many' pattern
template<typename FP, class Elem, typename ...Args>
void getResultVkArray(FP fp, std::vector<Elem>& rCollection, Args... args)
{
	uint32_t count = 0;
	VkResult res = fp(args..., &count, nullptr);
	if (res == VK_SUCCESS && count > 0)
	{
		rCollection.resize(count);
		res = fp(args..., &count, rCollection.data());
	}
	errOut( res );
}

//abstracts out the 'get how many, get that many' pattern
template<typename FP, class Elem, typename ...Args>
void getVoidVkArray(FP fp, std::vector<Elem>& rCollection, Args... args)
{
	uint32_t count = 0;
	fp(args..., &count, nullptr);
	if (count > 0)
	{
		rCollection.resize(count);
		fp(args..., &count, rCollection.data());
	}
}
