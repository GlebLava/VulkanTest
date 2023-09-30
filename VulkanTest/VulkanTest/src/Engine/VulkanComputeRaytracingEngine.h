#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <optional>
#include "ImageFrame.h"
#include "ComputePipeline.h"

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
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> presenter_family;
		std::optional<uint32_t> compute_family;

		bool is_complete()
		{
			return graphics_family.has_value() && presenter_family.has_value() && compute_family.has_value();
		}
	};

private:
	/*
		the "main" methods are called in order by the constructor
		Methods are listed in this way:
		main_method1();
		helper1_for_method1();
		helper2_for_method1();
		attributes_for_method1;
		
		main_method2();
		...
	*/

	void create_glfw_window();
	GLFWwindow* window;
	vk::SurfaceKHR surface;

	void create_vulkan_instance();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	vk::Instance vulkan_instance;
	vk::DebugUtilsMessengerEXT vulkan_debug_messenger;

	void pick_physical_device();
	QueueFamilyIndices find_queue_family_indices(vk::PhysicalDevice device);
	vk::PhysicalDevice physical_device;
	QueueFamilyIndices queue_family_indices;

	void create_logical_device();
	vk::Device logical_device;
	vk::Queue graphics_queue;
	vk::Queue compute_queue;
	vk::Queue presenter_queue;

	void create_swapchain();
	vk::SurfaceFormatKHR choose_surface_format();
	vk::PresentModeKHR choose_present_mode();
	vk::Extent2D choose_extent(vk::SurfaceCapabilitiesKHR surface_capabilities);
	vk::SwapchainKHR swapchain;
	std::vector<ImageFrame> image_frames;

	void create_pipeline();
	ComputePipeline pipeline;
	
	

};