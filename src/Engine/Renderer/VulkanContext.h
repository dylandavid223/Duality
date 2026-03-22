// =============================================================================
// FILE: src/Engine/Renderer/VulkanContext.h
// PURPOSE: Vulkan instance, device, and swapchain management
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Window.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include <optional>

namespace Duality {

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
    std::optional<u32> computeFamily;
    std::optional<u32> transferFamily;
    
    [[nodiscard]] bool IsComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanConfig {
    // Validation
    bool enableValidationLayers = true;
    bool enableGPUValidation = false;
    
    // Device features
    bool enableRayTracing = true;
    bool enableMeshShaders = true;
    bool enableAsyncCompute = true;
    
    // Swapchain
    u32 swapchainImageCount = 3;
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    
    // Memory
    u64 defaultMemoryPoolSize = 256 * Constants::MB;
};

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();
    
    bool Initialize(Window* window, const VulkanConfig& config = VulkanConfig());
    void Shutdown();
    
    // Wait for device idle
    void WaitIdle();
    
    // Getter functions
    [[nodiscard]] VkInstance GetInstance() const { return m_instance; }
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    [[nodiscard]] VkDevice GetDevice() const { return m_device; }
    [[nodiscard]] VkSurfaceKHR GetSurface() const { return m_surface; }
    [[nodiscard]] VmaAllocator GetAllocator() const { return m_allocator; }
    [[nodiscard]] VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
    [[nodiscard]] VkQueue GetPresentQueue() const { return m_presentQueue; }
    [[nodiscard]] VkQueue GetComputeQueue() const { return m_computeQueue; }
    [[nodiscard]] VkQueue GetTransferQueue() const { return m_transferQueue; }
    
    [[nodiscard]] u32 GetGraphicsQueueFamily() const { return m_queueFamilies.graphicsFamily.value(); }
    [[nodiscard]] u32 GetPresentQueueFamily() const { return m_queueFamilies.presentFamily.value(); }
    [[nodiscard]] u32 GetComputeQueueFamily() const { return m_queueFamilies.computeFamily.value(); }
    [[nodiscard]] u32 GetTransferQueueFamily() const { return m_queueFamilies.transferFamily.value(); }
    
    [[nodiscard]] VkCommandPool GetGraphicsCommandPool() const { return m_graphicsCommandPool; }
    [[nodiscard]] VkCommandPool GetComputeCommandPool() const { return m_computeCommandPool; }
    [[nodiscard]] VkCommandPool GetTransferCommandPool() const { return m_transferCommandPool; }
    
    // Swapchain
    bool CreateSwapchain();
    void DestroySwapchain();
    
    [[nodiscard]] VkSwapchainKHR GetSwapchain() const { return m_swapchain; }
    [[nodiscard]] VkFormat GetSwapchainImageFormat() const { return m_swapchainImageFormat; }
    [[nodiscard]] VkExtent2D GetSwapchainExtent() const { return m_swapchainExtent; }
    [[nodiscard]] const std::vector<VkImage>& GetSwapchainImages() const { return m_swapchainImages; }
    [[nodiscard]] const std::vector<VkImageView>& GetSwapchainImageViews() const { return m_swapchainImageViews; }
    
    // Frame synchronization
    [[nodiscard]] u32 AcquireNextImage(VkSemaphore semaphore);
    [[nodiscard]] VkResult Present(VkQueue queue, VkSemaphore waitSemaphore, u32 imageIndex);
    
    // Device properties
    [[nodiscard]] VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const { return m_deviceProperties; }
    [[nodiscard]] VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const { return m_deviceMemoryProperties; }
    [[nodiscard]] bool HasRayTracing() const { return m_hasRayTracing; }
    
private:
    bool CreateInstance();
    bool SetupDebugMessenger();
    bool CreateSurface();
    bool SelectPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateAllocator();
    bool CreateCommandPools();
    
    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    
    Window* m_window = nullptr;
    VulkanConfig m_config;
    
    // Vulkan handles
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    
    // Queues
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkQueue m_computeQueue = VK_NULL_HANDLE;
    VkQueue m_transferQueue = VK_NULL_HANDLE;
    
    // Command pools
    VkCommandPool m_graphicsCommandPool = VK_NULL_HANDLE;
    VkCommandPool m_computeCommandPool = VK_NULL_HANDLE;
    VkCommandPool m_transferCommandPool = VK_NULL_HANDLE;
    
    // Queue families
    QueueFamilyIndices m_queueFamilies;
    
    // Device properties
    VkPhysicalDeviceProperties m_deviceProperties{};
    VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties{};
    bool m_hasRayTracing = false;
    
    // Swapchain
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainExtent{};
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    
    // Validation layers
    inline static const std::vector<const char*> s_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    // Device extensions
    inline static const std::vector<const char*> s_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME
    };
};

} // namespace Duality