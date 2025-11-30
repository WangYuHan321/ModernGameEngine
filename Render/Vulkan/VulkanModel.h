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
#include <tiny_gltf.h>

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

		};
	}
}