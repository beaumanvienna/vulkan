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
        m_Engine->SetEntities(&m_Entities);

        m_Window = m_Engine->GetWindow();
        m_Window->SetWindowAspectRatio();
        InitCursor();

        LoadModel();

        InputHandlerSpec inputSpec{};
        m_InputHandler = std::make_unique<InputHandler>(inputSpec);

        return true;
    }

    void Lucre::Shutdown()
    {
    }

    void Lucre::OnUpdate()
    {
        m_InputHandler->OnUpdate(m_UserInput);

        float scaleAspectRatio = 1.0f;
        if (CoreSettings::m_EnableFullscreen)
        {
            scaleAspectRatio = 1.0f / m_Window->GetWindowAspectRatio();
        }

        m_Entities.clear();
        static float rotation{};
        rotation = glm::mod(rotation + 0.025f, glm::two_pi<float>());
        {
            auto quad = Entity::CreateEnity();
            quad.m_Model = m_Model;
            quad.m_Color = glm::vec3{0.1f, 0.4f, 1.0f};
            quad.m_Transform2D.m_Scale = glm::vec2{m_UserInput.m_Scale.x*0.5f, m_UserInput.m_Scale.y*0.5f} * scaleAspectRatio;
            quad.m_Transform2D.m_Translation = glm::vec2{-0.55f + m_UserInput.m_Translation.x,-0.55f + m_UserInput.m_Translation.y};
            quad.m_Transform2D.m_Rotation = rotation;
            m_Entities.push_back(std::move(quad));
        }
        {
            auto quad = Entity::CreateEnity();
            quad.m_Model = m_Model;
            quad.m_Color = glm::vec3{0.1f, 0.9f, 0.1f};
            quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f} * scaleAspectRatio;
            quad.m_Transform2D.m_Translation = glm::vec2{0.55f,-0.55f};
            quad.m_Transform2D.m_Rotation = rotation;
            m_Entities.push_back(std::move(quad));
        }
        {
            auto quad = Entity::CreateEnity();
            quad.m_Model = m_Model;
            quad.m_Color = glm::vec3{0.6f, 0.1f, 0.1f};
            quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f} * scaleAspectRatio;
            quad.m_Transform2D.m_Translation = glm::vec2{-0.55f, 0.55f};
            quad.m_Transform2D.m_Rotation = rotation;
            m_Entities.push_back(std::move(quad));
        }
        {
            auto quad = Entity::CreateEnity();
            quad.m_Model = m_Model;
            quad.m_Color = glm::vec3{0.5f, 0.4f, 0.3f};
            quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f} * scaleAspectRatio;
            quad.m_Transform2D.m_Translation = glm::vec2{0.55f, 0.55f};
            quad.m_Transform2D.m_Rotation = rotation;
            m_Entities.push_back(std::move(quad));
        }
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
            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2(-0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2(-0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2(-0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2(-0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},

            {glm::vec2(-0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec2( 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec2( 0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f)},
        };
        m_Model = m_Engine->LoadModel(vertices);

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

}
