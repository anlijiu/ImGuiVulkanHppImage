#include "app.hpp"

#include <array>
#include <stdexcept>
#include <cassert>

namespace lve {
    App::App() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    App::~App() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void App::run() {
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(lveDevice.device());
    }


    std::vector<LveModel::Vertex> App::Sierpinski_trig(std::vector<LveModel::Vertex> verts) {
        std::vector<LveModel::Vertex> outverts;

        for (int i = 0; i < verts.size(); i+=3) {
            LveModel::Vertex v1 = verts[i];
            LveModel::Vertex v2 = verts[i+1];
            LveModel::Vertex v3 = verts[i+2];

            LveModel::Vertex v12 = {(v1.position+v2.position) * 0.5f, (v1.color+v3.color) * 0.5f};
            LveModel::Vertex v13 = {(v1.position+v3.position) * 0.5f, (v2.color+v3.color) * 0.5f};
            LveModel::Vertex v23 = {(v2.position+v3.position) * 0.5f, (v1.color+v2.color) * 0.5f};

            outverts.push_back(v1);
            outverts.push_back(v12);
            outverts.push_back(v13);

            outverts.push_back(v2);
            outverts.push_back(v12);
            outverts.push_back(v23);

            outverts.push_back(v3);
            outverts.push_back(v13);
            outverts.push_back(v23);

            outverts.push_back(v12); 
            outverts.push_back(v13); 
            outverts.push_back(v23);
        }
        return outverts;
    }

    std::vector<LveModel::Vertex> App::Sierpinski(int iter) {
        std::vector<LveModel::Vertex> verts {
            {{0.0f, -0.8f}, {1.0f,0.0f,0.0f}},
            {{0.8f, 0.8f}, {0.0f,1.0f,0.0f}},
            {{-0.8f, 0.8f}, {0.0f,0.0f,1.0f}}
        };

        for (int i = 0; i < iter; i++) {
            verts = Sierpinski_trig(verts);
        }

        return verts;
    }

    void App::loadModels() {
        std::vector<LveModel::Vertex> vertices = Sierpinski(6); 

        lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }

    void App::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }


    void App::createPipeline() {
        assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
      
        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = lveSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
            lveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
      }

    void App::recreateSwapChain() {
        auto extent = lveWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
          extent = lveWindow.getExtent();
          glfwWaitEvents();
        }
        vkDeviceWaitIdle(lveDevice.device());
      
        if (lveSwapChain == nullptr) {
          lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
        } else {
          lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
          if (lveSwapChain->imageCount() != commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
          }
        }
      
        createPipeline();
      }

    
    void App::createCommandBuffers() {
        commandBuffers.resize(lveSwapChain->imageCount());
    
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    
        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void App::freeCommandBuffers() {
        vkFreeCommandBuffers(
            lveDevice.device(),
            lveDevice.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
      }

      void App::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      
        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
          throw std::runtime_error("failed to begin recording command buffer!");
        }
      
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);
      
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();
      
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
      
        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
      
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
      
        lvePipeline->bind(commandBuffers[imageIndex]);
        lveModel->bind(commandBuffers[imageIndex]);
        lveModel->draw(commandBuffers[imageIndex]);
      
        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
          throw std::runtime_error("failed to record command buffer!");
        }
      }

      void App::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapChain->acquireNextImage(&imageIndex);
      
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
          recreateSwapChain();
          return;
        }
      
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
          throw std::runtime_error("failed to acquire swap chain image!");
        }
      
        recordCommandBuffer(imageIndex);
        result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            lveWindow.wasWindowResized()) {
          lveWindow.resetWindowResizedFlag();
          recreateSwapChain();
          return;
        } else if (result != VK_SUCCESS) {
          throw std::runtime_error("failed to present swap chain image!");
        }
      }
}
