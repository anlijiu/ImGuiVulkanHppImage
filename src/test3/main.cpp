
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <volk.h>

#include <VkBootstrap.h>

#include "app.hpp"

#include <iostream>
#include <cstdlib>
#include <iostream>

int main() {
    volkInitialize();
    lve::App app{};

    try{
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
