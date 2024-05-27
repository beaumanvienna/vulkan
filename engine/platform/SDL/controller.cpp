/* Controller Copyright (c) 2021 Controller Development Team
   https://github.com/beaumanvienna/gfxRenderController

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

#include <fstream>
#include <filesystem>
#include <cmath>

#include "core.h"
#include "platform/input.h"
#include "events/joystickEvent.h"
#include "events/controllerEvent.h"
#include "platform/SDL/controller.h"
#include "auxiliary/memoryStream.h"
#include "resources/resources.h"

namespace GfxRenderEngine
{

    ControllerConfiguration Controller::m_ControllerConfiguration;

    Controller::Controller() : m_Initialzed(false)
    {
        m_Gamecontrollerdb = "resources/sdl/gamecontrollerdb.txt";

        SetNormalEventLoop();
    }

    Controller::~Controller() { CloseAllControllers(); }

    void Controller::StartConfig(int controllerID)
    {
        SetConfigEventLoop();
        m_ControllerConfiguration.Start(controllerID);
    }

    void Controller::SetEventCallback(const EventCallbackFunction& callback) { m_EventCallback = callback; }

    bool Controller::Start()
    {
        m_Initialzed = false;

        m_InternalDB = Engine::m_Engine->GetConfigFilePath() + "internalDB.txt";

        if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
        {
            LOG_CORE_WARN("Could not initialize SDL game controller subsystem");
        }
        else
        {
            if (SDL_GameControllerAddMappingsFromFile(m_InternalDB.c_str()) != -1)
            {
                LOG_CORE_INFO("{0} found", m_InternalDB);
            }

            // load file from memory
            size_t fileSize;
            void* data =
                (void*)ResourceSystem::GetDataPointer(fileSize, "/text/sdl/gamecontrollerdb.txt", IDR_SD_LCTRL_DB, "TEXT");

            SDL_RWops* sdlRWOps = SDL_RWFromMem(data, fileSize);
            if (SDL_GameControllerAddMappingsFromRW(sdlRWOps, 1) != -1)
            {
                m_Initialzed = true;
            }
            else
            {
                // load file from disk
                LOG_CORE_WARN("Could not load gamecontrollerdb.txt from memory, trying to load '{0}' from disk",
                              m_Gamecontrollerdb);
                if (SDL_GameControllerAddMappingsFromFile(m_Gamecontrollerdb.c_str()) != -1)
                {
                    m_Initialzed = true;
                }
                else
                {
                    LOG_CORE_WARN("Could not open gamecontrollerdb.txt");
                }
            }
            if (m_Initialzed)
            {
                LOG_CORE_INFO("SDL game controller subsystem initialized");
            }
        }
        Input::Start(this);
        return m_Initialzed;
    }

    bool Controller::Restart()
    {
        LOG_CORE_INFO("Restarting controller subsystem");
        CloseAllControllers();
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
        return Start();
    }

    void Controller::OnUpdate()
    {
        // Event handler
        SDL_Event SDLevent;

        // Handle events on queue
        while (SDL_PollEvent(&SDLevent) != 0)
        {
            m_EventLoop(SDLevent);
        }
    }

    void Controller::EventLoop(SDL_Event& SDLevent)
    {
        // main event loop
        switch (SDLevent.type)
        {
            case SDL_JOYDEVICEADDED:
                AddController(SDLevent.jdevice.which);
                break;
            case SDL_JOYDEVICEREMOVED:
                RemoveController(SDLevent.jdevice.which);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
            {
                int indexID = m_InstanceToIndex[SDLevent.cbutton.which];
                int controllerButton = SDLevent.cbutton.button;
                ControllerButtonPressedEvent event(indexID, controllerButton);
                m_EventCallback(event);
                break;
            }
            case SDL_CONTROLLERBUTTONUP:
            {
                int indexID = m_InstanceToIndex[SDLevent.cbutton.which];
                int controllerButton = SDLevent.cbutton.button;
                ControllerButtonReleasedEvent event(indexID, controllerButton);
                m_EventCallback(event);
                break;
            }
            case SDL_CONTROLLERAXISMOTION:
            {
                // int indexID = m_InstanceToIndex[SDLevent.caxis.which];
                // int axis = SDLevent.caxis.axis;
                // int value = SDLevent.caxis.value;
                break;
            }
            case SDL_JOYBUTTONDOWN:
            {
                int indexID = m_InstanceToIndex[SDLevent.jbutton.which];
                int joystickButton = SDLevent.jbutton.button;
                JoystickButtonPressedEvent event(indexID, joystickButton);
                m_EventCallback(event);
                break;
            }
            case SDL_JOYBUTTONUP:
            {
                int indexID = m_InstanceToIndex[SDLevent.jbutton.which];
                int joystickButton = SDLevent.jbutton.button;
                JoystickButtonReleasedEvent event(indexID, joystickButton);
                m_EventCallback(event);
                break;
            }
            case SDL_JOYAXISMOTION:
            {
                // int indexID = m_InstanceToIndex[SDLevent.jaxis.which];
                // int axis = SDLevent.jaxis.axis;
                // int value = SDLevent.jaxis.value;
                break;
            }
            case SDL_JOYHATMOTION:
            {
                // int indexID = m_InstanceToIndex[SDLevent.jhat.which];
                // int hat = SDLevent.jhat.hat;
                // int value = SDLevent.jhat.value;
                break;
            }
            case SDL_JOYBALLMOTION:
            {
                int indexID = m_InstanceToIndex[SDLevent.jhat.which];
                int ball = SDLevent.jball.ball;
                int xrel = SDLevent.jball.xrel;
                int yrel = SDLevent.jball.yrel;
                JoystickBallMovedEvent event(indexID, ball, xrel, yrel);
                m_EventCallback(event);
                break;
            }
        }
    }

    void Controller::ConfigEventLoop(SDL_Event& SDLevent)
    {
        if (!m_ControllerConfiguration.IsRunning())
        {
            if (m_ControllerConfiguration.MappingCreated())
            {
                std::string entry = m_ControllerConfiguration.GetDatabaseEntry();
                if (AddControllerToInternalDB(entry))
                {
                    LOG_CORE_INFO("added to internal db: {0}", entry);
                }

                RemoveDuplicatesInDB();

                int ret = SDL_GameControllerAddMappingsFromFile(m_InternalDB.c_str());
                if (ret == -1)
                {
                    LOG_CORE_CRITICAL("Warning: Unable to open internal controller database: {0}", m_InternalDB);
                }

                Restart();
            }

            SetNormalEventLoop();
            return;
        }

        // debounce buttons
        std::chrono::time_point<std::chrono::high_resolution_clock> previousTimeStamp;
        bool discardEvent;

        if (SDLevent.type == SDL_JOYBUTTONDOWN)
        {
            previousTimeStamp = m_TimeStamp;
            m_TimeStamp = Engine::m_Engine->GetTime();
            auto timeSinceLastEvent = m_TimeStamp - previousTimeStamp;
            discardEvent = timeSinceLastEvent < DEBOUNCE_TIME;
            if (discardEvent)
                return;
        }

        switch (SDLevent.type)
        {
            case SDL_JOYDEVICEADDED:
            {
                AddController(SDLevent.jdevice.which);
                break;
            }
            case SDL_JOYDEVICEREMOVED:
            {
                RemoveController(SDLevent.jdevice.which);
                break;
            }
            case SDL_JOYBUTTONDOWN:
            {
                m_ActiveController = m_InstanceToIndex[SDLevent.jbutton.which];
                int joystickButton = SDLevent.jbutton.button;
                m_ControllerConfiguration.StatemachineConf(joystickButton);
                break;
            }
            case SDL_JOYAXISMOTION:
            {
                m_ActiveController = m_InstanceToIndex[SDLevent.jaxis.which];
                int axis = SDLevent.jaxis.axis;
                int value = SDLevent.jaxis.value;

                if (abs(value) > 16384)
                {
                    m_ControllerConfiguration.StatemachineConfAxis(axis, (value < 0));
                }
                break;
            }
            case SDL_JOYHATMOTION:
            {
                m_ActiveController = m_InstanceToIndex[SDLevent.jhat.which];
                int hat = SDLevent.jhat.hat;
                int value = SDLevent.jhat.value;

                if ((value == SDL_HAT_UP) || (value == SDL_HAT_DOWN) || (value == SDL_HAT_LEFT) || (value == SDL_HAT_RIGHT))
                {
                    m_ControllerConfiguration.StatemachineConfHat(hat, value);
                }
                break;
            }
        }
    }

    void Controller::Shutdown()
    {
        CloseAllControllers();
        m_Initialzed = false;
    }

    void Controller::PrintJoyInfo(int indexID)
    {
        SDL_Joystick* joy = SDL_JoystickOpen(indexID);
        char guidStr[1024];
        // const char* name = SDL_JoystickName(joy);
        // int num_axes = SDL_JoystickNumAxes(joy);
        // int num_buttons = SDL_JoystickNumButtons(joy);
        // int num_hats = SDL_JoystickNumHats(joy);
        // int num_balls = SDL_JoystickNumBalls(joy);
        int instance = SDL_JoystickInstanceID(joy);
        char* mapping;
        SDL_GameController* gameCtrl;

        SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);

        SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));

        if (SDL_IsGameController(indexID))
        {
            LOG_CORE_INFO("Index: {0}, Instance: {1}, Name: {2}, Number of axes: {3}, Number of buttons: {4}, Number of "
                          "balls: {5}, compatible game controller",
                          indexID, instance, SDL_JoystickNameForIndex(indexID), SDL_JoystickNumAxes(joy),
                          SDL_JoystickNumButtons(joy), SDL_JoystickNumBalls(joy));

            gameCtrl = SDL_GameControllerOpen(indexID);
            mapping = SDL_GameControllerMapping(gameCtrl);
            if (mapping)
            {
                // LOG_CORE_INFO("Mapped as: {0}", mapping);
                SDL_free(mapping);
            }
        }
        else
        {
            LOG_CORE_ERROR(
                "Index: {0}, Instance: {1}, Name: {2}, Number of axes: {3}, Number of buttons: {4}, Number of balls: {5} ",
                indexID, instance, SDL_JoystickNameForIndex(indexID), SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy),
                SDL_JoystickNumBalls(joy));
            LOG_CORE_ERROR("Index {0} is not a compatible controller", indexID);
        }
    }

    void Controller::AddController(int indexID)
    {
        ControllerData controller;
        SDL_Joystick* joy;
        joy = SDL_JoystickOpen(indexID);

        if (joy)
        {
            if (CheckControllerIsSupported(indexID))
            {
                controller.m_IndexID = indexID;
                controller.m_Joystick = joy;
                controller.m_InstanceID = SDL_JoystickInstanceID(joy);

                std::string joystickName = SDL_JoystickNameForIndex(indexID);
                transform(joystickName.begin(), joystickName.end(), joystickName.begin(),
                          [](unsigned char c) { return tolower(c); });
                controller.m_Name = joystickName;

                // mapping
                SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
                CheckMapping(guid, controller.m_MappingOK, controller.m_Name);

                if (SDL_IsGameController(indexID))
                {
                    controller.m_GameController = SDL_GameControllerOpen(indexID);

                    char* mappingString;
                    mappingString = SDL_GameControllerMapping(controller.m_GameController);
                    if (mappingString)
                    {
                        std::string str = mappingString;
                        SDL_free(mappingString);
                        // remove guid
                        str = str.substr(str.find(",") + 1, str.length() - (str.find(",") + 1));
                        // extract name from db
                        str = str.substr(0, str.find(","));

                        transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });

                        controller.m_NameDB = str;
                    }
                }
                else
                {
                    LOG_CORE_ERROR("Index {0} is not a compatible controller", indexID);
                }
                LOG_CORE_INFO("Adding controller index: {0}, instance: {1}, name: {2}, name in gamecontrollerdb.txt: {3}",
                              controller.m_IndexID, controller.m_InstanceID, controller.m_Name,
                              (controller.m_MappingOK ? controller.m_NameDB : "not found"));

                LOG_CORE_INFO("number of axes: {0}, number of buttons: {1}, number of balls: {2}, {3}",
                              SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy), SDL_JoystickNumBalls(joy),
                              (controller.m_MappingOK ? "mapping ok (compatible game controller)" : "mapping not ok"));

                LOG_CORE_INFO("active controllers: {0}", SDL_NumJoysticks());
                m_Controllers.push_back(controller);
                controller.m_Joystick = nullptr; // checked in destrcutor

                m_InstanceToIndex.push_back(indexID);
            }
        }
        else
        {
            LOG_CORE_ERROR("Couldn't open Joystick {0}", indexID);
        }

        // Close if opened
        if (SDL_JoystickGetAttached(joy))
        {
            SDL_JoystickClose(joy);
        }
    }

    void Controller::RemoveController(int instanceID)
    {
        for (auto controller = m_Controllers.begin(); controller != m_Controllers.end(); controller++)
        {
            if (controller->m_InstanceID == instanceID)
            {
                controller = m_Controllers.erase(controller);
                break;
            }
        }
    }

    SDL_GameController* Controller::GetGameController(int indexID) const
    {
        auto controller = m_Controllers.begin();
        for (int i = 0; i < indexID; i++)
        {
            controller++;
        }
        return controller->m_GameController;
    }

    SDL_Joystick* Controller::GetJoystick(int indexID) const
    {
        auto controller = m_Controllers.begin();
        for (int i = 0; i < indexID; i++)
        {
            controller++;
        }
        return controller->m_Joystick;
    }

    void Controller::CloseAllControllers() { m_Controllers.clear(); }

    bool Controller::CheckControllerIsSupported(int indexID)
    {
        SDL_Joystick* joy = SDL_JoystickOpen(indexID);

        bool ok = false;
        std::string unsupported = "Nintendo Wii";
        std::string name = SDL_JoystickName(joy);

        // check for unsupported
        if (name.find(unsupported) != std::string::npos)
        {
            printf("not supported, ignoring controller: %s\n", name.c_str());
        }
        else
        {
            ok = true;
        }
        return ok;
    }

    bool Controller::CheckMapping(SDL_JoystickGUID guid, bool& mappingOK, std::string& name)
    {
        char guidStr[1024];
        std::string line, append, filename;

        mappingOK = false;

        // set up guidStr
        SDL_JoystickGetGUIDString(guid, guidStr, sizeof(guidStr));

        if (FindGuidInFile(m_InternalDB, guidStr, 32, &line))
        {
            LOG_CORE_INFO("GUID found in internal db");
            mappingOK = true;
            return mappingOK;
        }

        // check public db
        mappingOK = FindGuidInFile("/text/sdl/gamecontrollerdb.txt", IDR_SD_LCTRL_DB, "TEXT", guidStr, 32, &line);

        if (mappingOK)
        {
            LOG_CORE_INFO("GUID {0} found in public db", guidStr);
        }
        else
        {
            std::string lineOriginal;
            LOG_CORE_WARN("GUID {0} not found in public db", guidStr);
            for (int i = 27; i > 18; i--)
            {

                // search in public db for similar
                mappingOK = FindGuidInFile("/text/sdl/gamecontrollerdb.txt", IDR_SD_LCTRL_DB, "TEXT", guidStr, i, &line);

                if (mappingOK)
                {
                    mappingOK = false; // found but loading could fail
                    // initialize controller with this line
                    lineOriginal = line;

                    // mapping string after 2nd comma
                    int pos = line.find(",");
                    append = line.substr(pos + 1, line.length() - pos - 1);
                    pos = append.find(",");
                    append = append.substr(pos + 1, append.length() - pos - 1);

                    if (name.length() > 45)
                        name = name.substr(0, 45);

                    // assemble final entry
                    std::string entry = guidStr;
                    entry += "," + name + "," + append;

                    if (AddControllerToInternalDB(entry))
                    {
                        RemoveDuplicatesInDB();

                        int ret = SDL_GameControllerAddMappingsFromFile(m_InternalDB.c_str());
                        if (ret == -1)
                        {
                            LOG_CORE_WARN("Warning: Unable to open '{0}' ", m_InternalDB);
                        }
                        else
                        {
                            mappingOK = true; // now ok
                            // reset SDL
                            // closeAllJoy();
                            // SDL_QuitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
                            // initJoy();
                        }
                    }

                    break;
                }
            }
            if (mappingOK)
            {
                LOG_CORE_WARN("{0}: trying to load mapping from closest match: {1}", guidStr, lineOriginal);
            }
        }
        return mappingOK;
    }

    bool Controller::AddControllerToInternalDB(std::string entry)
    {
        bool ok = false;
        std::string line;
        std::string filename = m_InternalDB;
        std::vector<std::string> internal_db_entries;

        std::ifstream internal_db_input_filehandle(filename);

        if (internal_db_input_filehandle.is_open())
        {
            while (getline(internal_db_input_filehandle, line))
            {
                internal_db_entries.push_back(line);
            }
            internal_db_input_filehandle.close();
        }
        else
        {
            LOG_CORE_INFO("Creating internal game controller database {0}", filename);
        }

        std::ofstream internal_db_output_filehandle;
        internal_db_output_filehandle.open(filename.c_str(), std::ios_base::out);
        if (internal_db_output_filehandle.fail())
        {
            LOG_CORE_WARN("Could not write internal game controller database: {0}, no entry added\n", filename);
        }
        else
        {
            internal_db_output_filehandle << entry << +"\n";
            for (size_t i = 0; i < internal_db_entries.size(); i++)
            {
                internal_db_output_filehandle << internal_db_entries[i] << +"\n";
            }
            internal_db_output_filehandle.close();
            ok = true;
        }
        return ok;
    }

    void Controller::RemoveDuplicatesInDB(void)
    {
        // If duplicate GUIDs are found,
        // this function keeps only the 1st encounter.
        // This is why addControllerToInternalDB()
        // is inserting new entries at the beginning.
        std::string line, guidStr;
        std::vector<std::string> entryVec;
        std::vector<std::string> guidVec;
        std::string filename;
        bool found;

        filename = m_InternalDB;

        std::ifstream internalDB(filename);
        if (!internalDB.is_open())
        {
            LOG_CORE_WARN("Could not open file: removeDuplicate(), file {0} \n", filename);
        }
        else
        {
            while (getline(internalDB, line))
            {
                found = false;
                if (line.find(",") != std::string::npos)
                {
                    guidStr = line.substr(0, line.find(","));
                    for (size_t i = 0; i < guidVec.size(); i++)
                    {
                        if (guidVec[i] == guidStr)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found)
                {
                    guidVec.push_back(guidStr);
                    entryVec.push_back(line);
                }
            }

            internalDB.close();

            std::ofstream internal_db_output_filehandle;
            internal_db_output_filehandle.open(filename.c_str(), std::ios_base::out);
            if (internal_db_output_filehandle.fail())
            {
                LOG_CORE_WARN("Could not write internal game controller database: {0}, no entry added\n", filename);
            }
            else
            {
                for (size_t i = 0; i < entryVec.size(); i++)
                {
                    internal_db_output_filehandle << entryVec[i] << +"\n";
                }
                internal_db_output_filehandle.close();
            }
        }
    }

    bool Controller::FindGuidInFile(std::string& filename, char* text2match, int length, std::string* lineRet)
    {
        const char* file = filename.c_str();
        bool ok = false;
        std::string line;
        std::string guidStr = text2match;
        std::string text = guidStr.substr(0, length);

        *lineRet = "";

        std::ifstream fileHandle(file);
        if (!fileHandle.is_open())
        {
            LOG_CORE_INFO("Could not open file: findGuidInFile({0},{1},{2})", filename, text2match, length);
        }
        else
        {
            while (getline(fileHandle, line) && !ok)
            {
                if (line.find(text.c_str()) == 0)
                {
                    // found
                    ok = true;
                    *lineRet = line;
                }
            }
            fileHandle.close();
        }

        return ok;
    }

    bool Controller::FindGuidInFile(const char* path /* GNU */, int resourceID /* MSVC */,
                                    const std::string& resourceClass /* MSVC */, char* text2match, int length,
                                    std::string* lineRet)
    {
        bool ok = false;
        std::string line;
        std::string guidStr = text2match;
        std::string text = guidStr.substr(0, length);

        *lineRet = "";

        memoryStream controllerDataBase{"/text/sdl/gamecontrollerdb.txt", IDR_SD_LCTRL_DB, "TEXT"};

        while (getline(controllerDataBase, line) && !ok)
        {
            if (line.find(text.c_str()) == 0)
            {
                // found
                ok = true;
                *lineRet = line;
            }
        }

        return ok;
    }

    void Controller::GetGUID(int controllerID, std::string& guid)
    {
        char guidStr[64];
        SDL_JoystickGUID guidSDL;
        SDL_Joystick* joy = nullptr;

        for (auto controller = m_Controllers.begin(); controller != m_Controllers.end(); controller++)
        {
            if (controller->m_IndexID == controllerID)
            {
                joy = controller->m_Joystick;
                break;
            }
        }

        guidSDL = SDL_JoystickGetGUID(joy);
        SDL_JoystickGetGUIDString(guidSDL, guidStr, sizeof(guidStr));

        guid = std::string(guidStr);
    }

    std::string Controller::GetName(int controllerID)
    {
        for (auto controller = m_Controllers.begin(); controller != m_Controllers.end(); controller++)
        {
            if (controller->m_IndexID == controllerID)
            {
                return controller->m_Name;
            }
        }
        return "";
    }

    Controller::ControllerData::ControllerData()
        : m_Joystick(nullptr), m_GameController(nullptr), m_InstanceID(-1), m_IndexID(-1), m_Name(""), m_NameDB(""),
          m_MappingOK(false)
    {
    }

    Controller::ControllerData::~ControllerData()
    {
        if (m_Joystick)
        {
            LOG_CORE_INFO("Removing controller index: {0}, instance: {1}, name: {2}, name in gamecontrollerdb.txt: {3}",
                          m_IndexID, m_InstanceID, m_Name, m_NameDB);
            SDL_JoystickClose(m_Joystick);
        }
    }
} // namespace GfxRenderEngine
