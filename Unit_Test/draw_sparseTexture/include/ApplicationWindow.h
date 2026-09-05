#include "Render/Vulkan/ApplicationBase.h"


/*
 * Important note : This sample is work-in-progress and works basically, but it's not finished
 *
 * =============================================================================
 * 为什么这个例子要拆这么多层结构？（新手必读）
 * =============================================================================
 *
 * 普通纹理：vkCreateImage + vkAllocateMemory + vkBindImageMemory
 *   → 整张图一次性占满显存。4096×4096 全 mip 大约几十 MB，开销还能接受；
 *     但游戏里 16k / 32k 虚拟贴图如果也整张常驻，显存立刻爆掉。
 *
 * Sparse residency（稀疏驻留）像操作系统的虚拟内存：
 *   1. 先创建一张“逻辑上很大”的 VkImage，但此时几乎不占显存。
 *   2. 把图像按 GPU 规定的粒度切成很多 page（页 / 图块）。
 *   3. 只给当前真正用到的 page 分配 VkDeviceMemory，再通过
 *      vkQueueBindSparse 把这块内存“贴”到图像的对应区域。
 *   4. 不需要时把绑定解开、把内存释放。着色器读到未驻留区域会走
 *      sparseResidency 的 fallback（本例用随机色块演示）。
 *
 * Vulkan 的 sparse 绑定 API 本身就是层层嵌套的，所以本文件也必须
 * 有对应的结构，并不是重复造轮子：
 *
 *   vkQueueBindSparse(queue, 1, &bindSparseInfo, fence)
 *     └── VkBindSparseInfo                         一次提交的总包裹
 *           ├── pImageBinds
 *           │     └── VkSparseImageMemoryBindInfo  “可按页拆开绑定”的那部分
 *           │           └── pBinds[] = VkSparseImageMemoryBind
 *           │                 （每一页：绑哪块区域、用哪块内存）
 *           └── pImageOpaqueBinds
 *                 └── VkSparseImageOpaqueMemoryBindInfo  mip tail（不能按页拆）
 *                       └── pBinds[] = VkSparseMemoryBind
 *
 * 本例对应关系：
 *   VirtualTexturePage  = 我们自己的“一页”：位置 + 显存 + 是否已驻留
 *   VirtualTexture      = 整张虚拟图 + 上面那一整套 Vulkan 绑定结构
 *   SparseTexture       = VirtualTexture + sampler / imageView / descriptor
 *                         （普通纹理采样也需要这些，和 sparse 无关）
 *
 * 另外还有 mip tail：很小的那几级 mip（比如 4×4、2×2、1×1）往往小于
 * GPU 的 page 粒度，无法单独驻留，驱动会把它们捆成一块不透明内存
 * （opaque bind）。所以 image bind 和 opaque bind 两套结构都要有。
 */

// 虚拟纹理的一页（tile / page）
//
// 稀疏图创建后只是一个“地址空间”，这一页是其中一块矩形区域。
// 有内存绑定 = resident（驻留，着色器能读到像素）；
// memory == VK_NULL_HANDLE = 空洞（未驻留）。
struct VirtualTexturePage
{
	// 这一页在所属 mip 图像里的左上角（像素坐标），对应 VkSparseImageMemoryBind::offset
	VkOffset3D offset;
	// 这一页的宽高深（像素）。内部页一般等于 GPU 的 imageGranularity；
	// 贴图边缘那一圈可能更小（例如 4096 不能被粒度整除时）
	VkExtent3D extent;
	// 真正交给 vkQueueBindSparse 的那条“把某块显存贴到图像某区域”的描述。
	// imageMemoryBind.memory != VK_NULL_HANDLE 就表示这一页已经驻留。
	VkSparseImageMemoryBind imageMemoryBind;
	// 为这一页分配显存时要申请多少字节。通常等于整张稀疏图的 memory alignment
	// （Vulkan 要求每页分配大小对齐到这个值），不一定等于 width*height*4
	VkDeviceSize size;
	// 属于哪一级 mip。mip 0 是原图，数字越大图越小
	uint32_t mipLevel;
	// 属于哪一层 array layer。本例 layerCount = 1，所以一般是 0
	uint32_t layer;
	// 在 VirtualTexture::pages 里的下标，调试 / 统计用
	uint32_t index;
	// 标记“这一页即将解绑”。flushRandomPages 会先用 del=true 做一次
	// bind（memory = NULL，把页从图像上揭下来），等 GPU 完成后再真正 vkFreeMemory
	bool del;

	VirtualTexturePage();
	// 是否已分配并绑定了显存
	bool resident();
	// 给这一页 vkAllocateMemory，并填好 imageMemoryBind（还不提交到队列）
	bool allocate(VkDevice device, uint32_t memoryTypeIndex);
	// vkFreeMemory，并把 imageMemoryBind.memory 置空
	bool release(VkDevice device);
};

struct VirtualTexture
{
	VkDevice device;	
	VkImage image;
	VkBindSparseInfo bindSparseInfo;
	std::vector<VirtualTexturePage> pages;
	std::vector<VkSparseMemoryBind> sparseMemoryBinds;



};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:
	bool m_wireFrame = false;

	GlTFModel m_glTFModel;
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	Pipelines m_pipelines;

	UniformData m_uniform;
	DescriptorSetLayouts m_descriptorSetLayout;

	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSet;
	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformDataBuffer;

public:
	void DrawUI(const VkCommandBuffer cmdBuffer);
	void CreateDescriptorPool();
	void UpdateUniformBuffers();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void PreparePipeline();
	void PrepareUniformBuffer();
	void BuildCommandBuffer();

	virtual void GetEnabledFeatures();
	virtual void OnUpdateUIOverlay(Render::Vulkan::UIOverlay* overlay);

	void Prepare() override;
	void Render();
	void LoadAsset();
};

