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

#include "core.h"
#include "gui/common.h"
#include "gui/Common/UI/root.h"
#include "gui/Common/UI/UIscreen.h"
#include "gui/Common/UI/context.h"
#include "gui/Common/Input/inputState.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/Data/Text/i18n.h"
#include "gui/Common/Math/curves.h"

namespace GfxRenderEngine
{

    SCREEN_UIScreen::SCREEN_UIScreen() : SCREEN_Screen() {}

    SCREEN_UIScreen::~SCREEN_UIScreen() { delete root_; }

    void SCREEN_UIScreen::DoRecreateViews()
    {

        std::lock_guard<std::recursive_mutex> guard(screenManager()->inputLock_);

        if (recreateViews_)
        {
            m_ContextWidth = Engine::m_Engine->GetWindowWidth();
            m_ContextHeight = Engine::m_Engine->GetWindowHeight();

            SCREEN_UI::PersistMap persisted;
            bool persisting = root_ != nullptr;
            if (persisting)
            {
                root_->PersistData(SCREEN_UI::PERSIST_SAVE, "root", persisted);
            }

            delete root_;
            root_ = nullptr;
            CreateViews();
            SCREEN_UI::View* defaultView = root_ ? root_->GetDefaultFocusView() : nullptr;
            if (defaultView && defaultView->GetVisibility() == SCREEN_UI::V_VISIBLE)
            {
                defaultView->SetFocus();
            }
            recreateViews_ = false;

            if (persisting && root_ != nullptr)
            {
                root_->PersistData(SCREEN_UI::PERSIST_RESTORE, "root", persisted);

                SCREEN_UI::LayoutViewHierarchy(*screenManager()->getUIContext(), root_, ignoreInsets_);
                SCREEN_UI::View* focused = SCREEN_UI::GetFocusedView();
                if (focused)
                {
                    root_->SubviewFocused(focused);
                }
            }
        }
    }

    void SCREEN_UIScreen::update()
    {
        DoRecreateViews();

        if (root_)
        {
            UpdateViewHierarchy(root_);
        }
    }
    //
    void SCREEN_UIScreen::deviceLost()
    {
        if (root_)
            root_->DeviceLost();
    }

    void SCREEN_UIScreen::deviceRestored()
    {
        if (root_)
        {
            root_->DeviceRestored(screenManager()->getSCREEN_DrawContext());
        }
    }

    void SCREEN_UIScreen::preRender() {}

    void SCREEN_UIScreen::postRender() {}

    void SCREEN_UIScreen::render()
    {
        DoRecreateViews();

        if (root_)
        {
            SCREEN_UIContext* uiContext = screenManager()->getUIContext();
            SCREEN_UI::LayoutViewHierarchy(*uiContext, root_, ignoreInsets_);
            root_->Draw(*uiContext);
        }
    }

    SCREEN_TouchInput SCREEN_UIScreen::transformTouch(const SCREEN_TouchInput& touch)
    {
        SCREEN_TouchInput updated = touch;

        float x = touch.x - translation_.x;
        float y = touch.y - translation_.y;

        updated.x = x / scale_.x;
        updated.y = y / scale_.y;

        return updated;
    }

    bool SCREEN_UIScreen::touch(const SCREEN_TouchInput& touch)
    {
        bool clicked = false;
        if (root_)
        {
            return root_->Touch(touch);
        }
        return clicked;
    }

    bool SCREEN_UIScreen::key(const SCREEN_KeyInput& key)
    {
        if (root_)
        {
            return SCREEN_UI::KeyEvent(key, root_);
        }
        return false;
    }

    void SCREEN_UIScreen::TriggerFinish(DialogResult result) { screenManager()->finishDialog(this, result); }

    bool SCREEN_UIDialogScreen::key(const SCREEN_KeyInput& key)
    {
        bool retval = SCREEN_UIScreen::key(key);
        if (!retval && (key.flags & KEY_DOWN) && SCREEN_UI::IsEscapeKey(key))
        {
            if (finished_)
            {
                LOG_CORE_WARN("Screen already finished");
            }
            else
            {
                finished_ = true;
                TriggerFinish(DR_BACK);
            }
            return true;
        }
        return retval;
    }

    void SCREEN_UIDialogScreen::sendMessage(const char* msg, const char* value)
    {
        SCREEN_Screen* screen = screenManager()->dialogParent(this);
        if (screen)
        {
            screen->sendMessage(msg, value);
        }
    }

    bool SCREEN_UIScreen::axis(const SCREEN_AxisInput& axis)
    {
        if (root_)
        {
            SCREEN_UI::AxisEvent(axis, root_);
            return true;
        }
        return false;
    }

    SCREEN_UI::EventReturn SCREEN_UIScreen::OnBack(SCREEN_UI::EventParams& e)
    {
        TriggerFinish(DR_BACK);
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SCREEN_UIScreen::OnOK(SCREEN_UI::EventParams& e)
    {
        TriggerFinish(DR_OK);
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SCREEN_UIScreen::OnCancel(SCREEN_UI::EventParams& e)
    {
        TriggerFinish(DR_CANCEL);
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_PopupScreen::SCREEN_PopupScreen(std::string title, std::string button1, std::string button2, float customWidth)
        : box_(0), defaultButton_(nullptr), m_Title(title), customWidth_(customWidth)
    {
        auto di = GetI18NCategory("Dialog");
        if (!button1.empty())
        {
            button1_ = di->T(button1.c_str());
        }
        if (!button2.empty())
        {
            button2_ = di->T(button2.c_str());
        }

        alpha_ = 0.0f;
    }

    bool SCREEN_PopupScreen::touch(const SCREEN_TouchInput& touch)
    {
        if (!box_ || (touch.flags & TOUCH_DOWN) == 0 || touch.id != 0)
        {
            return SCREEN_UIDialogScreen::touch(touch);
        }

        if (!box_->GetBounds().Contains(touch.x, touch.y))
        {
            TriggerFinish(DR_BACK);
        }

        return SCREEN_UIDialogScreen::touch(touch);
    }

    bool SCREEN_PopupScreen::key(const SCREEN_KeyInput& key)
    {
        if (key.flags & KEY_DOWN)
        {
            if (key.keyCode == NKCODE_ENTER && defaultButton_)
            {
                SCREEN_UI::EventParams e{};
                defaultButton_->OnClick.Trigger(e);
                return true;
            }
        }

        return SCREEN_UIDialogScreen::key(key);
    }

    void SCREEN_PopupScreen::update()
    {
        SCREEN_UIDialogScreen::update();

        if (defaultButton_)
        {
            defaultButton_->SetEnabled(CanComplete(DR_OK));
        }

        float animatePos = 1.0f;

        ++frames_;
        if (finishFrame_ >= 0)
        {
            float leadOut = bezierEaseInOut((frames_ - finishFrame_) * (1.0f / (float)FRAMES_LEAD_OUT));
            animatePos = 1.0f - leadOut;

            if (frames_ >= finishFrame_ + FRAMES_LEAD_OUT)
            {
                // Actual finish happens here.
                screenManager()->finishDialog(this, finishResult_);
            }
        }
        else if (frames_ < FRAMES_LEAD_IN)
        {
            float leadIn = bezierEaseInOut(frames_ * (1.0f / (float)FRAMES_LEAD_IN));
            animatePos = leadIn;
        }

        if (animatePos < 1.0f)
        {
            alpha_ = animatePos;
            scale_.x = 0.9f + animatePos * 0.1f;
            scale_.y = 0.9f + animatePos * 0.1f;

            if (hasPopupOrigin_)
            {
                float xoff = popupOrigin_.x;
                float yoff = popupOrigin_.y;

                // Pull toward the origin a bit.
                translation_.x = xoff * (1.0f - animatePos) * 0.2f;
                translation_.y = yoff * (1.0f - animatePos) * 0.2f;
            }
            else
            {
                translation_.y = -m_ContextHeight * (1.0f - animatePos) * 0.2f;
            }
        }
        else
        {
            alpha_ = 1.0f;
            scale_.x = 1.0f;
            scale_.y = 1.0f;
            translation_.x = 0.0f;
            translation_.y = 0.0f;
        }
    }

    void SCREEN_PopupScreen::SetPopupOrigin(const SCREEN_UI::View* view)
    {
        hasPopupOrigin_ = true;
        popupOrigin_ = view->GetBounds().Center();
    }

    void SCREEN_PopupScreen::TriggerFinish(DialogResult result)
    {
        if (CanComplete(result))
        {
            finishFrame_ = frames_;
            finishResult_ = result;

            OnCompleted(result);
        }
    }

    void SCREEN_PopupScreen::resized() { RecreateViews(); }

    void SCREEN_PopupScreen::CreateViews()
    {
        using namespace SCREEN_UI;
        SCREEN_UIContext& dc = *screenManager()->getUIContext();

        AnchorLayout* anchor = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
        anchor->Overflow(false);
        root_ = anchor;

        float yres = screenManager()->getUIContext()->GetBounds().h;
        float scaleWidth = Engine::m_Engine->GetWindowHeight() / 1080.0f;
        box_ = new LinearLayout(
            ORIENT_VERTICAL, new AnchorLayoutParams(PopupWidth() * scaleWidth,
                                                    FillVertical() ? yres - 30.0f : static_cast<float>(WRAP_CONTENT),
                                                    dc.GetBounds().centerX(), dc.GetBounds().centerY(), NONE, NONE, true));

        root_->Add(box_);
        box_->SetBG(dc.theme->popupStyle.background);
        box_->SetHasDropShadow(true);

        box_->SetDropShadowExpand(std::max(m_ContextWidth, m_ContextHeight));

#define TRANSPARENT_BACKGROUND true
        m_TitleField = new Choice(m_Title, TRANSPARENT_BACKGROUND, new LayoutParams(530.0f, 64.0f));
        m_TitleField->SetFocusable(false);
        m_TitleField->SetCentered(true);
        box_->Add(m_TitleField);

        View* separator = new Separator();
        box_->Add(separator);

        CreatePopupContents(box_);
        root_->SetDefaultFocusView(box_);

        if (ShowButtons() && !button1_.empty())
        {
            LinearLayout* buttonRow =
                new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
            float scale = Engine::m_Engine->GetWindowHeight() / 1080.0f;
            Choice* okButton = new Choice(button1_, TRANSPARENT_BACKGROUND, new LayoutParams(265.0f * scale, 64.0f * scale));
            Choice* cancelButton =
                new Choice(button2_, TRANSPARENT_BACKGROUND, new LayoutParams(265.0f * scale, 64.0f * scale));

            okButton->SetCentered(true);
            cancelButton->SetCentered(true);

            okButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnOK);
            cancelButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnCancel);

            buttonRow->Add(okButton);
            buttonRow->Add(cancelButton);
            box_->Add(buttonRow);
        }
    }
    void SCREEN_PopupScreen::SetTitleField(const std::string& title)
    {
        m_Title = title;
        m_TitleField->SetText(title);
    }

    // void MessageSCREEN_PopupScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent)
    //{
    //     using namespace SCREEN_UI;
    //     SCREEN_UIContext &dc = *screenManager()->getUIContext();
    //
    //     std::vector<std::string> messageLines;
    //     SCREEN_PSplitString(message_, '\n', messageLines);
    //     for (const auto& lineOfText : messageLines)
    //     {
    //         parent->Add(new SCREEN_UI::TextView(lineOfText, ALIGN_LEFT | ALIGN_VCENTER,
    //         false))->SetTextColor(dc.theme->popupStyle.fgColor);
    //     }
    // }
    //
    // void MessageSCREEN_PopupScreen::OnCompleted(DialogResult result)
    //{
    //     if (result == DR_OK)
    //     {
    //         if (callback_)
    //         {
    //             callback_(true);
    //         }
    //     }
    //     else
    //     {
    //         if (callback_)
    //         {
    //             callback_(false);
    //         }
    //     }
    // }

    void ListSCREEN_PopupScreen::CreatePopupContents(SCREEN_UI::ViewGroup* parent)
    {
        using namespace SCREEN_UI;

        listView_ = parent->Add(new ListView(&adaptor_, m_PopupWidth, hidden_));
        listView_->SetMaxHeight(screenManager()->getUIContext()->GetBounds().h - 140.0f);
        listView_->OnChoice.Handle(this, &ListSCREEN_PopupScreen::OnListChoice);
    }

    SCREEN_UI::EventReturn ListSCREEN_PopupScreen::OnListChoice(SCREEN_UI::EventParams& e)
    {
        adaptor_.SetSelected(e.a);
        if (callback_)
        {
            callback_(adaptor_.GetSelected());
        }
        TriggerFinish(DR_OK);
        OnChoice.Dispatch(e);
        return SCREEN_UI::EVENT_DONE;
    }

    // namespace SCREEN_UI
    //{
    std::string ChopTitle(const std::string& title)
    {
        size_t pos = title.find('\n');
        if (pos != title.npos)
        {
            return title.substr(0, pos);
        }
        return title;
    }

    SCREEN_UI::EventReturn SCREEN_PopupMultiChoice::HandleClick(SCREEN_UI::EventParams& e)
    {
        restoreFocus_ = HasFocus();

        auto category = category_ ? GetI18NCategory(category_) : nullptr;

        std::vector<std::string> choices;
        for (int i = 0; i < numChoices_; i++)
        {
            choices.push_back(category ? category->T(choices_[i]) : choices_[i]);
        }

        ListSCREEN_PopupScreen* popupScreen = new ListSCREEN_PopupScreen(
            ChopTitle(text_), choices, *value_ - minVal_,
            std::bind(&SCREEN_PopupMultiChoice::ChoiceCallback, this, std::placeholders::_1), false, m_PopupWidth);
        popupScreen->SetHiddenChoices(hidden_);
        if (e.v)
        {
            popupScreen->SetPopupOrigin(e.v);
        }
        screenManager_->push(popupScreen);
        return SCREEN_UI::EVENT_DONE;
    }

    void SCREEN_PopupMultiChoice::Update() { UpdateText(); }

    void SCREEN_PopupMultiChoice::UpdateText()
    {
        if (!choices_)
        {
            return;
        }
        auto category = GetI18NCategory(category_);

        if (*value_ < minVal_ || *value_ > minVal_ + numChoices_ - 1)
        {
            valueText_ = "(invalid choice)";
        }
        else
        {
            valueText_ = category ? category->T(choices_[*value_ - minVal_]) : choices_[*value_ - minVal_];
        }
    }

    void SCREEN_PopupMultiChoice::ChoiceCallback(int num)
    {
        if (num != -1)
        {
            *value_ = num + minVal_;
            UpdateText();

            SCREEN_UI::EventParams e{};
            e.v = this;
            e.a = num;
            OnChoice.Trigger(e);

            if (restoreFocus_)
            {
                SetFocusedView(this);
            }
            PostChoiceCallback(num);
        }
    }

    void SCREEN_PopupMultiChoice::Draw(SCREEN_UIContext& dc)
    {
        SCREEN_UI::Style style = dc.theme->itemStyle;
        if (!IsEnabled())
        {
            style = dc.theme->itemDisabledStyle;
        }
        int paddingX = 12;
        dc.SetFontStyle(dc.theme->uiFont);

        float ignore;
        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, valueText_.c_str(), &textPadding_.right, &ignore,
                       ALIGN_RIGHT | ALIGN_VCENTER);
        textPadding_.right += paddingX;

        Choice::Draw(dc);
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            dc.DrawText(valueText_.c_str(), bounds_.x2() - paddingX + 2, bounds_.centerY() + 2, RETRO_COLOR_FONT_BACKGROUND,
                        ALIGN_RIGHT | ALIGN_VCENTER);
        }
        dc.DrawText(valueText_.c_str(), bounds_.x2() - paddingX, bounds_.centerY(), style.fgColor,
                    ALIGN_RIGHT | ALIGN_VCENTER);
    }

    SCREEN_PopupSliderChoice::SCREEN_PopupSliderChoice(int* value, int minValue, int maxValue, const std::string& text,
                                                       const std::string& units, SCREEN_UI::LayoutParams* layoutParams)
        : Choice(text, "", false, layoutParams), value_(value), minValue_(minValue), maxValue_(maxValue), step_(1),
          units_(units)
    {
        fmt_ = "%i";
        OnClick.Handle(this, &SCREEN_PopupSliderChoice::HandleClick);
    }

    SCREEN_PopupSliderChoice::SCREEN_PopupSliderChoice(int* value, int minValue, int maxValue, const std::string& text,
                                                       int step, const std::string& units,
                                                       SCREEN_UI::LayoutParams* layoutParams)
        : Choice(text, "", false, layoutParams), value_(value), minValue_(minValue), maxValue_(maxValue), step_(step),
          units_(units)
    {
        fmt_ = "%i";
        OnClick.Handle(this, &SCREEN_PopupSliderChoice::HandleClick);
    }

    SCREEN_UI::EventReturn SCREEN_PopupSliderChoice::HandleClick(SCREEN_UI::EventParams& e)
    {
        restoreFocus_ = HasFocus();

        SliderSCREEN_PopupScreen* popupScreen =
            new SliderSCREEN_PopupScreen(value_, minValue_, maxValue_, ChopTitle(text_), step_, units_);
        if (!negativeLabel_.empty())
        {
            popupScreen->SetNegativeDisable(negativeLabel_);
        }
        popupScreen->OnChange.Handle(this, &SCREEN_PopupSliderChoice::HandleChange);
        if (e.v)
        {
            popupScreen->SetPopupOrigin(e.v);
        }

        SCREEN_ScreenManager::m_ScreenManager->push(popupScreen);
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SCREEN_PopupSliderChoice::HandleChange(SCREEN_UI::EventParams& e)
    {
        e.v = this;
        OnChange.Trigger(e);

        if (restoreFocus_)
        {
            SetFocusedView(this);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    void SCREEN_PopupSliderChoice::Draw(SCREEN_UIContext& dc)
    {
        SCREEN_UI::Style style = dc.theme->itemStyle;
        if (!IsEnabled())
        {
            style = dc.theme->itemDisabledStyle;
        }
        int paddingX = 12;
        dc.SetFontStyle(dc.theme->uiFont);

        char temp[256];
        if (zeroLabel_.size() && *value_ == 0)
        {
            strcpy(temp, zeroLabel_.c_str());
        }
        else if (negativeLabel_.size() && *value_ < 0)
        {
            strcpy(temp, negativeLabel_.c_str());
        }
        else
        {
            sprintf(temp, fmt_, *value_);
        }

        float ignore;
        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, temp, &textPadding_.right, &ignore, ALIGN_RIGHT | ALIGN_VCENTER);
        textPadding_.right += paddingX;

        Choice::Draw(dc);
        dc.DrawText(temp, bounds_.x2() - paddingX, bounds_.centerY(), style.fgColor, ALIGN_RIGHT | ALIGN_VCENTER);
    }

    //    SCREEN_UI::EventReturn SCREEN_PopupSliderChoiceFloat::HandleClick(SCREEN_UI::EventParams &e)
    //    {
    //        restoreFocus_ = HasFocus();
    //
    //        SliderFloatSCREEN_PopupScreen *popupScreen = new SliderFloatSCREEN_PopupScreen(value_, minValue_, maxValue_,
    //        ChopTitle(text_), step_, units_); popupScreen->OnChange.Handle(this,
    //        &SCREEN_PopupSliderChoiceFloat::HandleChange); if (e.v)
    //            popupScreen->SetPopupOrigin(e.v);
    //        screenManager_->push(popupScreen);
    //        return EVENT_DONE;
    //    }
    //
    //    SCREEN_UI::EventReturn SCREEN_PopupSliderChoiceFloat::HandleChange(SCREEN_UI::EventParams &e)
    //    {
    //        e.v = this;
    //        OnChange.Trigger(e);
    //
    //        if (restoreFocus_) {
    //            SetFocusedView(this);
    //        }
    //        return EVENT_DONE;
    //    }
    //
    //    void SCREEN_PopupSliderChoiceFloat::Draw(SCREEN_UIContext &dc)
    //    {
    //        Style style = dc.theme->itemStyle;
    //        if (!IsEnabled())
    //        {
    //            style = dc.theme->itemDisabledStyle;
    //        }
    //        int paddingX = 12;
    //        dc.SetFontStyle(dc.theme->uiFont);
    //
    //        char temp[256];
    //        if (zeroLabel_.size() && *value_ == 0.0f)
    //        {
    //            strcpy(temp, zeroLabel_.c_str());
    //        }
    //        else
    //        {
    //            sprintf(temp, fmt_, *value_);
    //        }
    //
    //        float ignore;
    //        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, temp, &textPadding_.right, &ignore, ALIGN_RIGHT | ALIGN_VCENTER);
    //        textPadding_.right += paddingX;
    //
    //        Choice::Draw(dc);
    //        dc.DrawText(temp, bounds_.x2() - paddingX, bounds_.centerY(), style.fgColor, ALIGN_RIGHT | ALIGN_VCENTER);
    //    }
    //

    void SliderSCREEN_PopupScreen::SetNegativeDisable(const std::string& str)
    {
        negativeLabel_ = str;
        disabled_ = *value_ < 0;
    }

    SCREEN_UI::EventReturn SliderSCREEN_PopupScreen::OnDecrease(SCREEN_UI::EventParams& params)
    {
        if (sliderValue_ > minValue_ && sliderValue_ < maxValue_)
        {
            sliderValue_ = step_ * floor((sliderValue_ / step_) + 0.5f);
        }
        sliderValue_ -= step_;
        slider_->Clamp();
        changing_ = true;
        char temp[64];
        sprintf(temp, "%d", sliderValue_);
        // edit_->SetText(temp);
        changing_ = false;
        disabled_ = false;
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SliderSCREEN_PopupScreen::OnIncrease(SCREEN_UI::EventParams& params)
    {
        if (sliderValue_ > minValue_ && sliderValue_ < maxValue_)
        {
            sliderValue_ = step_ * floor((sliderValue_ / step_) + 0.5f);
        }
        sliderValue_ += step_;
        slider_->Clamp();
        changing_ = true;
        char temp[64];
        sprintf(temp, "%d", sliderValue_);
        // edit_->SetText(temp);
        changing_ = false;
        disabled_ = false;
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SliderSCREEN_PopupScreen::OnSliderChange(SCREEN_UI::EventParams& params)
    {
        changing_ = true;
        char temp[64];
        sprintf(temp, "%d", sliderValue_);
        // edit_->SetText(temp);
        changing_ = false;
        disabled_ = false;
        return SCREEN_UI::EVENT_DONE;
    }

    // SCREEN_UI::EventReturn SliderSCREEN_PopupScreen::OnTextChange(SCREEN_UI::EventParams &params)
    //{
    //     if (!changing_)
    //     {
    //         sliderValue_ = atoi(edit_->GetText().c_str());
    //         disabled_ = false;
    //         slider_->Clamp();
    //     }
    //     return SCREEN_UI::EVENT_DONE;
    // }

    void SliderSCREEN_PopupScreen::CreatePopupContents(SCREEN_UI::ViewGroup* parent)
    {
        using namespace SCREEN_UI;
        // SCREEN_UIContext &dc = *screenManager()->getUIContext();

        sliderValue_ = *value_;
        if (disabled_ && sliderValue_ < 0)
        {
            sliderValue_ = 0;
        }
        LinearLayout* vert =
            parent->Add(new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(SCREEN_UI::Margins(10, 10))));
        slider_ = new Slider(&sliderValue_, minValue_, maxValue_, new LinearLayoutParams(SCREEN_UI::Margins(10, 10)));
        slider_->OnChange.Handle(this, &SliderSCREEN_PopupScreen::OnSliderChange);
        vert->Add(slider_);

        // LinearLayout *lin = vert->Add(new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(SCREEN_UI::Margins(10,
        // 10)))); lin->Add(new Button(" - "))->OnClick.Handle(this, &SliderSCREEN_PopupScreen::OnDecrease); lin->Add(new
        // Button(" + "))->OnClick.Handle(this, &SliderSCREEN_PopupScreen::OnIncrease);
        //
        // char temp[64];
        // sprintf(temp, "%d", sliderValue_);
        // edit_ = new TextEdit(temp, "", new LinearLayoutParams(10.0f));
        // edit_->SetMaxLen(16);
        // edit_->SetTextColor(dc.theme->popupStyle.fgColor);
        // edit_->SetTextAlign(FLAG_DYNAMIC_ASCII);
        // edit_->OnTextChange.Handle(this, &SliderSCREEN_PopupScreen::OnTextChange);
        // changing_ = false;
        // lin->Add(edit_);

        // if (!units_.empty())
        //{
        //     lin->Add(new TextView(units_, new LinearLayoutParams(10.0f)))->SetTextColor(dc.theme->popupStyle.fgColor);
        // }
        //
        // if (!negativeLabel_.empty())
        //{
        //     vert->Add(new CheckBox(&disabled_, negativeLabel_));
        // }

        if (IsFocusMovementEnabled())
        {
            SCREEN_UI::SetFocusedView(slider_);
        }
    }

    //    void SliderFloatSCREEN_PopupScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent)
    //    {
    //        using namespace SCREEN_UI;
    //        SCREEN_UIContext &dc = *screenManager()->getUIContext();
    //
    //        sliderValue_ = *value_;
    //        LinearLayout *vert = parent->Add(new LinearLayout(ORIENT_VERTICAL, new
    //        LinearLayoutParams(SCREEN_UI::Margins(10, 10)))); slider_ = new SliderFloat(&sliderValue_, minValue_,
    //        maxValue_, new LinearLayoutParams(SCREEN_UI::Margins(10, 10))); slider_->OnChange.Handle(this,
    //        &SliderFloatSCREEN_PopupScreen::OnSliderChange); vert->Add(slider_);
    //
    //        LinearLayout *lin = vert->Add(new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(SCREEN_UI::Margins(10,
    //        10)))); lin->Add(new Button(" - "))->OnClick.Handle(this, &SliderFloatSCREEN_PopupScreen::OnDecrease);
    //        lin->Add(new Button(" + "))->OnClick.Handle(this, &SliderFloatSCREEN_PopupScreen::OnIncrease);
    //
    //        char temp[64];
    //        sprintf(temp, "%0.3f", sliderValue_);
    //        edit_ = new TextEdit(temp, "", new LinearLayoutParams(10.0f));
    //        edit_->SetMaxLen(16);
    //        edit_->SetTextColor(dc.theme->popupStyle.fgColor);
    //        edit_->SetTextAlign(FLAG_DYNAMIC_ASCII);
    //        edit_->OnTextChange.Handle(this, &SliderFloatSCREEN_PopupScreen::OnTextChange);
    //        changing_ = false;
    //        lin->Add(edit_);
    //        if (!units_.empty())
    //            lin->Add(new TextView(units_, new LinearLayoutParams(10.0f)))->SetTextColor(dc.theme->popupStyle.fgColor);
    //
    //        // slider_ = parent->Add(new SliderFloat(&sliderValue_, minValue_, maxValue_, new
    //        LinearLayoutParams(SCREEN_UI::Margins(10, 5)))); if (IsFocusMovementEnabled())
    //        {
    //            SCREEN_UI::SetFocusedView(slider_);
    //        }
    //    }
    //
    //    SCREEN_UI::EventReturn SliderFloatSCREEN_PopupScreen::OnDecrease(SCREEN_UI::EventParams &params)
    //    {
    //        if (sliderValue_ > minValue_ && sliderValue_ < maxValue_) {
    //            sliderValue_ = step_ * floor((sliderValue_ / step_) + 0.5f);
    //        }
    //        sliderValue_ -= step_;
    //        slider_->Clamp();
    //        changing_ = true;
    //        char temp[64];
    //        sprintf(temp, "%0.3f", sliderValue_);
    //        edit_->SetText(temp);
    //        changing_ = false;
    //        return EVENT_DONE;
    //    }
    //
    //    SCREEN_UI::EventReturn SliderFloatSCREEN_PopupScreen::OnIncrease(SCREEN_UI::EventParams &params)
    //    {
    //        if (sliderValue_ > minValue_ && sliderValue_ < maxValue_) {
    //            sliderValue_ = step_ * floor((sliderValue_ / step_) + 0.5f);
    //        }
    //        sliderValue_ += step_;
    //        slider_->Clamp();
    //        changing_ = true;
    //        char temp[64];
    //        sprintf(temp, "%0.3f", sliderValue_);
    //        edit_->SetText(temp);
    //        changing_ = false;
    //        return EVENT_DONE;
    //    }
    //
    //    SCREEN_UI::EventReturn SliderFloatSCREEN_PopupScreen::OnSliderChange(SCREEN_UI::EventParams &params)
    //    {
    //        changing_ = true;
    //        char temp[64];
    //        sprintf(temp, "%0.3f", sliderValue_);
    //        edit_->SetText(temp);
    //        changing_ = false;
    //        return EVENT_DONE;
    //    }
    //
    //    SCREEN_UI::EventReturn SliderFloatSCREEN_PopupScreen::OnTextChange(SCREEN_UI::EventParams &params)
    //    {
    //        if (!changing_) {
    //            sliderValue_ = atof(edit_->GetText().c_str());
    //            slider_->Clamp();
    //        }
    //        return EVENT_DONE;
    //    }

    void SliderSCREEN_PopupScreen::OnCompleted(DialogResult result)
    {
        if (result == DR_OK)
        {
            *value_ = disabled_ ? -1 : sliderValue_;
            SCREEN_UI::EventParams e{};
            e.v = nullptr;
            e.a = *value_;
            OnChange.Trigger(e);
        }
    }

    //    void SliderFloatSCREEN_PopupScreen::OnCompleted(DialogResult result)
    //    {
    //        if (result == DR_OK)
    //        {
    //            *value_ = sliderValue_;
    //            SCREEN_UI::EventParams e{};
    //            e.v = nullptr;
    //            e.a = (int)*value_;
    //            e.f = *value_;
    //            OnChange.Trigger(e);
    //        }
    //    }
    //
    //    SCREEN_PopupTextInputChoice::SCREEN_PopupTextInputChoice(std::string *value, const std::string &title, const
    //    std::string &placeholder, int maxLen, SCREEN_ScreenManager *screenManager, LayoutParams *layoutParams) :
    //    Choice(title, "", false, layoutParams), screenManager_(screenManager), value_(value), placeHolder_(placeholder),
    //    maxLen_(maxLen)
    //    {
    //        OnClick.Handle(this, &SCREEN_PopupTextInputChoice::HandleClick);
    //    }
    //
    //    SCREEN_UI::EventReturn SCREEN_PopupTextInputChoice::HandleClick(SCREEN_UI::EventParams &e)
    //    {
    //        restoreFocus_ = HasFocus();
    //
    //        TextEditSCREEN_PopupScreen *popupScreen = new TextEditSCREEN_PopupScreen(value_, placeHolder_,
    //        ChopTitle(text_), maxLen_); popupScreen->OnChange.Handle(this, &SCREEN_PopupTextInputChoice::HandleChange); if
    //        (e.v)
    //        {
    //            popupScreen->SetPopupOrigin(e.v);
    //        }
    //        screenManager_->push(popupScreen);
    //        return EVENT_DONE;
    //    }
    //
    //    void SCREEN_PopupTextInputChoice::Draw(SCREEN_UIContext &dc)
    //    {
    //        Style style = dc.theme->itemStyle;
    //        if (!IsEnabled())
    //        {
    //            style = dc.theme->itemDisabledStyle;
    //        }
    //        int paddingX = 12;
    //        dc.SetFontStyle(dc.theme->uiFont);
    //
    //        float ignore;
    //        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, value_->c_str(), &textPadding_.right, &ignore, ALIGN_RIGHT |
    //        ALIGN_VCENTER); textPadding_.right += paddingX;
    //
    //        Choice::Draw(dc);
    //        dc.DrawText(value_->c_str(), bounds_.x2() - 12, bounds_.centerY(), style.fgColor, ALIGN_RIGHT | ALIGN_VCENTER);
    //    }
    //
    //    SCREEN_UI::EventReturn SCREEN_PopupTextInputChoice::HandleChange(SCREEN_UI::EventParams &e)
    //    {
    //        e.v = this;
    //        OnChange.Trigger(e);
    //
    //        if (restoreFocus_)
    //        {
    //            SetFocusedView(this);
    //        }
    //        return EVENT_DONE;
    //    }
    //
    //    void TextEditSCREEN_PopupScreen::CreatePopupContents(SCREEN_UI::ViewGroup *parent)
    //    {
    //        using namespace SCREEN_UI;
    //        SCREEN_UIContext &dc = *screenManager()->getUIContext();
    //
    //        textEditValue_ = *value_;
    //        LinearLayout *lin = parent->Add(new LinearLayout(ORIENT_HORIZONTAL, new
    //        LinearLayoutParams((SCREEN_UI::Size)300, WRAP_CONTENT))); edit_ = new TextEdit(textEditValue_, placeholder_,
    //        new LinearLayoutParams(1.0f)); edit_->SetMaxLen(maxLen_); edit_->SetTextColor(dc.theme->popupStyle.fgColor);
    //        lin->Add(edit_);
    //
    //        if (IsFocusMovementEnabled())
    //            SCREEN_UI::SetFocusedView(edit_);
    //    }
    //
    //    void TextEditSCREEN_PopupScreen::OnCompleted(DialogResult result)
    //    {
    //        if (result == DR_OK) {
    //            *value_ = SCREEN_StripSpaces(edit_->GetText());
    //            SCREEN_UI::EventParams e{};
    //            e.v = edit_;
    //            OnChange.Trigger(e);
    //        }
    //    }
    //
    //    void SCREEN_ChoiceWithValueDisplay::Draw(SCREEN_UIContext &dc)
    //    {
    //        Style style = dc.theme->itemStyle;
    //        if (!IsEnabled())
    //        {
    //            style = dc.theme->itemDisabledStyle;
    //        }
    //        int paddingX = 12;
    //        dc.SetFontStyle(dc.theme->uiFont);
    //
    //        auto category = GetI18NCategory(category_);
    //        std::ostringstream valueText;
    //        if (translateCallback_ && sValue_)
    //        {
    //            valueText << translateCallback_(sValue_->c_str());
    //        }
    //        else if (sValue_ != nullptr)
    //        {
    //            if (category)
    //            {
    //                valueText << category->T(*sValue_);
    //            }
    //            else
    //            {
    //                valueText << *sValue_;
    //            }
    //        }
    //        else if (iValue_ != nullptr)
    //        {
    //            valueText << *iValue_;
    //        }
    //
    //        float ignore;
    //        dc.MeasureText(dc.theme->uiFont, 1.0f, 1.0f, valueText.str().c_str(), &textPadding_.right, &ignore, ALIGN_RIGHT
    //        | ALIGN_VCENTER); textPadding_.right += paddingX;
    //
    //        Choice::Draw(dc);
    //        dc.DrawText(valueText.str().c_str(), bounds_.x2() - paddingX, bounds_.centerY(), style.fgColor, ALIGN_RIGHT |
    //        ALIGN_VCENTER);
    //    }

    //}  // namespace SCREEN_UI
} // namespace GfxRenderEngine
