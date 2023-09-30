#include "VulkanComputeRaytracingEngine.h"
#include <iostream>
#include <algorithm>
#include <exception>
#include <set>

VulkanComputeRaytracingEngine::VulkanComputeRaytracingEngine() 
{
	create_glfw_window();
	create_vulkan_instance();
	pick_physical_device();
	create_logical_device();
	create_swapchain();
	create_pipeline();
}

void VulkanComputeRaytracingEngine::create_glfw_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void VulkanComputeRaytracingEngine::create_vulkan_instance()
{
	vk::ApplicationInfo appInfo{};
	appInfo.pApplicationName = "Hello Raytracing";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	vk::InstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.pApplicationInfo = &appInfo;

	// Extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	// Layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	bool validation_layer_supported = false;
	if (VALIDATION_LAYERS_ENABLED)
	{
		//const std::vector<const char*> enabledLayers = { "VK_LAYER_KHRONOS_validation" };

		// Check if validation_layer is even available
		std::vector<vk::LayerProperties> layer_properties = vk::enumerateInstanceLayerProperties();
		auto layer_it = std::find_if(layer_properties.begin(), layer_properties.end(),
									 [](vk::LayerProperties layer_property)
									 {
										 return strcmp(layer_property.layerName, enabled_layers[0]) == 0;
									 }
		);

		validation_layer_supported = layer_it != layer_properties.end();

		// if the validation_layer is supported
		if (validation_layer_supported)
		{
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
			instanceCreateInfo.ppEnabledLayerNames = enabled_layers.data();

			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = debug_callback;
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
	}

	vulkan_instance = vk::createInstance(instanceCreateInfo);

	if (validation_layer_supported)
	{
		auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan_instance, "vkCreateDebugUtilsMessengerEXT");

		if (CreateDebugUtilsMessengerEXT != nullptr)
		{
			VkDebugUtilsMessengerEXT c_style_debug_messenger;
			CreateDebugUtilsMessengerEXT(vulkan_instance, &debugCreateInfo, nullptr, &c_style_debug_messenger);
			vulkan_debug_messenger = c_style_debug_messenger;
		}
	}

	VkSurfaceKHR cStyleSurface;
	if (glfwCreateWindowSurface(vulkan_instance, window, nullptr, &cStyleSurface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
	surface = cStyleSurface;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanComputeRaytracingEngine::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer:" << std::endl << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


void VulkanComputeRaytracingEngine::pick_physical_device()
{
	std::vector<vk::PhysicalDevice> available_devices = vulkan_instance.enumeratePhysicalDevices();
	
	// Choose first available device that can do all the stuff we want
	// and assign it to physical_device
	for (auto& available_device : available_devices)
	{
		QueueFamilyIndices queue_family_indices_for_device = find_queue_family_indices(available_device);
		if (queue_family_indices_for_device.is_complete())
		{
			queue_family_indices = queue_family_indices_for_device;
			physical_device = available_device;
			return;
		}
	}

	// If this part is reached, no available_device that fits our needs was found
	throw std::exception("No physical device found that fullfills all our needs");
}


VulkanComputeRaytracingEngine::QueueFamilyIndices VulkanComputeRaytracingEngine::find_queue_family_indices(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;
	std::vector<vk::QueueFamilyProperties> queue_families = device.getQueueFamilyProperties();

	for (int i = 0; i < queue_families.size(); i++)
	{
		auto& queue_family = queue_families[i];

		if (queue_family.queueFlags & vk::QueueFlagBits::eCompute)
		{
			indices.compute_family = i;
		}

		if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphics_family = i;
		}

		if (device.getSurfaceSupportKHR(i, surface))
		{
			indices.presenter_family = i;
		}

		if (indices.is_complete())
			break;
	}

	return indices;
}

void VulkanComputeRaytracingEngine::create_logical_device()
{
	std::set<uint32_t> individualQueueIndexes = { queue_family_indices.compute_family.value(),
													  queue_family_indices.graphics_family.value(), 
													  queue_family_indices.presenter_family.value()	};

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (uint32_t queueIndex : individualQueueIndexes)
	{
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures{}; //Empty device features, because we dont need any specific ones

	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	logical_device = physical_device.createDevice(deviceCreateInfo);

	// queues made here
	vk::Queue graphics_queue = logical_device.getQueue(queue_family_indices.graphics_family.value(), 0);
	vk::Queue compute_queue = logical_device.getQueue(queue_family_indices.compute_family.value(), 0);
	vk::Queue presenter_queue = logical_device.getQueue(queue_family_indices.presenter_family.value(), 0);
}

void VulkanComputeRaytracingEngine::create_swapchain()
{
	vk::SurfaceFormatKHR surface_format = choose_surface_format();
	vk::PresentModeKHR present_mode = choose_present_mode();

	vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
	
	vk::Extent2D extent = choose_extent(surface_capabilities);

	uint32_t imageCount = std::min(surface_capabilities.maxImageCount, surface_capabilities.minImageCount + 1);

	vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
		vk::SwapchainCreateFlagsKHR(), //empty flags
		surface,
		imageCount,
		surface_format.format,
		surface_format.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eStorage
	);

	uint32_t queueFamilyIndices[] = {
		queue_family_indices.graphics_family.value(),
		queue_family_indices.compute_family.value(),
		queue_family_indices.presenter_family.value()
	};


	if (queue_family_indices.graphics_family.value() != queue_family_indices.presenter_family.value())
	{	// if graphics_queue is different from presenter_queue, the images need to be able to be shared
		// concurrently, because graphics and presenter will work on the same image, but be different queues
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent; 
		swapchainCreateInfo.queueFamilyIndexCount = 3;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	swapchainCreateInfo.preTransform = surface_capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfo.presentMode = present_mode;
	swapchainCreateInfo.clipped = VK_TRUE;

	swapchain = logical_device.createSwapchainKHR(swapchainCreateInfo);

	std::vector<vk::Image> images = logical_device.getSwapchainImagesKHR(swapchain);
	for (auto& image : images)
	{
		// create a new frame for each image
		image_frames.push_back(ImageFrame(logical_device, image, surface_format.format, extent));
	}
}

vk::SurfaceFormatKHR VulkanComputeRaytracingEngine::choose_surface_format()
{
	std::vector<vk::SurfaceFormatKHR> surface_formats = physical_device.getSurfaceFormatsKHR(surface);
	for (const auto& available_format : surface_formats)
	{
		if (available_format.format == vk::Format::eB8G8R8A8Unorm && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return available_format;
		}
	}
	
	throw std::exception("No fitting surface format found!");
}

vk::PresentModeKHR VulkanComputeRaytracingEngine::choose_present_mode()
{
	std::vector<vk::PresentModeKHR> present_modes = physical_device.getSurfacePresentModesKHR(surface);

	vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
	for (const auto& available_present_mode : present_modes)
	{
		if (available_present_mode == vk::PresentModeKHR::eMailbox)
		{
			present_mode = available_present_mode;
			break;
		}
	}

	return present_mode;
}

vk::Extent2D VulkanComputeRaytracingEngine::choose_extent(vk::SurfaceCapabilitiesKHR surface_capabilities)
{
	vk::Extent2D extent;
	if (surface_capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = surface_capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		extent = vk::Extent2D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
	}
	return extent;
}

void VulkanComputeRaytracingEngine::create_pipeline()
{
	pipeline = ComputePipeline(logical_device);
}



VulkanComputeRaytracingEngine::~VulkanComputeRaytracingEngine()
{
}