#pragma once
#include <vulkan/vulkan.hpp>
#include <fstream>

class ComputePipeline
{
public:
	ComputePipeline() {};
	ComputePipeline(vk::Device logical_device);

private:
	vk::Pipeline compute_pipeline;




	inline std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
};