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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include <thread>

#include "engine.h"
#include "coreSettings.h"
#include "resources/resources.h"
#include "events/controllerEvent.h"
#include "events/applicationEvent.h"
#include "events/mouseEvent.h"

#include "lucre.h"
#include "keyboardInputController.h"

namespace LucreApp
{

    std::shared_ptr<Lucre> Lucre::m_Instance;
    Engine* Lucre::m_Engine = nullptr;

    Lucre::Lucre()
        : m_GamepadInput{},
          m_CurrentScene{nullptr}
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

        m_GameState.Start();

        m_CameraController = std::make_shared<CameraController>();
        m_CameraController->SetTranslationSpeed(400.0f);
        m_CameraController->SetRotationSpeed(0.5f);

        m_CameraObject.reset( new Entity(Entity::CreateEntity()));

        KeyboardInputControllerSpec keyboardInputControllerSpec{};
        m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

        LoadModels();

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);

        PlaySound(IDR_WAVES);

        return true;
    }

    void Lucre::Shutdown()
    {
        m_GameState.Stop();
    }

    void Lucre::OnUpdate(const Timestep& timestep)
    {
        m_CurrentScene = m_GameState.OnUpdate();
        m_CurrentScene->OnUpdate(timestep);

        m_KeyboardInputController->MoveInPlaneXZ(timestep, *m_CameraObject);
        m_CameraController->SetViewYXZ
        (
            m_CameraObject->m_Transform.m_Translation,
            m_CameraObject->m_Transform.m_Rotation
        );

        // draw new scene
        m_Renderer->BeginScene(m_CameraController->GetCamera());

        m_GamepadInputController->GetTransform(m_Entities[0].m_Transform);
        m_GamepadInputController->GetTransform(m_Entities[1].m_Transform, true);
        m_GamepadInputController->GetTransform(m_Entities[2].m_Transform, true);

        auto frameRotation = static_cast<const float>(timestep) * 0.0006f;
        m_Entities[1].m_Transform.m_Rotation.y = glm::mod(m_Entities[1].m_Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        m_Entities[1].m_Transform.m_Rotation.z = glm::mod(m_Entities[1].m_Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());
        m_Entities[2].m_Transform.m_Rotation.y = glm::mod(m_Entities[2].m_Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        m_Entities[2].m_Transform.m_Rotation.z = glm::mod(m_Entities[2].m_Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());

        m_Renderer->Submit(m_Entities);
        m_Renderer->EndScene();
    }

    void Lucre::OnResize()
    {
        m_CameraController->SetProjection();
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

    void Lucre::LoadModels()
    {
        Builder builder{};

        // base cube
        builder.LoadModel("application/lucre/models/colored_cube.obj");
        m_Model = m_Engine->LoadModel(builder);
        auto object0 = Entity::CreateEntity();
        object0.m_Model = m_Model;
        object0.m_Transform.m_Translation = glm::vec3{0.0f, 0.7f, 2.5f};
        object0.m_Transform.m_Scale = glm::vec3{0.01f, 2.0f, 2.0f};
        object0.m_Transform.m_Rotation = glm::vec3{0.0f, 0.0f, glm::half_pi<float>()};
        m_Entities.push_back(std::move(object0));

        // base sphere
        //builder.LoadModel("application/lucre/models/sphere.obj");
        //m_Model = m_Engine->LoadModel(builder);
        //auto object0 = Entity::CreateEntity();
        //object0.m_Model = m_Model;
        //object0.m_Transform.m_Translation = glm::vec3{0.0f, 10.7f, 2.5f};
        //object0.m_Transform.m_Scale = glm::vec3{10.0f};
        //object0.m_Transform.m_Rotation = glm::vec3{0.0f};
        //m_Entities.push_back(std::move(object0));

        // moving onjects
        builder.LoadModel("application/lucre/models/flat_vase.obj");
        m_Model = m_Engine->LoadModel(builder);
        auto object1 = Entity::CreateEntity();
        object1.m_Model = m_Model;
        object1.m_Transform.m_Translation = glm::vec3{-0.8f, -0.2f, 2.5f};
        object1.m_Transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
        m_Entities.push_back(std::move(object1));

        builder.LoadModel("application/lucre/models/smooth_vase.obj");
        m_Model = m_Engine->LoadModel(builder);
        auto object2 = Entity::CreateEntity();
        object2.m_Model = m_Model;
        object2.m_Transform.m_Translation = glm::vec3{0.8f, -0.2f, 2.5f};
        object2.m_Transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
        m_Entities.push_back(std::move(object2));

        builder.LoadModel("application/lucre/models/sphere.obj");
        m_Model = m_Engine->LoadModel(builder);
        auto object3 = Entity::CreateEntity();
        object3.m_Model = m_Model;
        object3.m_Transform.m_Translation = glm::vec3{0.0f, -0.2f, 2.5f};
        object3.m_Transform.m_Scale = glm::vec3{0.05f};
        m_Entities.push_back(std::move(object3));

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

        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent event)
            {
                OnResize();
                return true;
            }
        );

        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent event)
            {
                auto zoomFactor = m_CameraController->GetZoomFactor();
                zoomFactor -= event.GetY()*0.1f;
                m_CameraController->SetZoomFactor(zoomFactor);
                return true;
            }
        );
    }
}
