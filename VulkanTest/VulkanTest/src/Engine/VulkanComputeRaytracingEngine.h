#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#ifdef NDEBUG
const bool VALIDATION_LAYERS_ENABLED = false;
#else
const bool VALIDATION_LAYERS_ENABLED = true;
#endif


const std::vector<const char*> enabled_layers = { "VK_LAYER_KHRONOS_validation" };


class VulkanComputeRaytracingEngine
{
public:
	VulkanComputeRaytracingEngine();
	~VulkanComputeRaytracingEngine();



private:
	//validation layer
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);



	void create_glfw_window();
	void create_vulkan_instance();
	void pick_physical_device();


	// GLFW
	GLFWwindow* window;

	// Debug
	vk::DebugUtilsMessengerEXT vulkan_debug_messenger;


	vk::Instance vulkan_instance;
	vk::PhysicalDevice physical_device;

};