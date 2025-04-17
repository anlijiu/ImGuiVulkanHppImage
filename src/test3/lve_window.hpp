#pragma once

#include "volk.h"

#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <cstdint>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lve {

    class LveWindow {
        public:
            LveWindow (int w, int h, std::string name);
            ~LveWindow(); //destructor

            LveWindow(const LveWindow &) = delete;
            LveWindow &operator = (const LveWindow &) = delete;

            bool shouldClose() {return glfwWindowShouldClose(window);};
            VkExtent2D getExtent() {return  {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};};
            bool wasWindowResized() {return framebufferResized;};
            void resetWindowResizedFlag() {framebufferResized = false;};

            void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        private:
            static void framebufferResizedCallback(GLFWwindow *window, int width, int height);
            int width;
            int height;
            bool framebufferResized;

            void initWindow();

            std::string windowName;
            GLFWwindow *window;
    };
}// namespace 
