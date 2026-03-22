// =============================================================================
// FILE: src/Engine/Renderer/VulkanContext.cpp
// PURPOSE: Full Vulkan implementation with all required systems
// =============================================================================

#include "VulkanContext.h"
#include "Engine/Core/Logger.h"
#include "Engine/Core/Timer.h"
#include <set>
#include <algorithm>

namespace Duality {

// ============================================================================
// Construction / Destruction
// ============================================================================

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext() {
    Shutdown();
}

bool VulkanContext::Initialize(Window* window, const VulkanConfig& config) {
    m_window = window;
    m_config = config;
    
    LOG_INFO("Initializing Vulkan context...");
    
    // Create Vulkan instance
    if (!CreateInstance()) {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }
    
    // Setup debug messenger
    if (config.enableValidationLayers && !SetupDebugMessenger()) {
        LOG_WARN("Failed to setup debug messenger");
    }
    
    // Create surface
    if (!CreateSurface()) {
        LOG_ERROR("Failed to create surface");
        return false;
    }
    
    // Select physical device
    if (!SelectPhysicalDevice()) {
        LOG_ERROR("Failed to select physical device");
        return false;
    }
    
    // Create logical device
    if (!CreateLogicalDevice()) {
        LOG_ERROR("Failed to create logical device");
        return false;
    }
    
    // Create memory allocator
    if (!CreateAllocator()) {
        LOG_ERROR("Failed to create memory allocator");
        return false;
    }
    
    // Create command pools
    if (!CreateCommandPools()) {
        LOG_ERROR("Failed to create command pools");
        return false;
    }
    
    // Create swapchain
    if (!CreateSwapchain()) {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }
    
    LOG_INFO("Vulkan context initialized successfully");
    LOG_INFO("  GPU: {}", m_deviceProperties.deviceName);
    LOG_INFO("  API Version: {}.{}.{}", 
        VK_VERSION_MAJOR(m_deviceProperties.apiVersion),
        VK_VERSION_MINOR(m_deviceProperties.apiVersion),
        VK_VERSION_PATCH(m_deviceProperties.apiVersion));
    LOG_INFO("  Ray Tracing: {}", m_hasRayTracing ? "Enabled" : "Disabled");
    
    return true;
}

void VulkanContext::Shutdown() {
    if (m_device) {
        WaitIdle();
    }
    
    DestroySwapchain();
    
    if (m_graphicsCommandPool) {
        vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
        m_graphicsCommandPool = VK_NULL_HANDLE;
    }
    
    if (m_computeCommandPool) {
        vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);
        m_computeCommandPool = VK_NULL_HANDLE;
    }
    
    if (m_transferCommandPool) {
        vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
        m_transferCommandPool = VK_NULL_HANDLE;
    }
    
    if (m_allocator) {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
    
    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    
    if (m_surface) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    
    if (m_debugMessenger) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func) {
            func(m_instance, m_debugMessenger, nullptr);
        }
        m_debugMessenger = VK_NULL_HANDLE;
    }
    
    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
    
    LOG_INFO("Vulkan context shutdown complete");
}

void VulkanContext::WaitIdle() {
    if (m_device) {
        vkDeviceWaitIdle(m_device);
    }
}

// ============================================================================
// Instance Creation
// ============================================================================

bool VulkanContext::CreateInstance() {
    // Application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Duality: Apocalypse";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Duality Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    
    // Instance extensions
    std::vector<const char*> extensions;
    
    // Get GLFW extensions
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (u32 i = 0; i < glfwExtensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }
    
    // Add debug extension if enabled
    if (m_config.enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    // Check if all extensions are supported
    u32 availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
    
    for (const auto* ext : extensions) {
        bool found = false;
        for (const auto& available : availableExtensions) {
            if (strcmp(ext, available.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOG_ERROR("Extension not supported: {}", ext);
            return false;
        }
    }
    
    // Instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    // Validation layers
    if (m_config.enableValidationLayers) {
        if (!CheckValidationLayerSupport()) {
            LOG_ERROR("Validation layers requested but not available");
            return false;
        }
        
        createInfo.enabledLayerCount = static_cast<u32>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create instance
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Vulkan instance");
        return false;
    }
    
    LOG_INFO("Vulkan instance created");
    return true;
}

bool VulkanContext::CheckValidationLayerSupport() {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : s_validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// Debug Messenger
// ============================================================================

bool VulkanContext::SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) {
        return func(m_instance, &createInfo, nullptr, &m_debugMessenger) == VK_SUCCESS;
    }
    
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE("Vulkan: {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("Vulkan: {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN("Vulkan: {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR("Vulkan: {}", pCallbackData->pMessage);
            break;
        default:
            break;
    }
    
    return VK_FALSE;
}

// ============================================================================
// Surface Creation
// ============================================================================

bool VulkanContext::CreateSurface() {
    if (!m_window || !m_window->GetHandle()) {
        LOG_ERROR("Invalid window handle");
        return false;
    }
    
    return glfwCreateWindowSurface(m_instance, m_window->GetHandle(), nullptr, &m_surface) == VK_SUCCESS;
}

// ============================================================================
// Physical Device Selection
// ============================================================================

bool VulkanContext::SelectPhysicalDevice() {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        LOG_ERROR("No Vulkan capable GPUs found");
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    // Score and select best device
    struct DeviceScore {
        VkPhysicalDevice device;
        u32 score;
    };
    std::vector<DeviceScore> scoredDevices;
    
    for (const auto& device : devices) {
        u32 score = 0;
        
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);
        
        // Prefer discrete GPUs
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 100;
        }
        
        // Prefer higher performance
        score += properties.limits.maxImageDimension2D / 1000;
        
        // Check for ray tracing support
        if (m_config.enableRayTracing) {
            bool hasRayTracing = false;
            u32 extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
            
            for (const auto& ext : availableExtensions) {
                if (strcmp(ext.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                    hasRayTracing = true;
                    break;
                }
            }
            
            if (hasRayTracing) {
                score += 500;
            }
        }
        
        // Check if device is suitable
        if (IsDeviceSuitable(device)) {
            scoredDevices.push_back({device, score});
        }
    }
    
    if (scoredDevices.empty()) {
        LOG_ERROR("No suitable GPU found");
        return false;
    }
    
    // Select highest scoring device
    std::sort(scoredDevices.begin(), scoredDevices.end(),
        [](const DeviceScore& a, const DeviceScore& b) { return a.score > b.score; });
    
    m_physicalDevice = scoredDevices[0].device;
    
    // Get device properties
    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);
    
    // Check ray tracing support
    if (m_config.enableRayTracing) {
        u32 extensionCount;
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());
        
        for (const auto& ext : availableExtensions) {
            if (strcmp(ext.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                m_hasRayTracing = true;
                break;
            }
        }
    }
    
    LOG_INFO("Selected GPU: {}", m_deviceProperties.deviceName);
    LOG_INFO("  Score: {}", scoredDevices[0].score);
    
    return true;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = FindQueueFamilies(device);
    
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    for (u32 i = 0; i < queueFamilyCount; i++) {
        // Graphics
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        // Compute
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }
        
        // Transfer (prefer dedicated if available)
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && 
            !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.transferFamily = i;
        }
        
        // Present
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        
        if (indices.IsComplete()) break;
    }
    
    // If no dedicated transfer queue, use graphics queue
    if (!indices.transferFamily.has_value()) {
        indices.transferFamily = indices.graphicsFamily;
    }
    
    return indices;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    std::set<std::string> requiredExtensions(s_deviceExtensions.begin(), s_deviceExtensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanContext::QuerySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
    
    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
    }
    
    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

// ============================================================================
// Logical Device Creation
// ============================================================================

bool VulkanContext::CreateLogicalDevice() {
    m_queueFamilies = FindQueueFamilies(m_physicalDevice);
    
    // Queue priorities
    f32 queuePriority = 1.0f;
    
    // Unique queue families
    std::set<u32> uniqueQueueFamilies = {
        m_queueFamilies.graphicsFamily.value(),
        m_queueFamilies.presentFamily.value(),
        m_queueFamilies.computeFamily.value(),
        m_queueFamilies.transferFamily.value()
    };
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (u32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.shaderInt64 = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;
    
    // Descriptor indexing features
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    
    // Ray tracing features
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    
    if (m_config.enableRayTracing && m_hasRayTracing) {
        rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rayTracingFeatures.rayTracingPipeline = VK_TRUE;
        
        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureFeatures.accelerationStructure = VK_TRUE;
        
        descriptorIndexingFeatures.pNext = &rayTracingFeatures;
        rayTracingFeatures.pNext = &accelerationStructureFeatures;
    }
    
    // Mesh shader features
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    if (m_config.enableMeshShaders) {
        meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        meshShaderFeatures.meshShader = VK_TRUE;
        meshShaderFeatures.taskShader = VK_TRUE;
        
        if (descriptorIndexingFeatures.pNext) {
            auto* next = &rayTracingFeatures;
            while (next->pNext) next = next->pNext;
            next->pNext = &meshShaderFeatures;
        } else {
            descriptorIndexingFeatures.pNext = &meshShaderFeatures;
        }
    }
    
    // Device create info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<u32>(s_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();
    
    // Link feature chain
    createInfo.pNext = &descriptorIndexingFeatures;
    
    // Validation layers
    if (m_config.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<u32>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create device
    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        LOG_ERROR("Failed to create logical device");
        return false;
    }
    
    // Get queues
    vkGetDeviceQueue(m_device, m_queueFamilies.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilies.presentFamily.value(), 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_queueFamilies.computeFamily.value(), 0, &m_computeQueue);
    vkGetDeviceQueue(m_device, m_queueFamilies.transferFamily.value(), 0, &m_transferQueue);
    
    return true;
}

// ============================================================================
// Memory Allocator
// ============================================================================

bool VulkanContext::CreateAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_physicalDevice;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    
    // Set memory pool
    VmaPoolCreateInfo poolInfo{};
    poolInfo.memoryTypeBits = ~0u;
    poolInfo.blockSize = m_config.defaultMemoryPoolSize;
    poolInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
    
    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
        LOG_ERROR("Failed to create VMA allocator");
        return false;
    }
    
    return true;
}

// ============================================================================
// Command Pools
// ============================================================================

bool VulkanContext::CreateCommandPools() {
    // Graphics command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilies.graphicsFamily.value();
    
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_graphicsCommandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create graphics command pool");
        return false;
    }
    
    // Compute command pool
    poolInfo.queueFamilyIndex = m_queueFamilies.computeFamily.value();
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_computeCommandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create compute command pool");
        return false;
    }
    
    // Transfer command pool
    poolInfo.queueFamilyIndex = m_queueFamilies.transferFamily.value();
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create transfer command pool");
        return false;
    }
    
    return true;
}

// ============================================================================
// Swapchain Management
// ============================================================================

bool VulkanContext::CreateSwapchain() {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);
    
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
    
    u32 imageCount = m_config.swapchainImageCount;
    if (imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    if (imageCount < swapChainSupport.capabilities.minImageCount) {
        imageCount = swapChainSupport.capabilities.minImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    u32 queueFamilyIndices[] = {
        m_queueFamilies.graphicsFamily.value(),
        m_queueFamilies.presentFamily.value()
    };
    
    if (m_queueFamilies.graphicsFamily != m_queueFamilies.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
    }
    
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        LOG_ERROR("Failed to create swapchain");
        return false;
    }
    
    // Get swapchain images
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
    
    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;
    
    // Create image views
    m_swapchainImageViews.resize(m_swapchainImages.size());
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
            LOG_ERROR("Failed to create swapchain image view");
            return false;
        }
    }
    
    LOG_INFO("Swapchain created: {}x{} with {} images", 
        extent.width, extent.height, imageCount);
    
    return true;
}

void VulkanContext::DestroySwapchain() {
    for (auto& imageView : m_swapchainImageViews) {
        if (imageView) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
    m_swapchainImageViews.clear();
    m_swapchainImages.clear();
    
    if (m_swapchain) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

VkSurfaceFormatKHR VulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& mode : availablePresentModes) {
        if (mode == m_config.preferredPresentMode) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    }
    
    int width, height;
    glfwGetFramebufferSize(m_window->GetHandle(), &width, &height);
    
    VkExtent2D actualExtent = {
        static_cast<u32>(width),
        static_cast<u32>(height)
    };
    
    actualExtent.width = std::clamp(actualExtent.width, 
        capabilities.minImageExtent.width, 
        capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);
    
    return actualExtent;
}

u32 VulkanContext::AcquireNextImage(VkSemaphore semaphore) {
    u32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, 
        semaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return ~0u;
    }
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire swapchain image");
        return ~0u;
    }
    
    return imageIndex;
}

VkResult VulkanContext::Present(VkQueue queue, VkSemaphore waitSemaphore, u32 imageIndex) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    return vkQueuePresentKHR(queue, &presentInfo);
}

} // namespace Duality