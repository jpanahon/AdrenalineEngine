#include "renderpass.h"
#include "tools.h"
#include "info.h"

void Adren::Renderpass::create(Image& depth, VkFormat& imageFormat, VkInstance& instance) {
    VkAttachmentDescription colorAttachment = Adren::Info::colorAttachment(imageFormat);
    VkAttachmentDescription depthAttachment = Adren::Info::depthAttachment(depth.format);


    VkAttachmentReference colorReference{};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    
    VkAttachmentReference depthReference{};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pDepthStencilAttachment = &depthReference;

    VkSubpassDependency dependency = Adren::Info::dependency();

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    Adren::Tools::vibeCheck("RENDER PASS", vkCreateRenderPass(device, &renderPassInfo, nullptr, &handle));
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)handle, "MAIN RENDER PASS");
}

void Adren::Renderpass::begin(VkCommandBuffer& commandBuffer, uint32_t& index, std::vector<VkFramebuffer>& framebuffers, VkExtent2D& extent) {
    VkRenderPassBeginInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = handle;
    renderpassInfo.framebuffer = framebuffers[index];
    renderpassInfo.renderArea.offset = { 0, 0 };
    renderpassInfo.renderArea.extent = extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.119f, 0.181f, 0.254f, 0.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderpassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
}