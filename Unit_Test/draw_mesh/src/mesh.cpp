#include "Vulkan.h"
#include "glm/glm.hpp"

#include <array>

/*
 
 struct Vertex {
 glm::vec3 pos;
 glm::vec3 normal;
 glm::vec3 color;
 glm::vec2 texCoord;
 
 static VkVertexInputBindingDescription getBindDescription()
 {
 VkVertexInputBindingDescription bindingDescription{};
 bindingDescription.binding = 0;
 bindingDescription.stride =  sizeof(Vertex);
 bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
 
 return bindingDescription;
 }
 
 static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions(){
 std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
 
 attributeDescriptions[0].binding = 0;
 attributeDescriptions[0].location = 0;
 attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
 attributeDescriptions[0].location = offsetof(Vertex, pos);
 
 attributeDescriptions[1].binding = 0;
 attributeDescriptions[1].location = 1;
 attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
 attributeDescriptions[1].location = offsetof(Vertex, normal);
 
 attributeDescriptions[2].binding = 0;
 attributeDescriptions[2].location = 2;
 attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
 attributeDescriptions[2].location = offsetof(Vertex, color);
 
 attributeDescriptions[3].binding = 0;
 attributeDescriptions[3].location = 3;
 attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
 attributeDescriptions[3].location = offsetof(Vertex, texCoord);
 }
 
 bool operator==(const Vertex& other) const{
 return pos == other.pos && color == other.color && texCoord == other.texCoord;
 }
 };
 
 struct Material
 {
 std::vector<VkImage> TextureImages;
 std::vector<VkDeviceMemory> TextureImagesMemorys;
 std::vector<VkImageView> TextureImageViews;
 std::vector<VkSampler> TextureSamplers;
 
 VkDescriptorPool DescriptorPool;
 std::vector<VkDescriptorSet> Descriptions;
 };
 
 
 struct Mesh
 {
 std::vector<Vertex> Vertices;
 std::vector<uint32_t> Indices;
 VkBuffer VertexBuffer;
 VkDeviceMemory VertexBufferMemory;
 VkBuffer IndexBuffer;
 VkDeviceMemory IndexBufferMemory;
 
 };
 
 */
