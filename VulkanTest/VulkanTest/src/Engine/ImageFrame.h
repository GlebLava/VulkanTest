#pragma once

#include <vulkan/vulkan.hpp>

class ImageFrame
{
public:
	ImageFrame(vk::Device logical_device, vk::Image image, vk::Format format, vk::Extent2D extent);

private:
	vk::Device logical_device;
	vk::ImageView image_view;
	vk::Format format;
	vk::Extent2D extent;
};