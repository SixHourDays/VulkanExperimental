#include "main.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "utility.h"
#include "mswindows.h"

//important project settings:
// General: character set: not set (avoid windows unicode strings)
// VC++ Directories: includes&libraries: VulkanAPI\ which has only the vulkan needed copied to it
// Linker: input: vulkan-1.lib

int main(int argc, char* argv[])
{
	VulkanInstance vulkan; //our organizing struct

	VkResult res;

	//layer & extension
	const VkExtensionProperties* pKHRSurfaceExt = nullptr;
	const VkExtensionProperties* pKHRWin32SurfaceExt = nullptr;
	{
		//get available loader extensions
		getResultVkArray( &vkEnumerateInstanceExtensionProperties
			, vulkan.extensionProperties, nullptr );
		std::cout << "Loader extensions:" << std::endl;
		for (const VkExtensionProperties& extProp : vulkan.extensionProperties)
		{
			std::cout << "\t" << extProp.extensionName << std::endl;

			if (strcmp(extProp.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
			{
				pKHRSurfaceExt = &extProp;
				continue;
			}
			//because VK_USE_PLATFORM_WIN32_KHR allows it (from main.h)
			if (strcmp(extProp.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
			{
				pKHRWin32SurfaceExt = &extProp;
				continue;
			}
		}
		
		//get available layers
		std::vector<VkLayerProperties> instanceLayerProperties;
		getResultVkArray( &vkEnumerateInstanceLayerProperties, instanceLayerProperties);

		//munge
		vulkan.layerProperties.resize(instanceLayerProperties.size());
		for (int i = 0; i < instanceLayerProperties.size(); ++i)
		{
			vulkan.layerProperties[i].properties = instanceLayerProperties[i];
		}

		//get available layer extensions
		std::cout << "Available layers:" << std::endl;
		for (VulkanLayer& rLayer : vulkan.layerProperties)
		{
			getResultVkArray( &vkEnumerateInstanceExtensionProperties
				, rLayer.extensionProperties, rLayer.properties.layerName);

			std::cout << "\t" << rLayer.properties.layerName
				<< " v" << rLayer.properties.implementationVersion
				<< " " << rLayer.properties.description << std::endl;
			if ( rLayer.extensionProperties.size() > 0 )
			{

				std::cout << "\tExtensions:" << std::endl;
				for (const VkExtensionProperties& extProp : rLayer.extensionProperties)
				{
					std::cout << "\t\t" << extProp.extensionName << std::endl;
				}
			}
		}
	}

	//Ensure:
	//	KHR_SURFACE
	//	KHR_WIN32_SURFACE
	//exist
	assert( pKHRSurfaceExt && pKHRWin32SurfaceExt, "Missing required loader extensions");
	vulkan.enabledExtensionNames.push_back(pKHRSurfaceExt->extensionName);
	vulkan.enabledExtensionNames.push_back(pKHRWin32SurfaceExt->extensionName);

	// initialize the VkApplicationInfo structure
	vulkan.app_config.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vulkan.app_config.pNext = nullptr;
	vulkan.app_config.pApplicationName = "VK Experi";
	vulkan.app_config.applicationVersion = 1;
	vulkan.app_config.pEngineName = "Brax";
	vulkan.app_config.engineVersion = 1;
	//TODOJEFFGIFFEN update driver to 1.0.5
	vulkan.app_config.apiVersion = VK_MAKE_VERSION(1, 0, 4);//VK_API_VERSION;

	// initialize the VkInstanceCreateInfo structure
	vulkan.config.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vulkan.config.pNext = nullptr;
	vulkan.config.flags = 0;
	vulkan.config.pApplicationInfo = &vulkan.app_config;
	vulkan.config.enabledExtensionCount = (uint32_t)vulkan.enabledExtensionNames.size();
	vulkan.config.ppEnabledExtensionNames = vulkan.enabledExtensionNames.data();
	vulkan.config.enabledLayerCount = 0;
	vulkan.config.ppEnabledLayerNames = nullptr;

	res = vkCreateInstance(&vulkan.config, nullptr, &vulkan.instance);
	errOut(res);
	

	//create mswindow
	MSWindowBinds windowBinds = CreateMSWindow("VulkanExperiment", 10, 10, 100, 100);
	//	while (true) {MessagePumpMSWindow(binds);}

	//create VkSurfaceKHR with mswindow (uses KHR_WIN32_SURFACE)
	vulkan.surface.config.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vulkan.surface.config.pNext = nullptr;
	vulkan.surface.config.flags = 0;
	vulkan.surface.config.hinstance = windowBinds.instanceHandle;
	vulkan.surface.config.hwnd = windowBinds.windowHandle;
		
	res = vkCreateWin32SurfaceKHR(vulkan.instance, &vulkan.surface.config, nullptr, &vulkan.surface.handle);
	errOut(res);

	//physical device enumeration
	{
		//get phys device handles
		std::vector<VkPhysicalDevice> deviceHandles;
		getResultVkArray(&vkEnumeratePhysicalDevices, deviceHandles, vulkan.instance);
		
		vulkan.devices.resize(deviceHandles.size());
		for (uint32_t i = 0; i < deviceHandles.size(); ++i)
		{
			VulkanDevice& rDevice = vulkan.devices[i];
			rDevice.handle = deviceHandles[i];

			//get phys device props
			vkGetPhysicalDeviceProperties(rDevice.handle, &rDevice.properties);

			std::cout << "Found " << rDevice.properties.deviceName << std::endl;
			std::cout << "\tDriver version " << rDevice.properties.driverVersion << std::endl;
			std::cout << "\tVulkan "
				<< VK_VERSION_MAJOR(rDevice.properties.apiVersion) << "."
				<< VK_VERSION_MINOR(rDevice.properties.apiVersion) << "."
				<< VK_VERSION_PATCH(rDevice.properties.apiVersion) << std::endl;

			//get available device extensions
			getResultVkArray(&vkEnumerateDeviceExtensionProperties
				, rDevice.extensionProperties, rDevice.handle, nullptr);
			std::cout << "\tAvailable extensions:" << std::endl;
			for (const VkExtensionProperties& extProp : rDevice.extensionProperties)
			{
				std::cout << "\t\t" << extProp.extensionName << std::endl;
			}

			//get available device layer extensions
			rDevice.layerExtensionProperties.resize(vulkan.layerProperties.size());
			for (int j = 0; j < vulkan.layerProperties.size(); ++j )
			{
				const VulkanLayer& layer = vulkan.layerProperties[j];
				std::vector<VkExtensionProperties>& rExtensions = rDevice.layerExtensionProperties[j];

				getResultVkArray(&vkEnumerateDeviceExtensionProperties
					, rExtensions, rDevice.handle, layer.properties.layerName);

				if (rExtensions.size() > 0)
				{
					std::cout << "\tLayer " << layer.properties.layerName << " extensions:" << std::endl;
					for (const VkExtensionProperties& extProp : rExtensions)
					{
						std::cout << "\t\t" << extProp.extensionName << std::endl;
					}
				}
			}
			
			//get phys device queue	fam props
			std::vector<VkQueueFamilyProperties> queFamProps;
			getVoidVkArray(&vkGetPhysicalDeviceQueueFamilyProperties, queFamProps, rDevice.handle);
			
			//data munge + query presentation support
			rDevice.queueFamilies.resize(queFamProps.size());
			for (uint32_t j = 0; j < queFamProps.size(); ++j)
			{
				VulkanQueueFamily &rQueFam = rDevice.queueFamilies[j];
				rQueFam.index = j;
				rQueFam.properties = queFamProps[j];

				//can physDevice+queue+surface present
				VkBool32 result = VK_FALSE;
				res = vkGetPhysicalDeviceSurfaceSupportKHR( rDevice.handle,
					rQueFam.index, vulkan.surface.handle, &result);
				errOut(res);
				rQueFam.canRenderToSurface = result == VK_TRUE;

				//can physdevice+queue draw to mswindows
				result = vkGetPhysicalDeviceWin32PresentationSupportKHR(rDevice.handle, rQueFam.index);
				rQueFam.canRenderToWin32Surface = result == VK_TRUE;
					
				std::cout << "\tQueue Family " << j << " has " << rQueFam.properties.queueCount << " queues"
					<< " Graphics: " << ((rQueFam.properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
					<< " Compute: " << ((rQueFam.properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
					<< " Transfer: " << ((rQueFam.properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
					<< " SparseBind: " << ((rQueFam.properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0)
					<< " Present Surface: " << rQueFam.canRenderToSurface
					<< " Present win32: " << rQueFam.canRenderToWin32Surface << std::endl;
			}
		}
	} //end physical device enumeration

	//TODOJEFFGIFFEN arbitrary single device choice here
	//choose phys device (naive pick 1st discreete gpu w graphics queues w win32 surface support)
	VulkanDevice* pDevice = nullptr;
	VulkanQueueFamily* pQueFamily = nullptr;
	for (VulkanDevice& device : vulkan.devices)
	{
		if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			for (VulkanQueueFamily& queFam : device.queueFamilies )
			{
				if ((queFam.properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
					&& queFam.canRenderToSurface
					&& queFam.canRenderToWin32Surface)
				{
					pDevice = &device;
					pQueFamily = &queFam;
					goto doubleBreak;
				}
			}
		}
	}
	doubleBreak:
	assert(pDevice != nullptr, "No discrete GPU with a graphics queue found");

	//Ensure:
	//	KHR_SWAPCHAIN
	//exist
	const VkExtensionProperties* pKHRSwapchainExt = nullptr;
	for (const VkExtensionProperties& extProp : pDevice->extensionProperties)
	{
		if (strcmp(extProp.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			pKHRSwapchainExt = &extProp;
			break;
		}
	}
	assert(pKHRSwapchainExt != nullptr, "Missing required device extensions");
	pDevice->enabledExtensionNames.push_back(pKHRSwapchainExt->extensionName);

	//TODOJEFFGIFFEN with multiple families detected, we would
	//walk through here discarding those we dont want to instantiate.
	//so queueFamilies goes from potential array to instantiated array
	assert(pDevice->queueFamilies.size() == 1, "Single family assumption false!");

	//configure family's queue priorities
	pQueFamily->queuePriorities.resize(pQueFamily->properties.queueCount);
	for ( float& f : pQueFamily->queuePriorities)
	{	f = 1.f; } //1 = max prio, 0 = min prio

	//TODOJEFFGIFFEN arbitrary single que family used here
	//configure queue families in logical device
	pQueFamily->config.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	pQueFamily->config.pNext = nullptr;
	pQueFamily->config.flags = 0;
	pQueFamily->config.queueFamilyIndex = pQueFamily->index;
	pQueFamily->config.queueCount = pQueFamily->properties.queueCount;
	pQueFamily->config.pQueuePriorities = pQueFamily->queuePriorities.data();
	
	//if we were creating more, we'd need a contiguous array of VkDeviceQueueCreateInfo
	//std::vector<VkDeviceQueueCreateInfo> queConfigArray(1)

	//create logical device representing chosen physical device
	pDevice->config.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	pDevice->config.pNext = nullptr;
	pDevice->config.flags = 0;
	pDevice->config.queueCreateInfoCount = (uint32_t)pDevice->queueFamilies.size();
	pDevice->config.pQueueCreateInfos = &pQueFamily->config;
	pDevice->config.enabledLayerCount = 0;
	pDevice->config.ppEnabledLayerNames = nullptr;
	pDevice->config.enabledExtensionCount = (uint32_t)pDevice->enabledExtensionNames.size();
	pDevice->config.ppEnabledExtensionNames = pDevice->enabledExtensionNames.data();
	pDevice->config.pEnabledFeatures = nullptr;

	res = vkCreateDevice(pDevice->handle, &pDevice->config, nullptr, &pDevice->device);
	errOut(res);

	//retrieve queue handles
	pQueFamily->queueHandles.resize(pQueFamily->config.queueCount);
	for (uint32_t i = 0; i < pQueFamily->config.queueCount; ++i)
	{
		VkQueue& rQueHandle = pQueFamily->queueHandles[i];
		vkGetDeviceQueue(pDevice->device, pQueFamily->index, i, &rQueHandle);
	}


	//enumerate surface modes
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice->handle, vulkan.surface.handle, &vulkan.surface.capabilities);
	{
		const VkSurfaceCapabilitiesKHR& caps = vulkan.surface.capabilities;
		//check surface can do double buffering
		assert(caps.minImageCount <= 2 && caps.maxImageCount >= 2, "surface doesn't support double buffering");
		//check surface can fb color attach
		assert(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "surface can't color attach to FB");
		//check system compositer displays default orientation
		assert(caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, "surface orientation isnt default");
		//check system compositer does opaque blend
		assert(caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, "surface doesn't do opaque blend");
	}
	
	getResultVkArray(&vkGetPhysicalDeviceSurfaceFormatsKHR,
		vulkan.surface.formats, pDevice->handle, vulkan.surface.handle);
	{
		//check for BGRA8 support
		bool foundBGRA8 = false;
		for (const VkSurfaceFormatKHR& surfFormat : vulkan.surface.formats)
		{
			if (surfFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				foundBGRA8 = true;
				break;
			}
		}
		assert(foundBGRA8, "surface doesn't support BGRA8");
	}

	getResultVkArray(&vkGetPhysicalDeviceSurfacePresentModesKHR,
		vulkan.surface.presModes, pDevice->handle, vulkan.surface.handle);
	{
		//no need to check for vsync on, VK_PRESENT_MODE_FIFO_KHR is required by spec
		//could check for VK_PRESENT_MODE_FIFO_RELAXED_KHR to run vsync off
	}

	//create swapchain
	vulkan.swapchain.config.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vulkan.swapchain.config.pNext = nullptr;
	vulkan.swapchain.config.flags = 0;
	vulkan.swapchain.config.surface = vulkan.surface.handle;
	vulkan.swapchain.config.minImageCount = 2; //ensured above
	vulkan.swapchain.config.imageFormat = VK_FORMAT_B8G8R8A8_UNORM; //ensured above
	vulkan.swapchain.config.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR; //only possibility
	vulkan.swapchain.config.imageExtent = vulkan.surface.capabilities.currentExtent;
	vulkan.swapchain.config.imageArrayLayers = 1; //not stereoscopic display
	vulkan.swapchain.config.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //ensured above
	vulkan.swapchain.config.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //only 1 queue family accesses images
	vulkan.swapchain.config.queueFamilyIndexCount = 0; //only used for sharing mode
	vulkan.swapchain.config.pQueueFamilyIndices = nullptr;
	vulkan.swapchain.config.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //ensured above
	vulkan.swapchain.config.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //ensured above
	vulkan.swapchain.config.presentMode = VK_PRESENT_MODE_FIFO_KHR; //standard requires support of this
	vulkan.swapchain.config.clipped = true;
	vulkan.swapchain.config.oldSwapchain = VK_NULL_HANDLE;

	res = vkCreateSwapchainKHR(pDevice->device, &vulkan.swapchain.config, nullptr, &vulkan.swapchain.handle);
	errOut(res);
	
	//retrieve swapchain images
	getResultVkArray(&vkGetSwapchainImagesKHR,
		vulkan.swapchain.images, pDevice->device, vulkan.swapchain.handle);

	//vkAcquireNextImageKHR
	//...do everything
	//vkQueuePresentKHR

	//create command pool for pQueFamily
	VulkanCommandPool* cmdPool = &pQueFamily->commandPool;
	cmdPool->poolConfig.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPool->poolConfig.pNext = nullptr; //TODOJEFFGIFFEN below
	//cmdPool->poolconfig.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; //hint it will be used and freed rapidly
	cmdPool->poolConfig.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //allows reset / rebegin on cmdbuffer
	cmdPool->poolConfig.queueFamilyIndex = pQueFamily->index;
	
	res = vkCreateCommandPool( pDevice->device, &cmdPool->poolConfig, nullptr, &cmdPool->pool);
	errOut(res);
	
	//create command buffers
	cmdPool->bufferConfig.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdPool->bufferConfig.pNext = nullptr;
	cmdPool->bufferConfig.commandPool = cmdPool->pool;
	cmdPool->bufferConfig.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //TODOJEFFGIFFEN
	//cmdPool->bufferConfig.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	cmdPool->bufferConfig.commandBufferCount = 1; //TODOJEFFGIFFEN
	cmdPool->cmdBuffers.resize(cmdPool->bufferConfig.commandBufferCount);

	res = vkAllocateCommandBuffers(pDevice->device, &cmdPool->bufferConfig, cmdPool->cmdBuffers.data());
	errOut(res);

	//record command buffers
	VkCommandBufferBeginInfo cmdBufBeginConfig;
	cmdBufBeginConfig.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufBeginConfig.pNext = nullptr;
	//cmdBufBeginConfig.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//cmdBufBeginConfig.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	//cmdBufBeginConfig.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufBeginConfig.pInheritanceInfo = nullptr; //secondary cmd bufs only

	res = vkBeginCommandBuffer(cmdPool->cmdBuffers[0], &cmdBufBeginConfig);
	errOut(res);
	
	//TODOJEFFGIFFEN
	//vkCmd...

	res = vkEndCommandBuffer(cmdPool->cmdBuffers[0]);
	errOut(res);

	//submit!
	VkSubmitInfo submitConfig;
	submitConfig.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitConfig.pNext = nullptr;
	submitConfig.waitSemaphoreCount = 0; //TODOJEFFGIFFEN
	submitConfig.pWaitSemaphores = nullptr;
	submitConfig.pWaitDstStageMask = nullptr;
	submitConfig.commandBufferCount = 1;
	submitConfig.pCommandBuffers = cmdPool->cmdBuffers.data();//TODOJEFFGIFFEN
	submitConfig.signalSemaphoreCount = 0;  //TODOJEFFGIFFEN
	submitConfig.pSignalSemaphores = nullptr;
	//TODOJEFFGIFFEN try submit a fence
	res = vkQueueSubmit(pQueFamily->queueHandles[0], 1, &submitConfig, VK_NULL_HANDLE);
	errOut(res);

	VkPresentInfoKHR presentConfig;
	presentConfig.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentConfig.pNext = nullptr;
	presentConfig.waitSemaphoreCount = 0; //TODOJEFFGIFFEN
	presentConfig.pWaitSemaphores = nullptr;
	presentConfig.swapchainCount = 1;
	presentConfig.pSwapchains = &vulkan.swapchain.handle;
#error
	presentConfig.pImageIndices = ? ? ? ;
	presentConfig.pResults = ? ? ? ;
	vkQueuePresentKHR( queue, &presentConfig );


	//vkQueueWaitIdle(VkQueue) //blocks for any queue to finish
	res = vkDeviceWaitIdle(pDevice->device); //blocks for all queues to finish
	errOut(res);
	

	vkFreeCommandBuffers(pDevice->device, cmdPool->pool, (uint32_t)cmdPool->cmdBuffers.size(), cmdPool->cmdBuffers.data());
	vkDestroyCommandPool(pDevice->device, pQueFamily->commandPool.pool, nullptr);
	
	vkDestroySwapchainKHR(pDevice->device, vulkan.swapchain.handle, nullptr);

	vkDestroyDevice(pDevice->device, nullptr); //must be idle before we destroy
	
	vkDestroySurfaceKHR(vulkan.instance, vulkan.surface.handle, nullptr);
	DestroyMSWindow(windowBinds);

	vkDestroyInstance(vulkan.instance, nullptr);

	return 0;
}