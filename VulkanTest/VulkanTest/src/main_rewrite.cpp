//#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>


#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "Engine/VulkanComputeRaytracingEngine.h"

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

int main()
{
	VulkanComputeRaytracingEngine engine;

	return 1;
}

int main1()
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
	vk::ApplicationInfo appInfo{};
	appInfo.pApplicationName = "Hello Raytracing";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	// Extensions
	const std::vector<const char*> enabledLayers = { "VK_LAYER_KHRONOS_validation" };


	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	vk::InstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;
	instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	vk::Instance vkInstance = vk::createInstance(instanceCreateInfo);

	/*
	create the vkSurface
	because we need to do this through glfw we need to do it in C style first
	*/
	VkSurfaceKHR cStyleSurface;
	if (glfwCreateWindowSurface(vkInstance, window, nullptr, &cStyleSurface) != VK_SUCCESS) 
		throw std::runtime_error("failed to create window surface!");
	vk::SurfaceKHR vkSurface = cStyleSurface;

	/*
	pick the physical device
	*/
	std::vector<vk::PhysicalDevice> availableDevices = vkInstance.enumeratePhysicalDevices();
	vk::PhysicalDevice vkPhysicalDevice = availableDevices[0]; // currently we are picking the first device without checking for compatability. Refactor later

	/*
	create the logical device
	*/

	/*
		For raytracing we want a compute queue and a graphics queue and a presenters queue
	*/
	std::vector<vk::QueueFamilyProperties> queueFamilies = vkPhysicalDevice.getQueueFamilyProperties();
	
    int graphicsQueueIndex = -1;
    int computeQueueIndex = -1;
    int presenterQueueIndex = -1;

	for (int i = 0; i < queueFamilies.size(); i++)
	{
		vk::QueueFamilyProperties queueFamily = queueFamilies[i];

		if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) // Not necessary because Vulkan requires computing on the gpu
		{
			computeQueueIndex = i;
		}

		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			graphicsQueueIndex = i;
		}

		if (vkPhysicalDevice.getSurfaceSupportKHR(i, vkSurface))
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

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (int queueIndex : individualQueueIndexes)
	{
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queueIndex);
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

	vk::Device vkDevice = vkPhysicalDevice.createDevice(deviceCreateInfo);

	// queues made here
	vk::Queue graphicsQueue = vkDevice.getQueue(graphicsQueueIndex, 0);
	vk::Queue computeQueue = vkDevice.getQueue(computeQueueIndex, 0);
	vk::Queue presenterQueue = vkDevice.getQueue(presenterQueueIndex, 0);

	/*
	 create swap chain
	*/

	vk::SurfaceCapabilitiesKHR vkSurfaceCapabilities = vkPhysicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
	std::vector<vk::SurfaceFormatKHR> vkSurfaceFormats = vkPhysicalDevice.getSurfaceFormatsKHR(vkSurface);
	std::vector<vk::PresentModeKHR> vkPresentModes = vkPhysicalDevice.getSurfacePresentModesKHR(vkSurface);

	// Choose which format to use
	vk::SurfaceFormatKHR vkSurfaceFormatChosen;
	for (const auto& availableFormat : vkSurfaceFormats)
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			vkSurfaceFormatChosen = availableFormat;
			break;
		}
	}

	// Choose which present mode to use
	vk::PresentModeKHR vkPresentModeChosen = vk::PresentModeKHR::eFifo;
	for (const auto& availablePresentMode : vkPresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			vkPresentModeChosen = availablePresentMode;
			break;
		}
	}

	// Choose swap extend form capabilities
	vk::Extent2D vkExtendChosen;
	if (vkSurfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		vkExtendChosen = vkSurfaceCapabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vkExtendChosen = vk::Extent2D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		vkExtendChosen.width = std::clamp(vkExtendChosen.width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
		vkExtendChosen.height = std::clamp(vkExtendChosen.height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
	}


	uint32_t imageCount = std::min(vkSurfaceCapabilities.maxImageCount, vkSurfaceCapabilities.minImageCount + 1);

	vk::SwapchainCreateInfoKHR swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
		vk::SwapchainCreateFlagsKHR(), //empty flags
		vkSurface,
		imageCount,
		vkSurfaceFormatChosen.format,
		vkSurfaceFormatChosen.colorSpace,
		vkExtendChosen,
		1,
		vk::ImageUsageFlagBits::eStorage
	);

	// Not sure what this part is about myself

	uint32_t queueFamilyIndices[] = { graphicsQueueIndex, computeQueueIndex, presenterQueueIndex };
	if (graphicsQueueIndex != presenterQueueIndex)
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 3;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	swapchainCreateInfo.preTransform = vkSurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchainCreateInfo.presentMode = vkPresentModeChosen;
	swapchainCreateInfo.clipped = VK_TRUE;

	vk::SwapchainKHR vkSwapchain = vkDevice.createSwapchainKHR(swapchainCreateInfo);
	std::vector<vk::Image> vkSwapchainImages = vkDevice.getSwapchainImagesKHR(vkSwapchain);


	vk::Format vkSwapchainImageFormat = vkSurfaceFormatChosen.format;
	vk::Extent2D vkSwapchainImageExtent = vkExtendChosen;

	/*
	create image views
	*/

	std::vector<vk::ImageView> vkSwapchainImageViews(vkSwapchainImages.size());
	for (size_t i = 0; i < vkSwapchainImages.size(); i++)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.image = vkSwapchainImages[i];
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = vkSwapchainImageFormat;
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlags{1};
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vkSwapchainImageViews[i] = vkDevice.createImageView(imageViewCreateInfo);
	}

	/*
	create the descriptor-set-layouts
	*/

	// Create descriptor set layout for the compute pipeline

	//raytracing pipeline layout
	std::vector<vk::DescriptorSetLayoutBinding> vkLayoutBindingsRayTracing(1);

	vk::DescriptorSetLayoutBinding vkLayoutBindingRayTracing;
	vkLayoutBindingRayTracing.binding = 0;
	vkLayoutBindingRayTracing.descriptorType = vk::DescriptorType::eStorageImage;
	vkLayoutBindingRayTracing.descriptorCount = 1;
	vkLayoutBindingRayTracing.stageFlags = vk::ShaderStageFlagBits::eCompute;

	vkLayoutBindingsRayTracing[0] = vkLayoutBindingRayTracing;

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfoRayTracing{};
	descriptorSetLayoutCreateInfoRayTracing.flags = vk::DescriptorSetLayoutCreateFlagBits();
	descriptorSetLayoutCreateInfoRayTracing.bindingCount = 1;
	descriptorSetLayoutCreateInfoRayTracing.pBindings = vkLayoutBindingsRayTracing.data();

	vk::DescriptorSetLayout vkRayTracingPipelineLayout = vkDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfoRayTracing);

	/*
	create Raytracing pipeline
	*/

	//make shader module
	auto sourceCode = readFile("shaders/raytracer.spv");
	vk::ShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.flags = vk::ShaderModuleCreateFlags();
	moduleCreateInfo.codeSize = sourceCode.size();
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());
	
	vk::ShaderModule vkShaderModule = vkDevice.createShaderModule(moduleCreateInfo);

	//Pipeline layout
	vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};
	shaderStageCreateInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
	shaderStageCreateInfo.module = vkShaderModule;
	shaderStageCreateInfo.pName = "main";

	vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.stage = shaderStageCreateInfo;


	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &vkRayTracingPipelineLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

	vk::PipelineLayout vkComputePipelineLayout = vkDevice.createPipelineLayout(pipelineLayoutCreateInfo);

	computePipelineCreateInfo.layout = vkComputePipelineLayout;
	
	//Create pipeline
	vk::Pipeline vkComputePipeline = vkDevice.createComputePipeline(nullptr, computePipelineCreateInfo).value;
	
	// destroy shaders
	vkDevice.destroyShaderModule(vkShaderModule);

	// REFACTOR FOLLOWING i am not sure what here is needed or not
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
	inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;

	vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = {}; //Not sure if rasterizer is needed?
	rasterizerCreateInfo.flags = vk::PipelineRasterizationStateCreateFlags();
	rasterizerCreateInfo.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
	rasterizerCreateInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
	rasterizerCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
	
	vk::PipelineMultisampleStateCreateInfo multisamplingCreateInfo = {}; //Not sure if multisampling is needed
	multisamplingCreateInfo.flags = vk::PipelineMultisampleStateCreateFlags();
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

	// Attachement
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	// make command pool
	vk::CommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;

	vk::CommandPool vkCommandPool = vkDevice.createCommandPool(commandPoolCreateInfo);

	vk::CommandBufferAllocateInfo commandBufferAllocInfo{};
	commandBufferAllocInfo.commandPool = vkCommandPool;
	commandBufferAllocInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocInfo.commandBufferCount = 1;

	// create "main" command buffer
	vk::CommandBuffer vkCommandBuffer = vkDevice.allocateCommandBuffers(commandBufferAllocInfo)[0];

	//create frame command buffers
	vk::CommandBufferAllocateInfo commandBufferAllocInfoForImageBuffers = {};
	commandBufferAllocInfoForImageBuffers.commandPool = vkCommandPool;
	commandBufferAllocInfoForImageBuffers.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocInfoForImageBuffers.commandBufferCount = 1;

	std::vector<vk::CommandBuffer> vkImageCommandBuffers = std::vector<vk::CommandBuffer>(vkSwapchainImages.size());

	//make a command buffer for each frame/image
	for (int i = 0; i < vkSwapchainImages.size(); i++)
	{
		vkImageCommandBuffers[i] = vkDevice.allocateCommandBuffers(commandBufferAllocInfoForImageBuffers)[0];
	}


	/*
	create pools for each frame?
	*/
	
	vk::DescriptorPoolSize poolSize;
	poolSize.type = vk::DescriptorType::eStorageImage;
	poolSize.descriptorCount = vkSwapchainImages.size();

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlags();
	descriptorPoolCreateInfo.maxSets = vkSwapchainImages.size();
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &poolSize; // We only have one pool currently

	vk::DescriptorPool vkDescriptorPool = vkDevice.createDescriptorPool(descriptorPoolCreateInfo);

	std::vector<vk::Fence> vkSwapchainFences = std::vector<vk::Fence>(vkSwapchainImages.size());
	std::vector<vk::Semaphore> vkSwapchainImageAvailable = std::vector<vk::Semaphore>(vkSwapchainImages.size());
	std::vector<vk::Semaphore> vkSwapchainRenderFinished = std::vector<vk::Semaphore>(vkSwapchainImages.size());

	std::vector<vk::DescriptorImageInfo> vkDescriptorImageInfos = std::vector<vk::DescriptorImageInfo>(vkSwapchainImages.size());
	std::vector<vk::DescriptorSet> vkDescriptorImageSets = std::vector<vk::DescriptorSet>(vkSwapchainImages.size());
	std::vector<vk::WriteDescriptorSet> vkWriteOpsFrame = std::vector<vk::WriteDescriptorSet>(vkSwapchainImages.size());

	for (int i = 0; i < vkSwapchainImages.size(); i++)
	{
		vk::FenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.flags = vk::FenceCreateFlags() | vk::FenceCreateFlagBits::eSignaled;

		vkSwapchainFences[i] = vkDevice.createFence(fenceCreateInfo);

		vk::SemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.flags = vk::SemaphoreCreateFlags();


		vkSwapchainImageAvailable[i] = vkDevice.createSemaphore(semaphoreCreateInfo);
		vkSwapchainRenderFinished[i] = vkDevice.createSemaphore(semaphoreCreateInfo);

		vkDescriptorImageInfos[i].imageLayout = vk::ImageLayout::eGeneral;
		vkDescriptorImageInfos[i].imageView = vkSwapchainImageViews[i];
		vkDescriptorImageInfos[i].sampler = nullptr;

		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfoFrames{};
		descriptorSetAllocateInfoFrames.descriptorPool = vkDescriptorPool;
		descriptorSetAllocateInfoFrames.descriptorSetCount = 1;
		descriptorSetAllocateInfoFrames.pSetLayouts = &vkRayTracingPipelineLayout;
		
		vkDescriptorImageSets[i] = vkDevice.allocateDescriptorSets(descriptorSetAllocateInfoFrames)[0];

		vk::WriteDescriptorSet colorBufferOp;

		colorBufferOp.dstSet = vkDescriptorImageSets[i];
		colorBufferOp.dstBinding = 0;
		colorBufferOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
		colorBufferOp.descriptorCount = 1;
		colorBufferOp.descriptorType = vk::DescriptorType::eStorageImage;
		colorBufferOp.pImageInfo = &vkDescriptorImageInfos[i];

		vkWriteOpsFrame[i] = { colorBufferOp };
	}



	int frameNumber = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		

		//render
		vkDevice.waitForFences(1, &vkSwapchainFences[frameNumber], VK_TRUE, UINT64_MAX);
		vkDevice.resetFences(1, &vkSwapchainFences[frameNumber]);

		uint32_t imageIndex;

		vk::ResultValue acquire = vkDevice.acquireNextImageKHR(vkSwapchain, UINT64_MAX, vkSwapchainImageAvailable[frameNumber], nullptr);
		imageIndex = acquire.value;

		//update any descriptor sets
		vkDevice.updateDescriptorSets(vkWriteOpsFrame[frameNumber], nullptr);

		//Compute raytrace
		vk::CommandBuffer commandBuffer = vkImageCommandBuffers[frameNumber];
		commandBuffer.reset();

		commandBuffer.begin(vk::CommandBufferBeginInfo());

		// I dont even know
		vk::Image image = vkSwapchainImages[imageIndex];
		vk::ImageSubresourceRange access;
		access.aspectMask = vk::ImageAspectFlagBits::eColor;
		access.baseMipLevel = 0;
		access.levelCount = 1;
		access.baseArrayLayer = 0;
		access.layerCount = 1;
		vk::ImageMemoryBarrier barrier;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eGeneral;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = access;
		vk::PipelineStageFlags sourceStage, destinationStage;

		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

		barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
		destinationStage = vk::PipelineStageFlagBits::eComputeShader;
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

		//dispatch raytrace
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, vkComputePipeline);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, vkComputePipelineLayout, 0, vkDescriptorImageSets[imageIndex], nullptr);

		commandBuffer.dispatch(static_cast<uint32_t>(vkSwapchainImageExtent.width / 8), static_cast<uint32_t>(vkSwapchainImageExtent.height / 8), 1);

		//prepare to present barrier
		vk::ImageSubresourceRange access2;
		access2.aspectMask = vk::ImageAspectFlagBits::eColor;
		access2.baseMipLevel = 0;
		access2.levelCount = 1;
		access2.baseArrayLayer = 0;
		access2.layerCount = 1;

		vk::ImageMemoryBarrier barrier2;
		barrier2.oldLayout = vk::ImageLayout::eGeneral;
		barrier2.newLayout = vk::ImageLayout::ePresentSrcKHR;
		barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier2.image = image;
		barrier2.subresourceRange = access2;

		vk::PipelineStageFlags sourceStage2, destinationStage2;

		barrier2.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
		sourceStage2 = vk::PipelineStageFlagBits::eComputeShader;

		barrier2.dstAccessMask = vk::AccessFlagBits::eNoneKHR;
		destinationStage2 = vk::PipelineStageFlagBits::eBottomOfPipe;

		commandBuffer.pipelineBarrier(sourceStage2, destinationStage2, vk::DependencyFlags(), nullptr, nullptr, barrier2);
		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		vk::Semaphore waitSemaphores[] = { vkSwapchainImageAvailable[frameNumber] };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vk::Semaphore signalSemaphores[] = { vkSwapchainRenderFinished[frameNumber] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		graphicsQueue.submit(submitInfo, vkSwapchainFences[frameNumber]);
		

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		vk::SwapchainKHR swapChains[] = { vkSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		vk::Result present;
		present = presenterQueue.presentKHR(presentInfo);



		frameNumber = (frameNumber + 1) % vkSwapchainImages.size();
	}



	return 1;
}

