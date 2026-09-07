// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/vulkan/pipeline.hpp"

#include <stdexcept>

namespace lucuma::vulkan::vk {

static void check(VkResult r, const char* what) {
	if (r != VK_SUCCESS)
		throw std::runtime_error(std::string("vulkan: ") + what + " -> " +
		                         resultString(r));
}

ComputePipeline::ComputePipeline(Context& ctx, std::span<const uint32_t> spirv,
                                 int nBuffers, uint32_t pushBytes,
                                 const char* entry)
    : ctx_(ctx), nBuffers_(nBuffers) {
	VkDevice dev = ctx_.device();
	module_ = ctx_.loadShader(spirv);

	std::vector<VkDescriptorSetLayoutBinding> binds(nBuffers);
	for (int i = 0; i < nBuffers; ++i) {
		binds[i].binding = static_cast<uint32_t>(i);
		binds[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		binds[i].descriptorCount = 1;
		binds[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	}
	VkDescriptorSetLayoutCreateInfo slci{};
	slci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	slci.bindingCount = static_cast<uint32_t>(binds.size());
	slci.pBindings = binds.data();
	check(vkCreateDescriptorSetLayout(dev, &slci, nullptr, &setLayout_),
	      "vkCreateDescriptorSetLayout");

	VkPushConstantRange pcr{};
	pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	pcr.offset = 0;
	pcr.size = pushBytes;
	VkPipelineLayoutCreateInfo plci{};
	plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plci.setLayoutCount = 1;
	plci.pSetLayouts = &setLayout_;
	plci.pushConstantRangeCount = pushBytes ? 1 : 0;
	plci.pPushConstantRanges = pushBytes ? &pcr : nullptr;
	check(vkCreatePipelineLayout(dev, &plci, nullptr, &layout_),
	      "vkCreatePipelineLayout");

	VkPipelineShaderStageCreateInfo stage{};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage.module = module_;
	stage.pName = entry;
	VkComputePipelineCreateInfo cpci{};
	cpci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	cpci.stage = stage;
	cpci.layout = layout_;
	check(vkCreateComputePipelines(dev, VK_NULL_HANDLE, 1, &cpci, nullptr,
	                               &pipeline_),
	      "vkCreateComputePipelines");

	VkDescriptorPoolSize ps{};
	ps.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ps.descriptorCount = static_cast<uint32_t>(nBuffers);
	VkDescriptorPoolCreateInfo dpci{};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.maxSets = 1;
	dpci.poolSizeCount = 1;
	dpci.pPoolSizes = &ps;
	check(vkCreateDescriptorPool(dev, &dpci, nullptr, &pool_),
	      "vkCreateDescriptorPool");

	VkDescriptorSetAllocateInfo dsai{};
	dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsai.descriptorPool = pool_;
	dsai.descriptorSetCount = 1;
	dsai.pSetLayouts = &setLayout_;
	check(vkAllocateDescriptorSets(dev, &dsai, &set_),
	      "vkAllocateDescriptorSets");
}

ComputePipeline::~ComputePipeline() {
	VkDevice dev = ctx_.device();
	if (pool_) vkDestroyDescriptorPool(dev, pool_, nullptr);
	if (pipeline_) vkDestroyPipeline(dev, pipeline_, nullptr);
	if (layout_) vkDestroyPipelineLayout(dev, layout_, nullptr);
	if (setLayout_) vkDestroyDescriptorSetLayout(dev, setLayout_, nullptr);
	if (module_) vkDestroyShaderModule(dev, module_, nullptr);
}

void ComputePipeline::bindBuffers(std::span<const VkBuffer> buffers) {
	if (static_cast<int>(buffers.size()) != nBuffers_)
		throw std::runtime_error("ComputePipeline::bindBuffers count mismatch");
	std::vector<VkDescriptorBufferInfo> infos(buffers.size());
	std::vector<VkWriteDescriptorSet> writes(buffers.size());
	for (size_t i = 0; i < buffers.size(); ++i) {
		infos[i].buffer = buffers[i];
		infos[i].offset = 0;
		infos[i].range = VK_WHOLE_SIZE;
		writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[i].dstSet = set_;
		writes[i].dstBinding = static_cast<uint32_t>(i);
		writes[i].descriptorCount = 1;
		writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writes[i].pBufferInfo = &infos[i];
	}
	vkUpdateDescriptorSets(ctx_.device(),
	                       static_cast<uint32_t>(writes.size()), writes.data(),
	                       0, nullptr);
}

void ComputePipeline::dispatch(VkCommandBuffer cb, const void* push,
                               uint32_t pushBytes, uint32_t gx, uint32_t gy,
                               uint32_t gz) {
	vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
	vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, layout_, 0, 1,
	                        &set_, 0, nullptr);
	if (pushBytes)
		vkCmdPushConstants(cb, layout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
		                   pushBytes, push);
	vkCmdDispatch(cb, gx, gy, gz);
}

}  // namespace lucuma::vulkan::vk
