/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <memory>

#include "coreSettings.h"
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"
#include "events/keyEvent.h"

#include "VKwindow.h"

bool VK_Window::m_GLFWIsInitialized = false;

VK_Window::VK_Window(const WindowProperties& props)
    : m_OK(false), m_IsFullscreen(false),
      m_AllowCursor(false)
{
    m_WindowProperties.m_Title    = props.m_Title;
    m_WindowProperties.m_Width    = props.m_Width;
    m_WindowProperties.m_Height   = props.m_Height;
    //m_WindowProperties.m_VSync    = props.m_VSync;
    m_WindowProperties.m_MousePosX= 0.0f;
    m_WindowProperties.m_MousePosY= 0.0f;
    m_WindowProperties.m_FramebufferResized = false;

    if (!m_GLFWIsInitialized)
    {
        // init glfw
        m_GLFWIsInitialized = InitGLFW();
    }
    if (m_GLFWIsInitialized)
    {

        CreateWindow();

        uint extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        LOG_CORE_INFO("{0}  extensions supported", extensionCount);

        // create a device
        m_Device = std::make_shared<VK_Device>(this);

        CreatePipelineLayout();
        RecreateSwapChain();
        CreateCommandBuffers();

        
        if (m_Window && m_SwapChain && m_Pipeline)
        {
            m_OK = true;
        }
        else
        {
            LOG_APP_WARN("Houston, we have problem: (m_Window && m_SwapChain && m_Pipeline) failed");
        }
    }

    //    // set app icon
    //    GLFWimage icon;
    //    size_t fileSize;
    //    const uchar* data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/images/I_ENGINE.png", IDB_ENGINE_LOGO, "PNG");
    //    icon.pixels = stbi_load_from_memory(data, fileSize, &icon.width, &icon.height, 0, 4); //rgba channels
    //    if (icon.pixels) 
    //    {
    //        glfwSetWindowIcon(m_Window, 1, &icon); 
    //        stbi_image_free(icon.pixels);
    //    }
    //    else
    //    {
    //        LOG_CORE_WARN("Could not load app icon");
    //    }
    //

}

VK_Window::~VK_Window()
{
    Shutdown();
}

void VK_Window::Shutdown()
{
    vkDestroyPipelineLayout(m_Device->Device(), m_PipelineLayout, nullptr);
    
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

//void VK_Window::SetVSync(int interval)
//{ 
//    m_WindowProperties.m_VSync = interval;
//    m_GraphicsContext->SetVSync(interval);
//}
//
void VK_Window::ToggleFullscreen()
{ 
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    if (m_IsFullscreen)
    {
        m_WindowProperties.m_Width  = m_WindowedWidth;
        m_WindowProperties.m_Height = m_WindowedHeight;

        glfwSetWindowMonitor(m_Window, nullptr, m_WindowPositionX, m_WindowPositionY, m_WindowedWidth, m_WindowedHeight, videoMode->refreshRate);
        glfwSetWindowPos(m_Window, m_WindowPositionX, m_WindowPositionY);
    }
    else
    {
        m_WindowedWidth = m_WindowProperties.m_Width; 
        m_WindowedHeight = m_WindowProperties.m_Height;
        glfwGetWindowPos(m_Window, &m_WindowPositionX, &m_WindowPositionY);

        glfwSetWindowMonitor(m_Window, monitors[0], 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
    }
    m_IsFullscreen = !m_IsFullscreen;
}

//void VK_Window::SetWindowAspectRatio()
//{
//    // set aspect ratio to current ratio
//    int numer = m_WindowProperties.m_Width;
//    int denom = m_WindowProperties.m_Height;
//    glfwSetWindowAspectRatio(m_Window, numer, denom);
//}
//
//void VK_Window::SetWindowAspectRatio(int numer, int denom)
//{
//    glfwSetWindowAspectRatio(m_Window, numer, denom);
//}

void VK_Window::OnUpdate()
{
    if (glfwWindowShouldClose(m_Window))
    {
        m_OK = false;
    }
    else
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(m_Device->Device());
}

void VK_Window::OnError(int errorCode, const char* description) 
{
        LOG_CORE_CRITICAL("GLEW error, code: {0}, description: {1}", std::to_string(errorCode), description);

}

void VK_Window::SetEventCallback(const EventCallbackFunction& callback)
{
    m_WindowProperties.m_EventCallback = callback;
    glfwSetWindowUserPointer(m_Window,&m_WindowProperties);
    
    glfwSetErrorCallback([](int errorCode, const char* description) { VK_Window::OnError(errorCode, description);});
    
    glfwSetKeyCallback(m_Window,[](GLFWwindow* window, int key, int scancode, int action, int modes)
        {
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
            
            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key);
                    OnEvent(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    OnEvent(event);
                    break;
                }
                case GLFW_REPEAT:
                    break;
            }
        }
    );
    
    //glfwSetWindowCloseCallback(m_Window,[](GLFWwindow* window)
    //    {
    //        WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
    //        EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
    //            
    //        WindowCloseEvent event;
    //        OnEvent(event);
    //    }
    //);

    glfwSetFramebufferSizeCallback(m_Window,[](GLFWwindow* window, int width, int height)
        {
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;

            windowProperties.m_Width = width;
            windowProperties.m_Height = height;
            windowProperties.m_FramebufferResized = true;

            WindowResizeEvent event(width, height);
            OnEvent(event);
        }
    );

    //glfwSetWindowSizeCallback(m_Window,[](GLFWwindow* window, int width, int height)
    //    {
    //        WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
    //        EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
    //
    //        windowProperties.m_Width = width;
    //        windowProperties.m_Height = height;
    //        windowProperties.m_FramebufferResized = true;
    //
    //        WindowResizeEvent event(width, height);
    //        OnEvent(event);
    //    }
    //);

    
    glfwSetWindowIconifyCallback(m_Window,[](GLFWwindow* window, int iconified)
        {
            int width, height;
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
            
            if (iconified)
            {
                width = height = 0;
            }
            else
            {
                glfwGetWindowSize(window, &width, &height);
            }
            
            windowProperties.m_Width = width;
            windowProperties.m_Height = height;
                
            WindowResizeEvent event(width, height);
            OnEvent(event);
        }
    );

    glfwSetMouseButtonCallback(m_Window,[](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
            
            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button,windowProperties.m_MousePosX,windowProperties.m_MousePosY);
                    OnEvent(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    OnEvent(event);
                    break;
                }
            }
        }
    );
    
    glfwSetCursorPosCallback(m_Window,[](GLFWwindow* window, double xpos, double ypos)
        {
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
            
            windowProperties.m_MousePosX = xpos;
            windowProperties.m_MousePosY = ypos;
                        
            MouseMovedEvent event(xpos, ypos);
            OnEvent(event);
            
        }
    );
    
    glfwSetScrollCallback(m_Window,[](GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
                        
            MouseScrolledEvent event(xoffset, yoffset);
            OnEvent(event);
            
        }
    );
}

bool VK_Window::InitGLFW()
{
    
    // init glfw
    if (!glfwInit())
    {
        LOG_CORE_CRITICAL("glfwInit() failed");
        return false;
    }

    return true;
}

void VK_Window::EnableMousePointer()
{
    if (m_AllowCursor)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void VK_Window::DisableMousePointer()
{
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void VK_Window::AllowCursor()
{
    m_AllowCursor = true;
}
void VK_Window::DisallowCursor()
{
    m_AllowCursor = false;
    DisableMousePointer();
}

void VK_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("Could not create surface");
    }
}

void VK_Window::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(VK_SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to create pipeline layout!");
    }
}
void VK_Window::CreatePipeline()
{
    ASSERT(m_SwapChain != nullptr);
    ASSERT(m_PipelineLayout != nullptr);

    PipelineConfigInfo pipelineConfig{};

    VK_Pipeline::DefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_PipelineLayout;

    // create a pipeline
    m_Pipeline = std::make_unique<VK_Pipeline>
    (
        m_Device,
        "bin/simpleShader.vert.spv",
        "bin/simpleShader.frag.spv",
        pipelineConfig
    );
}

void VK_Window::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapChain->ImageCount());
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = m_Device->GetCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint>(m_CommandBuffers.size());

    if (vkAllocateCommandBuffers(m_Device->Device(), &allocateInfo, m_CommandBuffers.data()) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to allocate command buffers");
    }
}

void VK_Window::FreeCommandBuffers()
{
    vkFreeCommandBuffers
    (
        m_Device->Device(),
        m_Device->GetCommandPool(),
        static_cast<uint>(m_CommandBuffers.size()),
        m_CommandBuffers.data()
    );
    m_CommandBuffers.clear();
}

void VK_Window::RecordCommandBuffer(int imageIndex)
{
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to begin recording command buffer");
    }
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(imageIndex);
    
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
    vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);

    RenderEntities(m_CommandBuffers[imageIndex]);

    vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
    if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("recording of command buffer failed");
    }
}

void VK_Window::RenderEntities(VkCommandBuffer commandBuffer)
{
    m_Pipeline->Bind(commandBuffer);
    for (auto& entity : m_Entities[0])
    {
        VK_SimplePushConstantData push{};
        push.m_Offset = entity.m_Transform2D.m_Translation;
        push.m_Color  = entity.m_Color;
        push.m_Transform = entity.m_Transform2D.Mat2();
        vkCmdPushConstants(
            commandBuffer,
            m_PipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(VK_SimplePushConstantData),
            &push);
        static_cast<VK_Model*>(entity.m_Model.get())->Bind(commandBuffer);
        static_cast<VK_Model*>(entity.m_Model.get())->Draw(commandBuffer);
    }
}

void VK_Window::RecreateSwapChain()
{
    auto extent = GetExtend();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = GetExtend();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_Device->Device());

    // create the swapchain and pipeline
    
    if (m_SwapChain == nullptr)
    {
        m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent);
    }
    else
    {
        m_SwapChain = std::make_unique<VK_SwapChain>(m_Device, extent, std::move(m_SwapChain));
        if (m_SwapChain->ImageCount() != m_CommandBuffers.size())
        {
            FreeCommandBuffers();
            CreateCommandBuffers();
        }
    }
    CreatePipeline();
}

void VK_Window::DrawFrame()
{
    uint imageIndex = 0;
    auto result = m_SwapChain->AcquireNextImage(&imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_CORE_CRITICAL("failed to acquire next swap chain image");
    }

    RecordCommandBuffer(imageIndex);
    result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || WasResized())
    {
        ResetWindowResizedFlag();
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to present swap chain image");
    }
}

std::shared_ptr<Model> VK_Window::LoadModel(std::vector<Vertex>& vertices)
{
    ASSERT(m_Device != nullptr);
    auto model = std::make_shared<VK_Model>(m_Device, vertices);
    return std::move(model);
}
void VK_Window::CreateWindow()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    m_RefreshRate = videoMode->refreshRate;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // make window invisible before it gets created
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    int monitorX, monitorY;
    glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);
    
    m_WindowedWidth = videoMode->width / 2.5f;
    m_WindowedHeight = m_WindowedWidth;// / 16 * 9;
    m_WindowPositionX = (videoMode->width - m_WindowedWidth) / 2;
    m_WindowPositionY = (videoMode->height - m_WindowedHeight) / 2;

    if (CoreSettings::m_EnableFullscreen)
    {
        #ifdef _WIN32    
            m_WindowProperties.m_Width = videoMode->width;
            m_WindowProperties.m_Height = videoMode->height;
            m_Window = glfwCreateWindow(
                m_WindowProperties.m_Width,
                m_WindowProperties.m_Height,
                m_WindowProperties.m_Title.c_str(),
                monitors[0], nullptr);
            m_IsFullscreen = true;
        #else
            m_WindowProperties.m_Width  = m_WindowedWidth;
            m_WindowProperties.m_Height = m_WindowedHeight;

            m_Window = glfwCreateWindow(
                m_WindowProperties.m_Width,
                m_WindowProperties.m_Height,
                m_WindowProperties.m_Title.c_str(),
                nullptr, nullptr);
            // center window
            glfwSetWindowPos(m_Window,
                monitorX + m_WindowPositionX,
                monitorY + m_WindowPositionY);
            m_IsFullscreen = false;
            ToggleFullscreen();
        #endif
    }
    else
    {    
        m_WindowProperties.m_Width  = m_WindowedWidth;
        m_WindowProperties.m_Height = m_WindowedHeight;

        m_Window = glfwCreateWindow(
                    m_WindowProperties.m_Width, 
                    m_WindowProperties.m_Height, 
                    m_WindowProperties.m_Title.c_str(),
                    nullptr,
                    nullptr);
        m_IsFullscreen = false;
    }

    // center window
    glfwSetWindowPos(
            m_Window,
            monitorX + m_WindowPositionX,
            monitorY + m_WindowPositionY);

    // make the window visible
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwShowWindow(m_Window);

}

void VK_Window::SetWindowAspectRatio()
{
    // set aspect ratio to current ratio
    int numer = m_WindowProperties.m_Width;
    int denom = m_WindowProperties.m_Height;
    glfwSetWindowAspectRatio(m_Window, numer, denom);
}

void VK_Window::SetWindowAspectRatio(int numer, int denom)
{
    glfwSetWindowAspectRatio(m_Window, numer, denom);
}
