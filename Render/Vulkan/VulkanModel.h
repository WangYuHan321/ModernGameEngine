#pragma once

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include<iostream>
#include<glm/glm.hpp>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

#include "VulkanTexture.h"
#include "../Camera.hpp"

namespace tinygltf
{
	class Model;
	class Node;
}


namespace Render
{
	namespace Vulkan
	{
		/*
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec4 color;
			glm::vec4 joint0;
			glm::vec4 weight0;
			glm::vec4 tangent;
		};

		struct Vertices
		{
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		};

		struct Indices 
		{
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		};

		struct Texture
		{
			VulkanDevice* device = nullptr;
			VkImage image;
			VkImageLayout imageLayout;
			VkDeviceMemory deviceMemory;
			VkImageView view;
			uint32_t width, height;
			uint32_t mipLevel;
			uint32_t layerCount;
			VkDescriptorImageInfo descriptor;
			VkSampler sampler;
			uint32_t index;

			void UpdateDescriptor() {};
			void Destroy() {};
		};

		struct Material
		{
			VulkanDevice* device = nullptr;
			enum AlphaMode { ALPHA_OPAQUE, ALPHA_MASK, ALPHA_BLEND };
			AlphaMode alphaMode = ALPHA_OPAQUE;
			float alphaCutoff = 1.0f;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
			glm::vec4 baseColorFactor = glm::vec4(1.0f);
			Texture* baseColorTexture = nullptr;
			Texture* metallRoughnessTexture = nullptr;
			Texture* normalTexture = nullptr;
			Texture* occlusionTexture = nullptr;
			Texture* emissiveTexture = nullptr;

			Texture* specularGlossinessTexture;
			Texture* diffuseTexture;

			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

			Material(VulkanDevice* device) : device(device) {}
			void CreateDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags) {};

		};

		struct Mesh
		{
			std::vector<Vertex> m_vertexVec;
			std::vector<uint32_t> m_vertexIndexVec;
		};
		*/

		class GlTFModel
		{
		public:
			Vulkan::VulkanDevice* vulkanDevice;
			VkQueue copyQueue;

			struct Vertex
			{
				glm::vec3 pos;
				glm::vec3 normal;
				glm::vec2 uv;
				glm::vec3 color;
			};

			struct VertexBuffer
			{
				VkBuffer buffer;
				VkDeviceMemory memory;
			};

			struct IndexBuffer
			{
				int count;
				VkBuffer buffer;
				VkDeviceMemory memory;
			};

			struct Image
			{
				Vulkan::VulkanTexture2D texture;
				VkDescriptorSet descriptorSet;
			};

			struct Texture
			{
				int32_t imageIndex;
			};

			struct Material
			{
				glm::vec4 baseColorFactor = glm::vec4(1.0f);
				uint32_t  baseColorTextureIndex;
			};

			struct Primitive
			{
				uint32_t firstIndex;
				uint32_t indexCount;
				int32_t materialIndex;
			};

			struct Mesh
			{
				std::vector<Primitive> primitives;
			};

			struct Node
			{
				Node* parent;
				uint32_t index;
				std::vector<Node*> children;
				Mesh mesh;
				glm::vec3 translation{};
				glm::vec3 scale{ 1.0 };
				glm::quat rotation{};
				int32_t skin = -1;
				glm::mat4 matrix;
				glm::mat4 GetLocalMatrix();
			};

			std::vector<Image> images;
			std::vector<Texture> textures;
			std::vector<Material> materials;
			std::vector<Node*>    nodes;

			VertexBuffer vertices;
			IndexBuffer indices;

			void LoadImages(tinygltf::Model& input);
			void LoadTextures(tinygltf::Model& input);
			void LoadMaterials(tinygltf::Model& input);
			void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, 
				GlTFModel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<GlTFModel::Vertex>& vertexBuffer);


			void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
			void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, GlTFModel::Node* node);
		};


		class VkModel
		{
		public:
			Vulkan::VulkanDevice* vulkanDevice;
			VkQueue copyQueue;

			enum FileLoadingFlags {
				None = 0x00000000,
				PreTransformVertices = 0x00000001,
				PreMultiplyVertexColors = 0x00000002,
				FlipY = 0x00000004,
				DontLoadImages = 0x00000008
			};

			struct Vertex
			{
				glm::vec3 pos;
				glm::vec3 normal;
				glm::vec2 uv;
				glm::vec4 color;
				glm::vec4 joint0;
				glm::vec4 weight0;
				glm::vec4 tangent;

			};

			struct VertexBuffer
			{
				VkBuffer buffer;
				VkDeviceMemory memory;
			};

			struct IndexBuffer
			{
				int count;
				VkBuffer buffer;
				VkDeviceMemory memory;
			};

			struct Image
			{
				Vulkan::VulkanTexture2D texture;
				VkDescriptorSet descriptorSet;
			};

			struct Material
			{
				glm::vec4 baseColorFactor = glm::vec4(1.0f);
				uint32_t  baseColorTextureIndex;
			};

			struct Primitive
			{
				uint32_t firstIndex;
				uint32_t indexCount;
				uint32_t firstVertex;
				uint32_t vertexCount;
				int32_t materialIndex;

				struct Dimensions {
					glm::vec3 min = glm::vec3(FLT_MAX);
					glm::vec3 max = glm::vec3(-FLT_MAX);
					glm::vec3 size;
					glm::vec3 center;
					float radius;
				} dimensions;

				void setDimensions(glm::vec3 min, glm::vec3 max);
				Primitive(uint32_t firstIndex, uint32_t indexCount) : firstIndex(firstIndex), indexCount(indexCount) {};
			};

			struct Mesh
			{
				struct UniformBuffer
				{
					Render::Vulkan::Buffer buffer;
					VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
				};

				struct UniformBlock
				{
					glm::mat4 matrix;
					glm::mat4 jointMatrix[64]{};
					float jointcount{ 0 };
				};

				UniformBuffer uniformBuffer;
				UniformBlock uniformBlock;

				Render::Vulkan::VulkanDevice* device;

				std::vector<Primitive*> primitives;
				std::string name;

				Mesh(Render::Vulkan::VulkanDevice* device, glm::mat4 matrix);
				~Mesh();
			};

			struct Skin;

			struct Node
			{
				Node* parent;
				uint32_t index;
				std::vector<Node*> children;
				Mesh* mesh;
				std::string name;
				glm::vec3 translation{};
				glm::vec3 scale{ 1.0 };
				glm::quat rotation{};
				Skin* skin;
				glm::mat4 matrix;
				glm::mat4 LocalMatrix();//获取当前节点变化
				glm::mat4 GetMatrix(); // 获取当前节点到根的变化
				void Update();
				~Node();
			};

			struct Skin
			{
				std::string name;
				Node* skeletonRoot = nullptr;
				std::vector<glm::mat4> inverseBindMatrices;
				std::vector<Node*> joints;
			};

			std::vector<Image> images;
			std::vector<VulkanTexture> textures;
			std::vector<Material> materials;
			std::vector<Node*>    nodes;
			std::vector<Node*> linearNodes;

			struct Vertices {
				int count;
				VkBuffer buffer;
				VkDeviceMemory memory;
			} vertices;
			struct Indices {
				int count;
				VkBuffer buffer;
				VkDeviceMemory memory;
			} indices;
		public:

			bool buffersBound = false;

			VkModel() {};
			~VkModel();

			void LoadFromFile(std::string filename, Render::Vulkan::VulkanDevice* device, VkQueue transferQueue, uint32_t fileLoadingFlags = Render::Vulkan::VkModel::FileLoadingFlags::None, float scale = 1.0f);

			void LoadImages(tinygltf::Model& input);
			void LoadTextures(tinygltf::Model& input);
			void LoadMaterials(tinygltf::Model& input);
			void LoadNode(Render::Vulkan::VkModel::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex,
				const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);

			void Draw(VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
			void DrawNode(Node* node, VkCommandBuffer commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		};
	}
}