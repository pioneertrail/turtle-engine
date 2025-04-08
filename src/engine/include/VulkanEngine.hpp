#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <stdexcept>

class VulkanEngine {
public:
    VulkanEngine();
    ~VulkanEngine();
    void run();

private:
    // Window config - TMNT dojo style!
    GLFWwindow* window;
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;
    const std::string WINDOW_TITLE = "TMNT Dojo - Vulkan Powered";

    // Vulkan core - the sewer pipes of our engine
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    // Debug - Splinter's watchful eye
    VkDebugUtilsMessengerEXT debugMessenger;
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    // Rendering - dojo training visuals
    std::vector<VkImageView> swapchainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // Core methods - ninja moves for setup
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // Vulkan setup - building the dojo
    void createInstance();
    void createSurface();
    void setupPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();

    // Debug setup
    void setupDebugMessenger();
    VkResult createDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, 
                                         const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
    void destroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

    // Rendering setup - dojo visuals
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void drawFrame();

    // Helpers - Splinter's wisdom
    std::vector<const char*> getRequiredExtensions();
    bool checkDeviceSuitability(VkPhysicalDevice dev);
    bool checkValidationLayerSupport();
}; 