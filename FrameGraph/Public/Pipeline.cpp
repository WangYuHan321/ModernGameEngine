#include "pipeline.h"

bool FrameGraph::PipelineDescription::Texture::operator==(const Texture& rhs) const
{
	return (state == rhs.state) && (textureType == rhs.textureType);
}

GND bool FrameGraph::PipelineDescription::Sampler::operator==(const Sampler& rhs) const
{
	return GND bool();
}

GND bool FrameGraph::PipelineDescription::SubpassInput::operator==(const SubpassInput& rhs) const
{
	return GND bool();
}

GND bool FrameGraph::PipelineDescription::StorageBuffer::operator==(const StorageBuffer& rhs) const
{
	return GND bool();
}

GND bool FrameGraph::PipelineDescription::RayTracingScene::operator==(const RayTracingScene& rhs) const
{
	return GND bool();
}

FrameGraph::PipelineDescription::_TextureUniform::_TextureUniform(const Local::UniformID& id, EImageSampler textureType, const BindingIndex& index, uint arraySize, EShaderStage stageFlags)
{
}

FrameGraph::PipelineDescription::_SamplerUniform::_SamplerUniform(const Local::UniformID& id, const BindingIndex& index, uint arraySize, EShaderStages stageFlags)
{
}

FrameGraph::PipelineDescription::_SubpassInputUniform::_SubpassInputUniform(const Local::UniformID& id, uint attachmentIndex, bool isMultisample, const BindingIndex& index, uint arraySize, EShaderStage stageFlags)
{
}

FrameGraph::PipelineDescription::_ImageUniform::_ImageUniform(const Local::UniformID& id, EImageSampler imageType, EShaderAccess access, const BindingIndex& index, uint arraySize, EShaderStage stageFlags)
{
}

FrameGraph::PipelineDescription::_UBufferUniform::_UBufferUniform(const Local::UniformID& id, BytesU size, const BindingIndex& index, uint arraySize, EShaderStage stageFlags, uint dynamicOffsetIndex)
{
}

FrameGraph::PipelineDescription::_StorageBufferUniform::_StorageBufferUniform(const Local::UniformID& id, BytesU staticSize, BytesU arrayStride, EShaderAccess access, const BindingIndex& index, uint arraySize, EShaderStage stageFlags, uint dynamicOffsetIndex)
{
}

FrameGraph::PipelineDescription::_RayTracingSceneUniform::_RayTracingSceneUniform(const Local::UniformID& id, const BindingIndex& index, uint arraySize, EShaderStage stageFlags)
{
}

GND bool FrameGraph::PipelineDescription::Uniform::operator==(const Uniform& rhs) const
{
	return GND bool();
}

GND bool FrameGraph::GraphicsPipelineDesc::FragmentOutput::operator==(const FragmentOutput& rhs) const
{
	return GND bool();
}

void FrameGraph::PipelineDescription::Shader::AddShaderData(EShaderLangFormat fmt, StringView entry, String&& src, StringView dbgName)
{
}

void FrameGraph::PipelineDescription::Shader::AddShaderData(EShaderLangFormat fmt, StringView entry, Array<uint>&& bin, StringView dbgName)
{
}

void FrameGraph::PipelineDescription::_AddDescriptorSet(const Local::DescriptorSetID& id, uint index, ArrayView<_TextureUniform> textures, ArrayView<_SamplerUniform> samplers, ArrayView<_SubpassInputUniform> subpassInputs, ArrayView<_ImageUniform> images, ArrayView<_UBufferUniform> uniformBuffers, ArrayView<_StorageBufferUniform> storageBuffers, ArrayView<_RayTracingSceneUniform> rtScenes)
{
}

void FrameGraph::PipelineDescription::_SetPushConstants(ArrayView<_PushConstant> values)
{
}
