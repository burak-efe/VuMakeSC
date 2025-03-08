#include "VuRenderer.h"

#include "VuConfig.h"

namespace Vu {
    bool VuRenderer::shouldWindowClose() {
        return false;//ctx::sdlEvent.type == SDL_EVENT_QUIT;
    }

    void VuRenderer::waitIdle() {
        vkDeviceWaitIdle(ctx::vuDevice->device);
    }

    void VuRenderer::beginFrame() {
        //SDL_PollEvent(&ctx::sdlEvent);
        waitForFences();
        VkResult result = vkAcquireNextImageKHR(
            ctx::vuDevice->device, swapChain.swapChain, UINT64_MAX,
            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentFrameImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            //resetSwapChain();
            std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(ctx::vuDevice->device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        beginRecordCommandBuffer(commandBuffers[currentFrame], currentFrameImageIndex);
    }

    void VuRenderer::waitForFences() {
        vkWaitForFences(ctx::vuDevice->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    }

    void VuRenderer::beginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        swapChain.beginRenderPass(commandBuffer, imageIndex);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = (float) swapChain.swapChainExtent.height;
        viewport.width    = (float) swapChain.swapChainExtent.width;
        viewport.height   = -(float) swapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChain.swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        bindGlobalBindlessSet(commandBuffer);
    }

    void VuRenderer::endRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex) {
        swapChain.endRenderPass(commandBuffer);
        VkCheck(vkEndCommandBuffer(commandBuffer));

    }

    void VuRenderer::bindMesh(VuMesh& mesh) {
        //we are using vertex pulling, so only index buffers we need to bind
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.get()->buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VuRenderer::bindMaterial(const VuMaterial& material) {
        auto commandBuffer = commandBuffers[currentFrame];
        material.bindPipeline(commandBuffer);
    }

    void VuRenderer::drawIndexed(uint32 indexCount) {
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    void VuRenderer::pushConstants(const GPU_PushConstant& pushConstant) {
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdPushConstants(commandBuffer, ctx::vuDevice->globalPipelineLayout, VK_SHADER_STAGE_ALL, 0, config::PUSH_CONST_SIZE,
                           &pushConstant);
    }

    // void VuRenderer::beginImgui() {
    //     ImGui_ImplVulkan_NewFrame();
    //     ImGui_ImplSDL3_NewFrame();
    //     ImGui_ImplSDL3_ProcessEvent(&ctx::sdlEvent);
    //     ImGui::NewFrame();
    // }

    // void VuRenderer::endImgui() {
    //     auto commandBuffer = commandBuffers[currentFrame];
    //
    //     ImGui::Render();
    //     ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    // }

    void VuRenderer::endFrame() {
        endRecordCommandBuffer(commandBuffers[currentFrame], currentFrameImageIndex);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore          waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount         = 1;
        submitInfo.pWaitSemaphores            = waitSemaphores;
        submitInfo.pWaitDstStageMask          = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VkCheck(vkQueueSubmit(ctx::vuDevice->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.swapChain};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;

        presentInfo.pImageIndices = &currentFrameImageIndex;

        auto result = vkQueuePresentKHR(ctx::vuDevice->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            //resetSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
    }


    void VuRenderer::updateFrameConstantBuffer(GPU_FrameConst ubo) {
        uniformBuffers[currentFrame].setData(&ubo, sizeof(ubo));
    }


    // void VuRenderer::resetSwapChain() {
    //     SDL_Event event;
    //
    //     int width  = 0;
    //     int height = 0;
    //     SDL_GetWindowSize(ctx::window, &width, &height);
    //     auto minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
    //
    //     while (width <= 0 || height <= 0 || minimized) {
    //         SDL_GetWindowSize(ctx::window, &width, &height);
    //         minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
    //         SDL_WaitEvent(&event);
    //     }
    //     vkDeviceWaitIdle(ctx::vuDevice->device);
    //     swapChain.resetSwapChain(surface);
    // }
}
