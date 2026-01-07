#include "Render/Vulkan/ApplicationBase.h"
#include <thread>
#include <queue>
#include <mutex>
#include <random>
#include <condition_variable>

struct ProjectModel
{
	Render::Vulkan::VkModel model;
	Render::Vulkan::VkModel startSphere;
};

struct Matrices
{
	glm::mat4 projection;
	glm::mat4 view;
};

struct Pipelines{
	VkPipeline phong{ VK_NULL_HANDLE };
	VkPipeline startsphere{ VK_NULL_HANDLE };
};

struct SecondaryCommandBuffers
{
	VkCommandBuffer background{ VK_NULL_HANDLE };
	VkCommandBuffer ui{ VK_NULL_HANDLE };
};

struct ThreadPushConstantBlock
{
	glm::mat4 mvp;
	glm::vec3 color;
};

struct ObjectData
{
	glm::mat4 model;
	glm::vec3 pos;
	glm::vec3 rotation;
	float rotationDir;
	float rotationSpeed;
	float scale;
	float deltaT;
	float stateT = 0;
	bool visible = true;
};

struct ThreadData
{
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::array<std::vector<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> commandBuffer;
	std::vector<ThreadPushConstantBlock> pushConstBlock;
	std::vector<ObjectData> objectData;
};

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Thread
{
private:
	bool destroying = false;
	std::thread worker;
	std::queue<std::function<void()>> jobQueue;
	std::mutex queueMutex;//加入队列的保护
	std::condition_variable condition;//执行的保护

	void QueueLoop()
	{
		while (true)
		{
			//wait
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(queueMutex);

				condition.wait(lock, [this] { return !jobQueue.empty() || destroying; });
				if (destroying) break;

				job = jobQueue.front();
			}

			job();//执行

			{
				std::lock_guard<std::mutex> lock(queueMutex);
				jobQueue.pop();
				condition.notify_one();//通知 上面的执行完了
			}
		}
	}

public:

	Thread()
	{
		worker = std::thread(&Thread::QueueLoop, this);//构造及执行
	}

	~Thread()
	{
		if (worker.joinable())
		{
			wait();
			queueMutex.lock();
			destroying = true;
			condition.notify_one();
			queueMutex.unlock();
			worker.join();
		}
	}

	void addJob(std::function<void()> function)
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		jobQueue.push(std::move(function));
		condition.notify_one();
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		condition.wait(lock, [this]() { return jobQueue.empty(); });
	}

};

class ThreadPool
{
public:
	std::vector<std::unique_ptr<Thread>> threads;

	void SetThreadCount(uint32_t count)
	{
		threads.clear();
		for (uint32_t i = 0; i < count; i++)
		{
			threads.push_back(make_unique<Thread>());
		}
	}

	void Wait()
	{
		for (auto& thread : threads)
		{
			thread->wait();
		}
	}
};

class Frustum
{
public:
	enum side {LEFT=0,RIGHT=1,TOP=2,BOTTOM=3,BACK=4,FRONT=5};
	std::array<glm::vec4, 6> planes;

	void Update(glm::mat4 matrix)
	{

	}

};

class ApplicationWin : public ApplicationBase
{
public:

	ProjectModel m_models;
	Matrices m_matrix;
	Pipelines m_pipelines;
	VkPipelineLayout m_pipelineLayout;

	std::array<SecondaryCommandBuffers, MAX_FRAMES_IN_FLIGHT> m_secondaryCommandBuffer;
	std::vector<ThreadData> m_threadData;
	int m_numThreads{ 0 };
	int m_numObjectPerThread{ 0 };


	ThreadPool m_threadPool;

	std::default_random_engine m_rndEngine;

	ApplicationWin();
	~ApplicationWin() override;

public:
	float Rnd(float range);

	void DrawUI(const VkCommandBuffer cmdBuffer);
	void CreateDescriptorPool();
	void UpdateUniformBuffers();

	void PrepareGraphicsPipeline();
	void PrepareUniformBuffer();
	void PrepareMulthreadRenderer();
	void BuildGraphicsCommandBuffer();

	void Prepare() override;
	void Render();
	void LoadAsset(std::string fileNamePath);
};

