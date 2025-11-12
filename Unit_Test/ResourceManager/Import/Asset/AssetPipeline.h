#pragma once
#include "Base/Asset.h"

class AssetPipeline
{
    // 系统初始化与销毁
    bool initialize(const std::filesystem::path& configDirectory);  // 读取所有配置，初始化子系统
    void shutdown();  // 优雅关闭，保存状态，清理资源

    void Execute();

    // 子系统管理
    void initializeDefaultPipeline();  // 创建默认处理管道
    void cleanup();  // 资源清理
};