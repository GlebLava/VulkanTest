#include "ComputePipeline.h"

ComputePipeline::ComputePipeline(vk::Device logical_device)
{
	// descriptor set for pipeline
	std::vector<vk::DescriptorSetLayoutBinding> layout_bindings(1);

	// A DescriptorSetLayoutBinding binds a descriptor set layout to a corresponding resource in the descriptor set
	vk::DescriptorSetLayoutBinding layout_binding;

	// Each DescriptorSetLayoutBinding is identified by a unique binding number
	// we only have one binding and identify this one with the number 0
	layout_binding.binding = 0;
	// The resources to be accessed by the layout is a storage image
	layout_binding.descriptorType = vk::DescriptorType::eStorageImage;
	// This binding only binds ONE descriptor set
	layout_binding.descriptorCount = 1;
	// this binding will be used in the compute stage
	layout_binding.stageFlags = vk::ShaderStageFlagBits::eCompute;

	layout_bindings[0] = layout_binding;

	vk::DescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.flags = vk::DescriptorSetLayoutCreateFlagBits();
	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = layout_bindings.data();

	vk::DescriptorSetLayout layout = logical_device.createDescriptorSetLayout(layout_create_info);


	//make shader module
	auto sourceCode = readFile("shaders/raytracer.spv");
	vk::ShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.flags = vk::ShaderModuleCreateFlags();
	moduleCreateInfo.codeSize = sourceCode.size();
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

	vk::ShaderModule shader_module = logical_device.createShaderModule(moduleCreateInfo);

	//Pipeline layout
	vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};
	shaderStageCreateInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
	shaderStageCreateInfo.module = shader_module;
	shaderStageCreateInfo.pName = "main";

	vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.stage = shaderStageCreateInfo;

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlags();
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &layout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

	vk::PipelineLayout compute_pipeline_layout = logical_device.createPipelineLayout(pipelineLayoutCreateInfo);

	computePipelineCreateInfo.layout = compute_pipeline_layout;

	// Create pipeline
	compute_pipeline = logical_device.createComputePipeline(nullptr, computePipelineCreateInfo).value;

	// destroy shaders
	logical_device.destroyShaderModule(shader_module);

	// Attachement
	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.flags = vk::PipelineColorBlendStateCreateFlags();
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;
}