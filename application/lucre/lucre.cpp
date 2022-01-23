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
   
#include <thread>

#include "lucre.h"
#include "engine.h"
#include "coreSettings.h"
#include "resources/resources.h"
#include "events/controllerEvent.h"

namespace LucreApp
{

    std::shared_ptr<Lucre> Lucre::m_Instance;
    Engine* Lucre::m_Engine = nullptr;

    Lucre::Lucre()
        : m_UserInput{}
    {
    }

    bool Lucre::Start()
    {
        InitSettings();

        std::thread consoleInputHandler(ConsoleInputHandler);
        consoleInputHandler.detach();

        m_Engine = Engine::m_Engine;
        m_Renderer = m_Engine->GetRenderer();

        m_Window = m_Engine->GetWindow();
        m_Window->SetWindowAspectRatio();
        InitCursor();

        LoadModel();

        InputHandlerSpec inputSpec{};
        m_InputHandler = std::make_unique<InputHandler>(inputSpec);

        PlaySound(IDR_WAVES);

        return true;
    }

    void Lucre::Shutdown()
    {
    }

    void Lucre::OnUpdate()
    {
        // draw new scene
        m_Renderer->BeginScene();

        m_InputHandler->GetTransform(m_UserInput);

        m_Entities[0].m_Transform.m_Rotation.y = glm::mod(m_Entities[0].m_Transform.m_Rotation.y + 0.01f, glm::two_pi<float>());
        m_Entities[0].m_Transform.m_Rotation.z = glm::mod(m_Entities[0].m_Transform.m_Rotation.z + 0.01f, glm::two_pi<float>());
        
        m_Entities[0].m_Transform.m_Scale = m_UserInput.m_Scale;
        m_Entities[0].m_Transform.m_Translation.x = m_UserInput.m_Translation.x;
        m_Entities[0].m_Transform.m_Translation.y = m_UserInput.m_Translation.y;

        m_Renderer->Submit(m_Entities);
        m_Renderer->EndScene();
    }

    std::shared_ptr<Lucre> Lucre::Create()
    {
        if (!m_Instance)
        {
            m_Instance = std::make_shared<Lucre>();
        }
        return m_Instance;
    }

    void Lucre::ConsoleInputHandler()
    {
        while (true)
        {
            LOG_APP_INFO("press enter to exit");
            getchar(); // block until enter is pressed
            m_Engine->Shutdown();
            break;
        }

    }

    void Lucre::LoadModel()
    {
        std::vector<Vertex> vertices =
        {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        };

        glm::vec3 offset{0.0f, 0.0f, 0.0f};
        for (auto& vertex : vertices)
        {
            vertex.position += offset;
        }

        m_Model = m_Engine->LoadModel(vertices);

        m_Entities.clear();
        auto cube = Entity::CreateEnity();
        cube.m_Model = m_Model;
        cube.m_Transform.m_Translation = glm::vec3{0.0f, 0.0f, 0.5f};
        cube.m_Transform.m_Scale = glm::vec3{0.5f, 0.5f, 0.5f};
        m_Entities.push_back(std::move(cube));

    }

    void Lucre::InitCursor()
    {
        size_t fileSize;
        const uchar* data;

        m_EmptyCursor = Cursor::Create();
        data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/images/cursorEmpty.png", IDB_CURSOR_EMPTY, "PNG");
        m_EmptyCursor->SetCursor(data, fileSize, 1, 1);

        m_Cursor = Cursor::Create();
        data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/images/cursor.png", IDB_CURSOR_RETRO, "PNG");
        m_Cursor->SetCursor(data, fileSize, 32, 32);

        m_Engine->AllowCursor();
    }

    void Lucre::ShowCursor()
    {
        m_Cursor->RestoreCursor();
    }

    void Lucre::HideCursor()
    {
        m_EmptyCursor->RestoreCursor();
    }

    void Lucre::InitSettings()
    {
        m_AppSettings.InitDefaults();
        m_AppSettings.RegisterSettings();

        // apply external settings
        m_Engine->ApplyAppSettings();
    }

    void Lucre::PlaySound(int resourceID)
    {
        if (CoreSettings::m_EnableSystemSounds)
        {
            switch(resourceID)
            {
                case IDR_WAVES:
                    m_Engine->PlaySound("/sounds/waves.ogg", IDR_WAVES, "OGG");
                    break;
                case IDR_BUCKLE:
                    m_Engine->PlaySound("/sounds/buckle.ogg", IDR_BUCKLE, "OGG");
                    break;
            }

        }
    }

    void Lucre::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<ControllerButtonPressedEvent>([this](ControllerButtonPressedEvent event)
            {
                switch (event.GetControllerButton())
                {
                    case Controller::BUTTON_GUIDE:
                        m_Engine->Shutdown();
                        break;
                    case Controller::BUTTON_A:
                        PlaySound(IDR_BUCKLE);
                        break;
                }
                return false;
            }
        );
    }
}
