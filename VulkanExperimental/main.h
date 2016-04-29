#pragma once
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR //enables VK_KHR_win32_surface extension
#include <vulkan.h>

struct VulkanCommandPool;
struct VulkanDevice;
struct VulkanLayer;
struct VulkanQueue;
struct VulkanQueueFamily;

struct VulkanSurface
{
	VkWin32SurfaceCreateInfoKHR config;
	VkSurfaceKHR handle;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presModes;
};

struct VulkanSwapchain
{
	VkSwapchainCreateInfoKHR config;
	VkSwapchainKHR handle;
	std::vector<VkImage> images;
};

struct VulkanInstance
{
	std::vector<VkExtensionProperties> extensionProperties;
	std::vector<VulkanLayer> layerProperties;
	VkApplicationInfo app_config;
	std::vector<const char*> enabledExtensionNames;
	VkInstanceCreateInfo config;
	VkInstance instance;
	VulkanSurface surface;
	VulkanSwapchain swapchain;
	std::vector<VulkanDevice> devices;
};

struct VulkanLayer
{
	VkLayerProperties properties;
	std::vector<VkExtensionProperties> extensionProperties;
};

struct VulkanDevice
{
	VkPhysicalDevice handle; //refers to physical device
	VkPhysicalDeviceProperties properties;
	std::vector<VkExtensionProperties> extensionProperties;
	std::vector<std::vector<VkExtensionProperties>> layerExtensionProperties;
	std::vector<const char*> enabledExtensionNames;
	VkDeviceCreateInfo config;
	VkDevice device; //refers to logical device representing physical
	std::vector<VulkanQueueFamily> queueFamilies;
};

struct VulkanCommandPool
{
	VkCommandPoolCreateInfo poolConfig;
	VkCommandPool pool;
	VkCommandBufferAllocateInfo bufferConfig;
	std::vector<VkCommandBuffer> cmdBuffers;
};

struct VulkanQueueFamily
{
	int index;
	bool canRenderToSurface;
	bool canRenderToWin32Surface;
	VkQueueFamilyProperties properties;
	VkDeviceQueueCreateInfo config;
	std::vector<float> queuePriorities;
	std::vector<VkQueue> queueHandles;

	VulkanCommandPool commandPool;
};

