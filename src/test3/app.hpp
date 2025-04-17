#pragma once
#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"

#include <memory>
#include <vector>

namespace lve {
    class App {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            App();
            ~App();
            App(const App &) = delete;
            App &operator = (const App &) = delete;

            void run();
            std::vector<LveModel::Vertex> Sierpinski(int Iter);
            std::vector<LveModel::Vertex> Sierpinski_trig(std::vector<LveModel::Vertex> verts);
        private:
            void loadModels();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);

            LveWindow lveWindow{WIDTH, HEIGHT, "Sierpinski stuff!"};
            LveDevice lveDevice{lveWindow};
            
            std::unique_ptr<LveSwapChain> lveSwapChain;
            std::unique_ptr<LvePipeline> lvePipeline;

            VkPipelineLayout pipelineLayout;
            
            std::vector<VkCommandBuffer> commandBuffers;
            std::unique_ptr<LveModel> lveModel;
    };
}