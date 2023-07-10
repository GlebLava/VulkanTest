#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>



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

    uint32_t graphicsAndComputeAndPresentIndex = 1;
    VkQueueFamilyProperties queueFamily = queueFamilies[graphicsAndComputeAndPresentIndex];
    if (!(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT & VK_QUEUE_GRAPHICS_BIT
        && isPresenter(vkPhysicalDevice, graphicsAndComputeAndPresentIndex, vkSurface)))
    {
        throw std::runtime_error("wrong queue! Reconstruct queue logik. Think about using multiple queues");
    }


    float queuePriority = 1.0f;
    // create our 3 queueCreateInfos
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    // queues made here
    VkQueue allQueue;
    // I know that the queueFamilyIndex is 0 on my machine
    vkGetDeviceQueue(vkDevice, graphicsAndComputeAndPresentIndex, 0, &allQueue); 

    /*
     create swap chain
    */




	return 1;
}

