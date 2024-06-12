#include "pch.hpp"
#include "Renderer.hpp"

#include "DescriptorAllocator.hpp"

bool Renderer::descriptorsInit()
{
    // create a descriptor pool that will hold 10 sets with 1 image each
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
        {
            {vk::DescriptorType::eStorageImage, 1}
        };

    gData.globalDescriptorAllocator.init(gData.device, 10, sizes);

    // make the descriptor set layout for our compute draw
    DescriptorLayoutBuilder builder;
    builder.addBinding(0, vk::DescriptorType::eStorageImage);
    auto setLayoutResult = builder.build(gData.device, vk::ShaderStageFlagBits::eCompute);
    if (!setLayoutResult.has_value())
    {
        gLogger->critical("Failed to create descriptor set layout for draw image !");
        return false;
    }
    gData.drawImageDescriptorLayout = setLayoutResult.value();
    

    //Allocate descriptor

    auto setResult = gData.globalDescriptorAllocator.allocate(gData.device, gData.drawImageDescriptorLayout);
    if(!setResult.has_value())
    {
        gLogger->critical("Failed to create allocate descriptor set for draw image !");
        return false;
    }
    gData.drawImageDescriptor = setResult.value();

    vk::DescriptorImageInfo imgInfo = {};
	imgInfo.imageLayout = vk::ImageLayout::eGeneral;
	imgInfo.imageView = gData.drawImage.imageView;

    vk::WriteDescriptorSet drawImageWrite = {};
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = gData.drawImageDescriptor;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = vk::DescriptorType::eStorageImage;
	drawImageWrite.pImageInfo = &imgInfo;

    gData.device.updateDescriptorSets({drawImageWrite}, {});

    //make sure both the descriptor allocator and the new layout get cleaned up properly
	gData.mainDeletionQueue.push_back([&]() {
		gData.globalDescriptorAllocator.cleanup(gData.device);
        gData.device.destroyDescriptorSetLayout(gData.drawImageDescriptorLayout);
    });


    return true;
}