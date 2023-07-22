#include "VulkanComputeRaytracingEngine.h"
#include <iostream>
#include <algorithm>

VulkanComputeRaytracingEngine::VulkanComputeRaytracingEngine() 
{
	create_glfw_window();
	create_vulkan_instance();
	pick_physical_device();
	
}

void VulkanComputeRaytracingEngine::create_glfw_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(800, 800, "Vulkan", nullptr, nullptr);
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


}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanComputeRaytracingEngine::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer:" << std::endl << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


void VulkanComputeRaytracingEngine::pick_physical_device()
{

}


VulkanComputeRaytracingEngine::~VulkanComputeRaytracingEngine()
{

}