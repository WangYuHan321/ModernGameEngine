#pragma once
#include "Base/Asset.h"
#include <filesystem>

class AssetPipeline
{
    // 系统初始化与销毁
    bool Initialize(const std::filesystem::path& configDirectory);  // 读取所有配置，初始化子系统
    void Shutdown();  // 优雅关闭，保存状态，清理资源

    void Execute();

    // 子系统管理
    void InitializeDefaultPipeline();  // 创建默认处理管道
    void Cleanup();  // 资源清理
};