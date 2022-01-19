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

#include "VKwindow.h"

bool VK_Window::m_GLFWIsInitialized = false;

VK_Window::VK_Window(const WindowProperties& props)
    : m_OK(false), m_IsFullscreen(false)
{
    m_WindowProperties.m_Title    = props.m_Title;
    m_WindowProperties.m_Width    = props.m_Width;
    m_WindowProperties.m_Height   = props.m_Height;
    //m_WindowProperties.m_VSync    = props.m_VSync;
    //m_WindowProperties.m_MousePosX= 0.0f;
    //m_WindowProperties.m_MousePosY= 0.0f;

    //m_AllowCursor = false;
    if (!m_GLFWIsInitialized)
    {
        // init glfw
        m_GLFWIsInitialized = InitGLFW();
    }
    if (m_GLFWIsInitialized)
    {
        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
        m_RefreshRate = videoMode->refreshRate;
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(800, 600, props.m_Title.c_str(), nullptr, nullptr);

        uint extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        LOG_CORE_INFO("{0}  extensions supported", extensionCount);

        // create a device
        m_Device = std::make_shared<VK_Device>(this);

        // create the swapchain
        m_SwapChain = std::make_shared<VK_SwapChain>(m_Device, GetExtend());

        std::vector<VK_Model::Vertex> vertices =
        {
            {glm::vec2( 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2(-0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)}
        };
        m_Model = std::make_shared<VK_Model>(m_Device, vertices);

        CreatePipelineLayout();
        CreatePipeline();
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
    //    int count;
    //    GLFWmonitor** monitors = glfwGetMonitors(&count);
    //    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    //    m_RefreshRate = videoMode->refreshRate;
    //
    //    // make window invisible before it gets created
    //    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    //
    //    int monitorX, monitorY;
    //    glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);
    //
    //    if (m_WindowProperties.m_Width == -1)
    //    {
    //        m_WindowedWidth = videoMode->width / 1.5f;
    //        m_WindowedHeight = m_WindowedWidth / 16 * 9;
    //        m_WindowPositionX = (videoMode->width - m_WindowedWidth) / 2;
    //        m_WindowPositionY = (videoMode->height - m_WindowedHeight) / 2;
    //
    //        if (CoreSettings::m_EnableFullscreen)
    //        {
    //            #ifdef _WIN32    
    //                m_WindowProperties.m_Width = videoMode->width;
    //                m_WindowProperties.m_Height = videoMode->height;
    //                m_Window = glfwCreateWindow(
    //                    m_WindowProperties.m_Width,
    //                    m_WindowProperties.m_Height,
    //                    m_WindowProperties.m_Title.c_str(),
    //                    monitors[0], NULL);
    //                m_IsFullscreen = true;
    //            #else
    //                m_WindowProperties.m_Width  = m_WindowedWidth;
    //                m_WindowProperties.m_Height = m_WindowedHeight;
    //
    //                m_Window = glfwCreateWindow(
    //                    m_WindowProperties.m_Width,
    //                    m_WindowProperties.m_Height,
    //                    m_WindowProperties.m_Title.c_str(),
    //                    NULL, NULL);
    //                // center window
    //                glfwSetWindowPos(m_Window,
    //                    monitorX + m_WindowPositionX,
    //                    monitorY + m_WindowPositionY);
    //                m_IsFullscreen = false;
    //                ToggleFullscreen();
    //            #endif
    //        }
    //        else
    //        {
    //            m_WindowProperties.m_Width  = m_WindowedWidth;
    //            m_WindowProperties.m_Height = m_WindowedHeight;
    //
    //            m_Window = glfwCreateWindow(
    //                        m_WindowProperties.m_Width, 
    //                        m_WindowProperties.m_Height, 
    //                        m_WindowProperties.m_Title.c_str(), 
    //                        NULL, NULL);
    //        }
    //    }
    //    else
    //    {
    //        m_WindowedWidth = m_WindowProperties.m_Width;
    //        m_WindowedHeight = m_WindowProperties.m_Height;
    //        m_WindowPositionX = (videoMode->width - m_WindowedWidth) / 2;
    //        m_WindowPositionY = (videoMode->height - m_WindowedHeight) / 2;
    //
    //        m_WindowProperties.m_Width = m_WindowedWidth;
    //        m_WindowProperties.m_Height = m_WindowedHeight;
    //
    //        m_Window = glfwCreateWindow(
    //            m_WindowProperties.m_Width,
    //            m_WindowProperties.m_Height,
    //            m_WindowProperties.m_Title.c_str(),
    //            NULL, NULL);
    //    }
    //    
    //    if (!m_Window)
    //    {
    //        LOG_CORE_CRITICAL("Failed to create main window");
    //        char description[1024];
    //        int errorCode = glfwGetError((const char **)(&description));
    //
    //        if (errorCode != GLFW_NO_ERROR)
    //        {
    //            LOG_CORE_CRITICAL("glfw error code: {0}", errorCode);
    //        }
    //        glfwTerminate();
    //    }
    //    else
    //    {
    //        if (!CoreSettings::m_EnableFullscreen)
    //        {
    //            // center window
    //            glfwSetWindowPos(m_Window,
    //                monitorX + m_WindowPositionX,
    //                monitorY + m_WindowPositionY);
    //        }
    //        
    //        
    //        // make the window visible
    //        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    //        glfwShowWindow(m_Window);
    //
    //        // set app icon
    //        GLFWimage icon;
    //        size_t fileSize;
    //        const uchar* data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/atlas/images/I_ENGINE.png", IDB_ENGINE_LOGO, "PNG");
    //        icon.pixels = stbi_load_from_memory(data, fileSize, &icon.width, &icon.height, 0, 4); //rgba channels
    //        if (icon.pixels) 
    //        {
    //            glfwSetWindowIcon(m_Window, 1, &icon); 
    //            stbi_image_free(icon.pixels);
    //        }
    //        else
    //        {
    //            LOG_CORE_WARN("Could not load app icon");
    //        }
    //        
    //        m_GraphicsContext = GraphicsContext::Create(m_Window, m_RefreshRate);
    //        if (!m_GraphicsContext->Init())
    //        {
    //            LOG_CORE_CRITICAL("Could not create a rendering context");
    //            
    //        }
    //        else
    //        {
    //            SetVSync(props.m_VSync);
    //        }
    //
    //        // init glew
    //        if (InitGLEW())
    //        {
    //            // all good
    //            m_OK = true;
    //        }
    //    }
    //}
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
//void VK_Window::ToggleFullscreen()
//{ 
//    int count;
//    GLFWmonitor** monitors = glfwGetMonitors(&count);
//    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
//    if (m_IsFullscreen)
//    {
//        m_WindowProperties.m_Width  = m_WindowedWidth;
//        m_WindowProperties.m_Height = m_WindowedHeight;
//
//        glfwSetWindowMonitor(m_Window, nullptr, m_WindowPositionX, m_WindowPositionY, m_WindowedWidth, m_WindowedHeight, videoMode->refreshRate);
//        glfwSetWindowPos(m_Window, m_WindowPositionX, m_WindowPositionY);
//    }
//    else
//    {
//        m_WindowedWidth = m_WindowProperties.m_Width; 
//        m_WindowedHeight = m_WindowProperties.m_Height;
//        glfwGetWindowPos(m_Window, &m_WindowPositionX, &m_WindowPositionY);
//
//        glfwSetWindowMonitor(m_Window, monitors[0], 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
//    }
//    m_IsFullscreen = !m_IsFullscreen;
//}
//
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

//void VK_Window::OnError(int errorCode, const char* description) 
//{
//        LOG_CORE_CRITICAL("GLEW error, code: {0}, description: {1}", std::to_string(errorCode), description);
//
//}
//
//void VK_Window::SetEventCallback(const EventCallbackFunction& callback)
//{
//    m_WindowProperties.m_EventCallback = callback;
//    glfwSetWindowUserPointer(m_Window,&m_WindowProperties);
//    
//    glfwSetErrorCallback([](int errorCode, const char* description) { VK_Window::OnError(errorCode, description);});
//    
//    glfwSetKeyCallback(m_Window,[](GLFWwindow* window, int key, int scancode, int action, int modes)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//            
//            switch (action)
//            {
//                case GLFW_PRESS:
//                {
//                    KeyPressedEvent event(key);
//                    OnEvent(event);
//                    break;
//                }
//                case GLFW_RELEASE:
//                {
//                    KeyReleasedEvent event(key);
//                    OnEvent(event);
//                    break;
//                }
//                case GLFW_REPEAT:
//                    break;
//            }
//        }
//    );
//    
//    glfwSetWindowCloseCallback(m_Window,[](GLFWwindow* window)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//                
//            WindowCloseEvent event;
//            OnEvent(event);
//        }
//    );
//
//    glfwSetWindowSizeCallback(m_Window,[](GLFWwindow* window, int width, int height)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//            
//            windowProperties.m_Width = width;
//            windowProperties.m_Height = height;
//                
//            WindowResizeEvent event(width, height);
//            OnEvent(event);
//        }
//    );
//
//    glfwSetFramebufferSizeCallback(m_Window,[](GLFWwindow* window, int width, int height)
//        {
//            GLCall(glViewport(0, 0, width, height));
//        }
//    );
//
//    glfwSetWindowIconifyCallback(m_Window,[](GLFWwindow* window, int iconified)
//        {
//            int width, height;
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//            
//            if (iconified)
//            {
//                width = height = 0;
//            }
//            else
//            {
//                glfwGetWindowSize(window, &width, &height);
//            }
//            
//            windowProperties.m_Width = width;
//            windowProperties.m_Height = height;
//                
//            WindowResizeEvent event(width, height);
//            OnEvent(event);
//        }
//    );
//
//    glfwSetMouseButtonCallback(m_Window,[](GLFWwindow* window, int button, int action, int mods)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//            
//            switch (action)
//            {
//                case GLFW_PRESS:
//                {
//                    MouseButtonPressedEvent event(button,windowProperties.m_MousePosX,windowProperties.m_MousePosY);
//                    OnEvent(event);
//                    break;
//                }
//                case GLFW_RELEASE:
//                {
//                    MouseButtonReleasedEvent event(button);
//                    OnEvent(event);
//                    break;
//                }
//            }
//        }
//    );
//    
//    glfwSetCursorPosCallback(m_Window,[](GLFWwindow* window, double xpos, double ypos)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//            
//            windowProperties.m_MousePosX = xpos;
//            windowProperties.m_MousePosY = ypos;
//                        
//            MouseMovedEvent event(xpos, ypos);
//            OnEvent(event);
//            
//        }
//    );
//    
//    glfwSetScrollCallback(m_Window,[](GLFWwindow* window, double xoffset, double yoffset)
//        {
//            WindowData& windowProperties = *(WindowData*)glfwGetWindowUserPointer(window);
//            EventCallbackFunction OnEvent = windowProperties.m_EventCallback;
//                        
//            MouseScrolledEvent event(xoffset, yoffset);
//            OnEvent(event);
//            
//        }
//    );
//}

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

//void VK_Window::EnableMousePointer()
//{
//    if (m_AllowCursor)
//    {
//        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//    }
//}
//
//void VK_Window::DisableMousePointer()
//{
//    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//}
//
//void VK_Window::AllowCursor()
//{
//    m_AllowCursor = true;
//}
//void VK_Window::DisallowCursor()
//{
//    m_AllowCursor = false;
//    DisableMousePointer();
//}
void VK_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("Could not create surface");
    }
}

void VK_Window::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(m_Device->Device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to create pipeline layout!");
    }
}
void VK_Window::CreatePipeline()
{
    auto pipelineConfig = VK_Pipeline::DefaultPipelineConfigInfo(m_SwapChain->Width(), m_SwapChain->Height());
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

    for (uint i = 0; i < m_CommandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("failed to begin recording command buffer");
        }
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(i);
        
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_Pipeline->Bind(m_CommandBuffers[i]);
        m_Model->Bind(m_CommandBuffers[i]);
        m_Model->Draw(m_CommandBuffers[i]);

        vkCmdEndRenderPass(m_CommandBuffers[i]);
        if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
        {
            LOG_CORE_CRITICAL("recording of command buffer failed");
        }

        //VkViewport viewport{};
        //viewport.x = 0.0f;
        //viewport.y = 0.0f;
        //viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
        //viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
        //viewport.minDepth = 0.0f;
        //viewport.maxDepth = 1.0f;
        //VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
        //vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        //vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
}

void VK_Window::DrawFrame()
{
    uint imageIndex = 0;
    auto result = m_SwapChain->AcquireNextImage(&imageIndex);
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_CORE_CRITICAL("failed to acquire next swap chain image");
    }
    
    result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS)
    {
        LOG_CORE_CRITICAL("failed to present swap chain image");
    }
}
