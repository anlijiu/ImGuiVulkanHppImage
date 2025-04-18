#ifndef TINYVK_TINYVKBUFFER
#define TINYVK_TINYVKBUFFER
	#include "./TinyVulkan.hpp"

	namespace TINYVULKAN_NAMESPACE {
		/*
			ABVOUT BUFFERS & IMAGES:
				When Creating Buffers:
					Buffers must be initialized with a VkDviceSize, which is the size of the data in BYTES, not the
					size of the data container (number of items). This same principle applies to stagging buffer data.

				There are 3 types of GPU dedicated memory buffers:
					Vertex:		Allows you to send mesh triangle data to the GPU.
					Index:		Allws you to send mapped indices for vertex buffers to the GPU.
					Uniform:	Allows you to send data to shaders using uniforms.
						* Push Constants are an alternative that do not require buffers, simply use: vkCmdPushConstants(...).

				The last buffer type is a CPU memory buffer for transfering data from the CPU to the GPU:
					Staging:	Staging CPU data for transfer to the GPU.

				Render images are for rendering sprites or textures on the GPU (similar to the swap chain, but handled manually).
					The default image layout is: VK_IMAGE_LAYOUT_UNDEFINED
					To render to shaders you must change/transition the layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
					Once the layout is set for transfering you can write data to the image from CPU memory to GPU memory.
					Finally for use in shaders you need to change the layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
		*/

		enum class TinyVkBufferType {
			TINYVK_BUFFER_TYPE_VERTEX,	 /// For passing mesh/triangle vertices for rendering to shaders.
			TINYVK_BUFFER_TYPE_INDEX,	 /// For indexing Vertex information in Vertex Buffers.
			TINYVK_BUFFER_TYPE_UNIFORM,	 /// For passing uniform/shader variable data to shaders.
			TINYVK_BUFFER_TYPE_INDIRECT, /// For writing VkIndirectCommand's to a buffer for Indirect drawing.
			TINYVK_BUFFER_TYPE_STAGING,	 /// For tranfering CPU bound buffer data to the GPU.
			TINYVK_BUFFER_TYPE_STORAGE,  /// For writing data from fragment/compute shaders.
		};

		/// @brief GPU device Buffer for sending data to the render (GPU) device.
		class TinyVkBuffer : public TinyVkDisposable {
		private:
			void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags) {
				VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				bufCreateInfo.size = size;
				bufCreateInfo.usage = usage;

				VmaAllocationCreateInfo allocCreateInfo {};
				allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
				allocCreateInfo.flags = flags;
				
				if (vmaCreateBuffer(renderContext.vkdevice.GetAllocator(), &bufCreateInfo, &allocCreateInfo, &buffer, &memory, &description) != VK_SUCCESS)
					throw TinyVkRuntimeError("TinyVulkan: Could not allocate memory for TinyVkBuffer!");
			}
		
		public:
			std::timed_mutex buffer_lock;
			const TinyVkBufferType bufferType;
			TinyVkRenderContext& renderContext;
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation memory = VK_NULL_HANDLE;
			VmaAllocationInfo description;
			VkDeviceSize size;
			VkFence bufferWaitable;

			/// @brief Deleted copy constructor (dynamic objects are not copyable).
			TinyVkBuffer operator=(const TinyVkBuffer& buffer) = delete;
			
			~TinyVkBuffer() { this->Dispose(); }

			void Disposable(bool waitIdle) {
				if (waitIdle) renderContext.vkdevice.DeviceWaitIdle();

				vmaDestroyBuffer(renderContext.vkdevice.GetAllocator(), buffer, memory);
				vkDestroyFence(renderContext.vkdevice.GetLogicalDevice(), bufferWaitable, VK_NULL_HANDLE);
			}

			/// @brief Creates a VkBuffer of the specified size in bytes with auto-set memory allocation properties by TinyVkBufferType.
			TinyVkBuffer(TinyVkRenderContext& renderContext, VkDeviceSize dataSize, TinyVkBufferType type)
			: renderContext(renderContext), size(dataSize), bufferType(bufferType) {
				onDispose.hook(TinyVkCallback<bool>([this](bool forceDispose) {this->Disposable(forceDispose); }));

				switch (type) {
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_VERTEX:
					CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
					break;
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDEX:
					CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
					break;
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_UNIFORM:
					CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
					break;
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDIRECT:
					CreateBuffer(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
					break;
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_STORAGE:
					CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
					break;
					case TinyVkBufferType::TINYVK_BUFFER_TYPE_STAGING:
					default:
					CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
					break;
				}

				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				if (vkCreateFence(renderContext.vkdevice.GetLogicalDevice(), &fenceInfo, VK_NULL_HANDLE, &bufferWaitable) != VK_SUCCESS)
					throw TinyVkRuntimeError("TinyVulkan: Failed to create synchronization objects for TinyVkBuffer!");
			}

			/// @brief Begins a transfer command and returns the command buffer index pair used for the command allocated from a TinyVkCommandPool.
			std::pair<VkCommandBuffer, int32_t> BeginTransferCmd() {
				std::pair<VkCommandBuffer, int32_t> bufferIndexPair = renderContext.commandPool.LeaseBuffer(true);
				
				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				vkBeginCommandBuffer(bufferIndexPair.first, &beginInfo);
				return bufferIndexPair;
			}

			/// @brief Ends a transfer command and gives the leased/rented command buffer pair back to the TinyVkCommandPool.
			void EndTransferCmd(std::pair<VkCommandBuffer, int32_t> bufferIndexPair) {
				vkEndCommandBuffer(bufferIndexPair.first);

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &bufferIndexPair.first;

				vkQueueSubmit(renderContext.graphicsPipeline.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(renderContext.graphicsPipeline.GetGraphicsQueue());
				vkResetCommandBuffer(bufferIndexPair.first, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
				renderContext.commandPool.ReturnBuffer(bufferIndexPair);
			}

			/// @brief Copies data from the source TinyVkBuffer into this TinyVkBuffer.
			void TransferBufferCmd(TinyVkRenderContext& renderContext, TinyVkBuffer& srcBuffer, TinyVkBuffer& dstBuffer, VkDeviceSize dataSize, VkDeviceSize srceOffset = 0, VkDeviceSize destOffset = 0) {
				std::pair<VkCommandBuffer,int32_t> bufferIndexPair = BeginTransferCmd();

				VkBufferCopy copyRegion{};
				copyRegion.srcOffset = srceOffset;
				copyRegion.dstOffset = destOffset;
				copyRegion.size = dataSize;
				vkCmdCopyBuffer(bufferIndexPair.first, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

				EndTransferCmd(bufferIndexPair);
			}
			
			/// @brief Copies data from CPU accessible memory to GPU accessible memory for a list of buffers.
			void StageBufferDataQueue(std::vector<std::tuple<TinyVkBuffer&, void*, VkDeviceSize, VkDeviceSize, VkDeviceSize>> buffers) {
				TinyVkBuffer& startBuffer = std::get<0>(buffers[0]);
				std::pair<VkCommandBuffer,int32_t> bufferIndexPair = BeginTransferCmd();

				for(std::tuple<TinyVkBuffer&, void*, VkDeviceSize, VkDeviceSize, VkDeviceSize> staging : buffers) {
					TinyVkBuffer& buffer = std::get<0>(staging);
					void* memory = std::get<1>(staging);
					VkDeviceSize size = std::get<2>(staging);
					VkDeviceSize srcOffset = std::get<3>(staging);
					VkDeviceSize dstOffset = std::get<4>(staging);
					memcpy(buffer.description.pMappedData, memory, (size_t)size);
				}

				EndTransferCmd(bufferIndexPair);
			}

			/// @brief Copies data from CPU accessible memory to GPU accessible memory.
			void StageBufferData(void* data, VkDeviceSize dataSize, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) {
				TinyVkBuffer stagingBuffer = TinyVkBuffer(renderContext, dataSize, TinyVkBufferType::TINYVK_BUFFER_TYPE_STAGING);
				memcpy(stagingBuffer.description.pMappedData, data, (size_t)dataSize);
				TransferBufferCmd(renderContext, stagingBuffer, *this, size, srcOffset, dstOffset);
				stagingBuffer.Dispose();
			}

			#pragma region SYNCHRONIZATION
			
			/// @brief Get pipeline stages relative to the current image layout and command buffer recording stage.
			void GetPipelineBarrierStages(TinyVkCmdBufferSubmitStage cmdBufferStage, VkPipelineStageFlags& srcStage, VkPipelineStageFlags& dstStage, VkAccessFlags& srcAccessMask, VkAccessFlags& dstAccessMask) {
				if (cmdBufferStage == TinyVkCmdBufferSubmitStage::TINYVK_BEGIN) {
					switch(bufferType) {
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_STAGING:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_STORAGE:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
						srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_VERTEX:
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDEX:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						dstStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_UNIFORM:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						dstStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDIRECT:
						default:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
						srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						dstStage = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
						break;
					}
				}

				if (cmdBufferStage == TinyVkCmdBufferSubmitStage::TINYVK_END) {
					switch(bufferType) {
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_STAGING:
						srcAccessMask = VK_ACCESS_NONE;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
						dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_STORAGE:
						srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
						dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_VERTEX:
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDEX:
						srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
						dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_UNIFORM:
						srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						break;
						case TinyVkBufferType::TINYVK_BUFFER_TYPE_INDIRECT:
						default:
						srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
						dstAccessMask = VK_ACCESS_NONE;
						srcStage = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
						dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						break;
					}
				}

				if (cmdBufferStage == TinyVkCmdBufferSubmitStage::TINYVK_BEGIN_TO_END) {
					srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					srcAccessMask = VK_ACCESS_NONE;
					dstAccessMask = VK_ACCESS_NONE;
				}
			}
			
			/// @brief Get the pipeline barrier info for resource synchronization in buffer pipeline barrier pNext chain.
			VkBufferMemoryBarrier GetPipelineBarrier(TinyVkCmdBufferSubmitStage cmdBufferStage, VkPipelineStageFlags& srcStage, VkPipelineStageFlags& dstStage) {
				VkBufferMemoryBarrier pipelineBarrier {
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.srcAccessMask = VK_ACCESS_NONE, .dstAccessMask = VK_ACCESS_NONE,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.size = VK_WHOLE_SIZE, .offset = 0,
					.buffer = buffer,
				};

				GetPipelineBarrierStages(cmdBufferStage, srcStage, dstStage, pipelineBarrier.srcAccessMask, pipelineBarrier.dstAccessMask);
				return pipelineBarrier;
			}

			/// @brief Get the pipeline barrier info and submit call of vkCmdPipelineBarrier to VkCommandBuffer.
			void MemoryPipelineBarrier(VkCommandBuffer cmdBuffer, TinyVkCmdBufferSubmitStage cmdBufferStage) {
				VkPipelineStageFlags srcStage, dstStage;
				VkBufferMemoryBarrier pipelineBarrier = GetPipelineBarrier(cmdBufferStage, srcStage, dstStage);
				vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, VK_NULL_HANDLE, 1, &pipelineBarrier, 0, VK_NULL_HANDLE);
			}
			
			#pragma endregion
			#pragma region PIPELINE_DESCRIPTORS
			
			/// @brief Creates the data descriptor that represents this buffer when passing into graphicspipeline.SelectWrite*Descriptor().
			VkDescriptorBufferInfo GetBufferDescriptor(VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) { return { buffer, offset, range }; }

			#pragma endregion
			
			/// @brief Get the data/memory size of a vector of objects.
			template<typename T>
			static size_t GetSizeofVector(std::vector<T> vector) { return vector.size() * sizeof(T); }

			/// @brief Get the data/memory size of an array of objects.
			template<typename T, size_t S>
			static size_t GetSizeofArray(std::array<T,S> array) { return S * sizeof(T); }
		};
	}
#endif