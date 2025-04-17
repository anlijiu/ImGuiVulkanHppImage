#include "lve_model.hpp"

// std
#include <cassert>
#include <cstddef>
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace lve {

LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : lveDevice{device} {
  createVertexBuffers(vertices);
}

LveModel::~LveModel() {
  vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
}

void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  lveDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      vertexBuffer,
      vertexBufferMemory);

  void *data;
  vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
}

void LveModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;             //lo stride non cambia quando aggiungo dati come il colore alla posizione dei vertici
  bindingDescriptions[0].stride = sizeof(Vertex); //questo perché sto scrivendo in memoria con interleaving anziché a insiemi
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // pos vec2
  attributeDescriptions[0].offset = offsetof(Vertex, position); // in realtà per adesso è 0 ma utile per correttezza e modifiche future


  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1; //nomi delle variabili intercambiabili, conta la location in memoria
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // color vec3
  attributeDescriptions[1].offset = offsetof(Vertex, color); // distanza in memoria
  return attributeDescriptions;
}

}  // namespace lve