/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
   https://github.com/beaumanvienna/vulkan

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

#pragma once

#include <iostream>
#include <set>

#include "gui/Common/UI/screen.h"
#include "gui/Common/UI/view.h"
#include "gui/Common/UI/viewGroup.h"

namespace GfxRenderEngine
{

    class SCREEN_I18NCategory;
    namespace SCREEN_Draw
    {
        class SCREEN_DrawContext;
    }

    class SCREEN_UIScreen : public SCREEN_Screen
    {
    public:
        SCREEN_UIScreen();
        ~SCREEN_UIScreen();

        void update() override;
        void preRender() override;
        void render() override;
        void postRender() override;
        void deviceLost() override;
        void deviceRestored() override;

        bool touch(const SCREEN_TouchInput& touch) override;
        bool key(const SCREEN_KeyInput& touch) override;
        bool axis(const SCREEN_AxisInput& touch) override;

        SCREEN_TouchInput transformTouch(const SCREEN_TouchInput& touch) override;

        virtual void TriggerFinish(DialogResult result);

        // Some useful default event handlers
        SCREEN_UI::EventReturn OnOK(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OnCancel(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OnBack(SCREEN_UI::EventParams& e);

    protected:
        virtual void CreateViews() = 0;
        virtual void DrawBackground(SCREEN_UIContext& dc) {}

        virtual void RecreateViews() override { recreateViews_ = true; }

        SCREEN_UI::ViewGroup* root_ = nullptr;
        glm::vec3 translation_ = glm::vec3(0.0f);
        glm::vec3 scale_ = glm::vec3(1.0f);
        float alpha_ = 1.0f;
        bool ignoreInsets_ = false;

        float m_ContextWidth;
        float m_ContextHeight;

    private:
        void DoRecreateViews();

        bool recreateViews_ = true;
    };

    class SCREEN_UIDialogScreen : public SCREEN_UIScreen
    {
    public:
        SCREEN_UIDialogScreen() : SCREEN_UIScreen(), finished_(false) {}

        bool key(const SCREEN_KeyInput& key) override;
        void sendMessage(const char* msg, const char* value) override;

    private:
        bool finished_;
    };

    class SCREEN_PopupMultiChoice : public SCREEN_UI::Choice
    {
    public:
        SCREEN_PopupMultiChoice(int* value, const std::string& text, const char** choices, int minVal, int numChoices,
                                const char* category, SCREEN_ScreenManager* screenManager,
                                SCREEN_UI::LayoutParams* layoutParams = nullptr, float popupWidth = 800.0f)
            : SCREEN_UI::Choice(text, "", false, layoutParams), value_(value), choices_(choices), minVal_(minVal),
              numChoices_(numChoices), category_(category), screenManager_(screenManager), m_PopupWidth(popupWidth)
        {
            if (*value >= numChoices + minVal)
            {
                *value = numChoices + minVal - 1;
            }
            if (*value < minVal)
            {
                *value = minVal;
            }
            OnClick.Handle(this, &SCREEN_PopupMultiChoice::HandleClick);
            UpdateText();
        }

        virtual void Draw(SCREEN_UIContext& dc) override;
        virtual void Update() override;

        void HideChoice(int c) { hidden_.insert(c); }

        SCREEN_UI::Event OnChoice;

    protected:
        int* value_;
        const char** choices_;
        int minVal_;
        int numChoices_;
        void UpdateText();

    private:
        SCREEN_UI::EventReturn HandleClick(SCREEN_UI::EventParams& e);

        void ChoiceCallback(int num);
        virtual void PostChoiceCallback(int num) {}

        const char* category_;
        SCREEN_ScreenManager* screenManager_;
        std::string valueText_;
        bool restoreFocus_ = false;
        std::set<int> hidden_;
        float m_PopupWidth;
    };

    class SCREEN_PopupScreen : public SCREEN_UIDialogScreen
    {
    public:
        SCREEN_PopupScreen(std::string title, std::string button1 = "", std::string button2 = "",
                           float customWidth = 530.0f);

        virtual void CreatePopupContents(SCREEN_UI::ViewGroup* parent) = 0;
        virtual void CreateViews() override;
        virtual bool isTransparent() const override { return true; }
        virtual bool touch(const SCREEN_TouchInput& touch) override;
        virtual bool key(const SCREEN_KeyInput& key) override;
        virtual void resized() override;

        virtual void TriggerFinish(DialogResult result) override;

        void SetPopupOrigin(const SCREEN_UI::View* view);

    protected:
        virtual bool FillVertical() const { return false; }
        virtual SCREEN_UI::Size PopupWidth() const { return customWidth_; }
        virtual bool ShowButtons() const { return true; }
        virtual bool CanComplete(DialogResult result) { return true; }
        virtual void OnCompleted(DialogResult result) {}

        virtual void update() override;
        void SetTitleField(const std::string& title);

    private:
        SCREEN_UI::Choice* m_TitleField;
        std::string m_Title;
        SCREEN_UI::ViewGroup* box_;
        SCREEN_UI::Button* defaultButton_;
        std::string button1_;
        std::string button2_;
        float customWidth_;
        enum
        {
            FRAMES_LEAD_IN = 6,
            FRAMES_LEAD_OUT = 4,
        };

        int frames_ = 0;
        int finishFrame_ = -1;
        DialogResult finishResult_;
        bool hasPopupOrigin_ = false;
        Point popupOrigin_;
    };

    class ListSCREEN_PopupScreen : public SCREEN_PopupScreen
    {
    public:
        ListSCREEN_PopupScreen(std::string title) : SCREEN_PopupScreen(title) {}
        ListSCREEN_PopupScreen(std::string title, const std::vector<std::string>& items, int selected,
                               std::function<void(int)> callback, bool showButtons = false, float customWidth = 410)
            : SCREEN_PopupScreen(title, "OK", "Cancel", customWidth), adaptor_(items, selected), callback_(callback),
              showButtons_(showButtons), m_PopupWidth(customWidth)
        {
        }
        ListSCREEN_PopupScreen(std::string title, const std::vector<std::string>& items, int selected,
                               bool showButtons = false)
            : SCREEN_PopupScreen(title, "OK", "Cancel"), adaptor_(items, selected), showButtons_(showButtons)
        {
        }

        int GetChoice() const { return listView_->GetSelected(); }
        std::string GetChoiceString() const { return adaptor_.GetTitle(listView_->GetSelected()); }
        void SetHiddenChoices(std::set<int> hidden) { hidden_ = hidden; }
        virtual std::string tag() const override { return std::string("listpopup"); }

        SCREEN_UI::Event OnChoice;

    protected:
        virtual bool FillVertical() const override { return false; }
        virtual bool ShowButtons() const override { return showButtons_; }
        virtual void CreatePopupContents(SCREEN_UI::ViewGroup* parent) override;
        SCREEN_UI::StringVectorListAdaptor adaptor_;
        SCREEN_UI::ListView* listView_ = nullptr;
        float m_PopupWidth;

    private:
        SCREEN_UI::EventReturn OnListChoice(SCREEN_UI::EventParams& e);

        std::function<void(int)> callback_;
        bool showButtons_ = false;
        std::set<int> hidden_;
    };

    class SCREEN_PopupSliderChoice : public SCREEN_UI::Choice
    {
    public:
        SCREEN_PopupSliderChoice(int* value, int minValue, int maxValue, const std::string& text,
                                 const std::string& units = "", SCREEN_UI::LayoutParams* layoutParams = 0);
        SCREEN_PopupSliderChoice(int* value, int minValue, int maxValue, const std::string& text, int step,
                                 const std::string& units = "", SCREEN_UI::LayoutParams* layoutParams = 0);

        virtual void Draw(SCREEN_UIContext& dc) override;

        void SetFormat(const char* fmt) { fmt_ = fmt; }
        void SetZeroLabel(const std::string& str) { zeroLabel_ = str; }
        void SetNegativeDisable(const std::string& str) { negativeLabel_ = str; }

        SCREEN_UI::Event OnChange;

    private:
        SCREEN_UI::EventReturn HandleClick(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn HandleChange(SCREEN_UI::EventParams& e);

        int* value_;
        int minValue_;
        int maxValue_;
        int step_;
        const char* fmt_;
        std::string zeroLabel_;
        std::string negativeLabel_;
        std::string units_;
        bool restoreFocus_;
    };

    class SliderSCREEN_PopupScreen : public SCREEN_PopupScreen
    {
    public:
        SliderSCREEN_PopupScreen(int* value, int minValue, int maxValue, const std::string& title, int step = 1,
                                 const std::string& units = "")
            : SCREEN_PopupScreen(title, "OK", "Cancel"), units_(units), value_(value), minValue_(minValue),
              maxValue_(maxValue), step_(step)
        {
        }
        virtual void CreatePopupContents(SCREEN_UI::ViewGroup* parent) override;

        void SetNegativeDisable(const std::string& str);

        SCREEN_UI::Event OnChange;

    private:
        SCREEN_UI::EventReturn OnDecrease(SCREEN_UI::EventParams& params);
        SCREEN_UI::EventReturn OnIncrease(SCREEN_UI::EventParams& params);
        SCREEN_UI::EventReturn OnSliderChange(SCREEN_UI::EventParams& params);
        virtual void OnCompleted(DialogResult result) override;
        SCREEN_UI::Slider* slider_ = nullptr;
        std::string units_;
        std::string negativeLabel_;
        int* value_;
        int sliderValue_ = 0;
        int minValue_;
        int maxValue_;
        int step_;
        bool changing_ = false;
        bool disabled_ = false;
    };

    class SCREEN_PopupMultiChoiceDynamic : public SCREEN_PopupMultiChoice
    {
    public:
        SCREEN_PopupMultiChoiceDynamic(std::string* value, const std::string& text, std::vector<std::string>& choices,
                                       const char* category, SCREEN_ScreenManager* screenManager,
                                       SCREEN_UI::LayoutParams* layoutParams = nullptr, float popupWidth = 800.0f)
            : SCREEN_PopupMultiChoice(&valueInt_, text, nullptr, 0, (int)choices.size(), category, screenManager,
                                      layoutParams, popupWidth),
              valueStr_(value)
        {
            choices_ = new const char*[numChoices_];
            valueInt_ = 0;
            for (int i = 0; i < numChoices_; i++)
            {
                const uint MAX_STRING_LENGTH = 60;
                std::string choice = choices[i].substr(0, MAX_STRING_LENGTH);

                choices_[i] = new char[choice.size() + 1];
                memcpy((char*)choices_[i], choice.c_str(), choice.size() + 1);

                if ((*value).substr(0, MAX_STRING_LENGTH) == choices_[i])
                {
                    valueInt_ = i;
                }
            }
            value_ = &valueInt_;
            UpdateText();
        }
        ~SCREEN_PopupMultiChoiceDynamic()
        {
            for (int i = 0; i < numChoices_; i++)
            {
                delete[] choices_[i];
            }
            delete[] choices_;
        }

    protected:
        void PostChoiceCallback(int num) { *valueStr_ = choices_[num]; }

    private:
        int valueInt_;
        std::string* valueStr_;
    };
} // namespace GfxRenderEngine
