#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "utils.h"

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>



/*
Ray tracing setup

First stages should be the same, right?
From creating a vulkan instance to creating the logical device

- swap chain
- image views
- specific render pass, without rasterization?
- graphics pipeline

*/


const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool isPresenter(VkPhysicalDevice vkPhysicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR vkSurface)
{
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, queueFamilyIndex, vkSurface, &presentSupport);
	return presentSupport;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}




int main()
{
	/*
	create a glfw window
	*/
	GLFWwindow* window;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(800, 800, "Vulkan", nullptr, nullptr);



	/*
	create vulkan instance
	I dont really care about validation layers right now
	*/
	VkInstance vkInstance;

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	// Extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	/*
	create the vkSurface
	*/
	VkSurfaceKHR vkSurface;
	if (glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	/*
	pick the physical device
	*/
	uint32_t deviceCount = 0;

	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
	vkPhysicalDevice = devices[0]; //Pick first device, dont even check if it has everything we need

	/*
	create the logical device
	*/
	VkDevice vkDevice;

	/*
		For raytracing we want a compute queue and a graphics queue and a presenters queue.
		My gpu has a queue that fullfills all of these requirements
	*/

	// NEVERMIND REFACTOR THIS 

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	
    int graphicsQueueIndex = -1;
    int computeQueueIndex = -1;
    int presenterQueueIndex = -1;

	for (int i = 0; i < queueFamilyCount; i++)
	{
		VkQueueFamilyProperties queueFamily = queueFamilies[i];

		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeQueueIndex = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueIndex = i;
		}

		if (isPresenter(vkPhysicalDevice, i, vkSurface))
		{
			presenterQueueIndex = i;
		}

		if (graphicsQueueIndex != -1 && computeQueueIndex != -1 && presenterQueueIndex != -1)
		{
			break; //because first we have found every queue we need 
		}
	}

	// now we need to give a queueInfo of every INDIVIDUALL queue we have found
	std::set<int> individualQueueIndexes = { graphicsQueueIndex, computeQueueIndex, presenterQueueIndex };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (int queueIndex : individualQueueIndexes)
	{
		uint32_t uintValue = static_cast<uint32_t>(queueIndex);
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uintValue;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}



	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	// queues made here
	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue presenterQueue;

	vkGetDeviceQueue(vkDevice, graphicsQueueIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(vkDevice, computeQueueIndex, 0, &computeQueue);
	vkGetDeviceQueue(vkDevice, presenterQueueIndex, 0, &presenterQueue);

	/*
	 create swap chain
	*/
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImage> swapChainImages;

	// query capabilities of swap chain
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &vkSurfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, formats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, presentModes.data());

	// Choose which format to use
	VkSurfaceFormatKHR surfaceFormatChosen = chooseSwapSurfaceFormat(formats);

	// Choose which present mode to use
	VkPresentModeKHR presentModeChosen = chooseSwapPresentMode(presentModes);

	// Choose swap extend form capabilites
	VkExtent2D extendChosen = chooseSwapExtent(vkSurfaceCapabilities, window);

	uint32_t imageCount = vkSurfaceCapabilities.minImageCount + 1;
	if (imageCount > vkSurfaceCapabilities.maxImageCount)
	{
		imageCount = vkSurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = vkSurface;

	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = surfaceFormatChosen.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormatChosen.colorSpace;
	swapchainCreateInfo.imageExtent = extendChosen;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { graphicsQueueIndex, computeQueueIndex, presenterQueueIndex };

	
	if (graphicsQueueIndex != presenterQueueIndex)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 3;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	
	swapchainCreateInfo.preTransform = vkSurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentModeChosen;
	swapchainCreateInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vkDevice, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormatChosen.format;
	swapChainExtent = extendChosen;

	/*
	create image views
	*/
	std::vector<VkImageView> swapChainImageViews(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}

	/*
	create render pass
	*/
	VkRenderPass renderPass;

	// Define the attachment description for the storage image
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = swapChainImageFormat;// Format of the storage image attachment
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the storage image at the beginning of each frame
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the results in the storage image
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Initial layout of the storage image
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL; // Final layout of the storage image

	// Define the subpass for the ray tracing computations
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // Index of the attachment in the render pass
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Define the subpass dependency (if necessary)
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Create the render pass
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

	/*
	Create the graphics pipeline
	*/


	return 1;
}

