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

#include "core.h"
#include "input.h"
#include "engine.h"
#include "application.h"
#include "VKmodel.h"

std::shared_ptr<Application> Application::m_Instance;

Application::Application()
{
}

bool Application::Start()
{
    std::thread consoleInputHandler(ConsoleInputHandler);
    consoleInputHandler.detach();

    m_Window = Engine::m_Engine->GetWindow();
    m_Window->SetWindowAspectRatio();
    m_Window->DisallowCursor();
    m_Window->SetEntities(&m_Entities);

    LoadModel();

    return true;
}

void Application::Shutdown()
{
}

void Application::HandleInput(Transform2DComponent& transform)
{
    static constexpr float DEADZONE = 0.2f;
    static constexpr float SENSITIVITY = 0.05f;

    // left
    glm::vec2 controllerAxisInputLeft = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::LEFT_STICK);

    if (std::abs(controllerAxisInputLeft.x) > DEADZONE)
    {
        transform.m_Translation.x += controllerAxisInputLeft.x * SENSITIVITY;
    }
    transform.m_Translation.x = std::clamp(transform.m_Translation.x, -0.6f, 1.7f);
    
    if (std::abs(controllerAxisInputLeft.y) > DEADZONE)
    {
        transform.m_Translation.y -= controllerAxisInputLeft.y * SENSITIVITY;
    }
    transform.m_Translation.y = std::clamp(transform.m_Translation.y, -0.6f, 1.7f);

    // right
    glm::vec2 controllerAxisInputRight = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);
    
    if (std::abs(controllerAxisInputRight.x) > DEADZONE)
    {
        transform.m_Scale.x += controllerAxisInputRight.x * SENSITIVITY;
    }

    if (std::abs(controllerAxisInputRight.y) > DEADZONE)
    {
        transform.m_Scale.x -= controllerAxisInputRight.y * SENSITIVITY;
    }
    transform.m_Scale.x = std::clamp(transform.m_Scale.x, 0.01f, 2.0f);
    transform.m_Scale.y = transform.m_Scale.x;
}

void Application::OnUpdate()
{
    static Transform2DComponent transform{};
    HandleInput(transform);

    m_Entities.clear();
    static float rotation{};
    rotation = glm::mod(rotation + 0.025f, glm::two_pi<float>());
    {
        auto quad = Entity::CreateEnity();
        quad.m_Model = m_Model;
        quad.m_Color = glm::vec3{0.1f, 0.4f, 1.0f};
        quad.m_Transform2D.m_Scale = glm::vec2{transform.m_Scale.x*0.5f, transform.m_Scale.y*0.5f};
        quad.m_Transform2D.m_Translation = glm::vec2{-0.55f + transform.m_Translation.x,-0.55f + transform.m_Translation.y};
        quad.m_Transform2D.m_Rotation = rotation;
        m_Entities.push_back(std::move(quad));
    }
    {
        auto quad = Entity::CreateEnity();
        quad.m_Model = m_Model;
        quad.m_Color = glm::vec3{0.1f, 0.9f, 0.1f};
        quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f};
        quad.m_Transform2D.m_Translation = glm::vec2{0.55f,-0.55f};
        quad.m_Transform2D.m_Rotation = rotation;
        m_Entities.push_back(std::move(quad));
    }
    {
        auto quad = Entity::CreateEnity();
        quad.m_Model = m_Model;
        quad.m_Color = glm::vec3{0.6f, 0.1f, 0.1f};
        quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f};
        quad.m_Transform2D.m_Translation = glm::vec2{-0.55f, 0.55f};
        quad.m_Transform2D.m_Rotation = rotation;
        m_Entities.push_back(std::move(quad));
    }
    {
        auto quad = Entity::CreateEnity();
        quad.m_Model = m_Model;
        quad.m_Color = glm::vec3{0.5f, 0.4f, 0.3f};
        quad.m_Transform2D.m_Scale = glm::vec2{0.5f, 0.5f};
        quad.m_Transform2D.m_Translation = glm::vec2{0.55f, 0.55f};
        quad.m_Transform2D.m_Rotation = rotation;
        m_Entities.push_back(std::move(quad));
    }
}

std::shared_ptr<Application> Application::Create()
{
    if (!m_Instance)
    {
        m_Instance = std::make_shared<Application>();
    }
    return m_Instance;
}

void Application::ConsoleInputHandler()
{
    while (true)
    {
        LOG_APP_INFO("press enter to exit");
        getchar(); // block until enter is pressed
        Engine::m_Engine->Shutdown();
        break;
    }

}

void Application::LoadModel()
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
    m_Model = Engine::m_Engine->LoadModel(vertices);

}
