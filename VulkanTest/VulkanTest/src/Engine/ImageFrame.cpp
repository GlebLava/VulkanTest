#include "ImageFrame.h"

ImageFrame::ImageFrame(vk::Device logical_device, vk::Image image, vk::Format format, vk::Extent2D extent)
{
	this->logical_device = logical_device;
	this->format = format;
	this->extent = extent;

	vk::ImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
	imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlags{ 1 };
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	this->image_view = logical_device.createImageView(imageViewCreateInfo);
}