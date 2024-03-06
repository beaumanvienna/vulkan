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

#include <map>
#include <functional>
#include <memory>

#include "core.h"
#include "gui/common.h"
#include "gui/Render/textureAtlas.h"
#include "gui/Common/Math/geom2d.h"
#include "gui/Common/Input/inputState.h"

namespace GfxRenderEngine
{

    struct SCREEN_KeyInput;
    struct SCREEN_TouchInput;
    struct SCREEN_AxisInput;

    class SCREEN_DrawBuffer;
    class SCREEN_Texture;
    class SCREEN_UIContext;

    namespace SCREEN_Draw
    {
        class SCREEN_DrawContext;
        class SCREEN_Texture;
    } // namespace SCREEN_Draw

    namespace SCREEN_UI
    {
        class View;

        enum DrawableType
        {
            DRAW_NOTHING,
            DRAW_SOLID_COLOR,
            DRAW_4GRID,
            DRAW_STRETCH_IMAGE,
        };

        enum Visibility
        {
            V_VISIBLE,
            V_INVISIBLE,
            V_GONE,
        };

        struct Drawable
        {
            Drawable() : type(DRAW_NOTHING), color(0xFFFFFFFF) {}

            explicit Drawable(uint32_t col) : type(DRAW_SOLID_COLOR), color(col) {}

            DrawableType type;
            uint32_t color;
        };

        struct Style
        {
            Style() : fgColor(0xFFFFFFFF), background(0xFF303030) {}

            uint32_t fgColor;
            Drawable background;
        };

        struct FontStyle
        {
            FontStyle() : atlasFont(0), sizePts(0), flags(0) {}

            FontStyle(const char* name, int size) : atlasFont(0), fontName(name), sizePts(size), flags(0) {}

            FontStyle(FontID atlasFnt, const char* name, int size)
                : atlasFont(atlasFnt), fontName(name), sizePts(size), flags(0)
            {
            }

            FontID atlasFont;
            // For native fonts:
            std::string fontName;
            int sizePts;
            int flags;
        };

        struct Theme
        {
            FontStyle uiFont;
            FontStyle uiFontSmall;
            FontStyle uiFontSmaller;

            Sprite checkOn;
            Sprite checkOff;
            Sprite sliderKnob;
            Sprite whiteImage;
            Sprite dropShadow4Grid;

            Style buttonStyle;
            Style buttonFocusedStyle;
            Style buttonDownStyle;
            Style buttonDisabledStyle;
            Style buttonHighlightedStyle;

            Style itemStyle;
            Style itemDownStyle;
            Style itemFocusedStyle;
            Style itemDisabledStyle;
            Style itemHighlightedStyle;

            Style headerStyle;
            Style infoStyle;

            Style popupTitle;
            Style popupStyle;
        };

        enum FocusDirection
        {
            FOCUS_UP,
            FOCUS_DOWN,
            FOCUS_LEFT,
            FOCUS_RIGHT,
            FOCUS_NEXT,
            FOCUS_PREV,
        };

        enum
        {
            WRAP_CONTENT = -1,
            FILL_PARENT = -2,
        };

        enum Gravity
        {
            G_LEFT = 0,
            G_RIGHT = 1,
            G_HCENTER = 2,

            G_HORIZMASK = 3,

            G_TOP = 0,
            G_BOTTOM = 4,
            G_VCENTER = 8,

            G_TOPLEFT = G_TOP | G_LEFT,
            G_TOPRIGHT = G_TOP | G_RIGHT,

            G_BOTTOMLEFT = G_BOTTOM | G_LEFT,
            G_BOTTOMRIGHT = G_BOTTOM | G_RIGHT,

            G_CENTER = G_HCENTER | G_VCENTER,

            G_VERTMASK = 3 << 2,
        };

        typedef float Size;

        enum Orientation
        {
            ORIENT_HORIZONTAL,
            ORIENT_VERTICAL,
        };

        inline Orientation Opposite(Orientation o)
        {
            if (o == ORIENT_HORIZONTAL)
                return ORIENT_VERTICAL;
            else
                return ORIENT_HORIZONTAL;
        }

        inline FocusDirection Opposite(FocusDirection d)
        {
            switch (d)
            {
                case FOCUS_UP:
                    return FOCUS_DOWN;
                case FOCUS_DOWN:
                    return FOCUS_UP;
                case FOCUS_LEFT:
                    return FOCUS_RIGHT;
                case FOCUS_RIGHT:
                    return FOCUS_LEFT;
                case FOCUS_PREV:
                    return FOCUS_NEXT;
                case FOCUS_NEXT:
                    return FOCUS_PREV;
            }
            return d;
        }

        enum MeasureSpecType
        {
            UNSPECIFIED,
            EXACTLY,
            AT_MOST,
        };

        enum EventReturn
        {
            EVENT_DONE,
            EVENT_SKIPPED,
            EVENT_CONTINUE,
        };

        enum FocusFlags
        {
            FF_LOSTFOCUS = 1,
            FF_GOTFOCUS = 2
        };

        enum PersistStatus
        {
            PERSIST_SAVE,
            PERSIST_RESTORE,
        };

        typedef std::vector<int> PersistBuffer;
        typedef std::map<std::string, SCREEN_UI::PersistBuffer> PersistMap;

        class ViewGroup;

        struct MeasureSpec
        {
            MeasureSpec(MeasureSpecType t, float s = 0.0f) : type(t), size(s) {}
            MeasureSpec() : type(UNSPECIFIED), size(0) {}

            MeasureSpec operator-(float amount) { return MeasureSpec(type, size - amount); }
            MeasureSpecType type;
            float size;
        };

        struct EventParams
        {
            View* v;
            uint32_t a, b, x, y;
            float f;
            std::string s;
        };

        struct HandlerRegistration
        {
            std::function<EventReturn(EventParams&)> func;
        };

        class Event
        {
        public:
            Event() {}
            ~Event();

            void Trigger(EventParams& e);

            EventReturn Dispatch(EventParams& e);

            template <class T> T* Handle(T* thiz, EventReturn (T::*theCallback)(EventParams& e))
            {
                Add(std::bind(theCallback, thiz, std::placeholders::_1));
                return thiz;
            }

            void Add(std::function<EventReturn(EventParams&)> func);

        private:
            std::vector<HandlerRegistration> handlers_;
        };

        struct Margins
        {
            Margins() : top(0), bottom(0), left(0), right(0) {}
            explicit Margins(int8_t all) : top(all), bottom(all), left(all), right(all) {}
            Margins(int8_t horiz, int8_t vert) : top(vert), bottom(vert), left(horiz), right(horiz) {}
            Margins(int8_t l, int8_t t, int8_t r, int8_t b) : top(t), bottom(b), left(l), right(r) {}

            int horiz() const { return left + right; }
            int vert() const { return top + bottom; }

            int8_t top;
            int8_t bottom;
            int8_t left;
            int8_t right;
        };

        struct Padding
        {
            Padding() : top(0), bottom(0), left(0), right(0) {}
            explicit Padding(float all) : top(all), bottom(all), left(all), right(all) {}
            Padding(float horiz, float vert) : top(vert), bottom(vert), left(horiz), right(horiz) {}
            Padding(float l, float t, float r, float b) : top(t), bottom(b), left(l), right(r) {}

            float horiz() const { return left + right; }
            float vert() const { return top + bottom; }

            float top;
            float bottom;
            float left;
            float right;
        };

        enum LayoutParamsType
        {
            LP_PLAIN = 0,
            LP_LINEAR = 1,
            LP_ANCHOR = 2,
        };

        class LayoutParams
        {
        public:
            LayoutParams(LayoutParamsType type = LP_PLAIN) : width(WRAP_CONTENT), height(WRAP_CONTENT), type_(type) {}
            LayoutParams(Size w, Size h, LayoutParamsType type = LP_PLAIN) : width(w), height(h), type_(type) {}
            virtual ~LayoutParams() {}
            Size width;
            Size height;

            bool Is(LayoutParamsType type) const { return type_ == type; }

            template <typename T> T* As()
            {
                if (Is(T::StaticType()))
                {
                    return static_cast<T*>(this);
                }
                return nullptr;
            }

            template <typename T> const T* As() const
            {
                if (Is(T::StaticType()))
                {
                    return static_cast<const T*>(this);
                }
                return nullptr;
            }

            static LayoutParamsType StaticType() { return LP_PLAIN; }

        private:
            LayoutParamsType type_;
        };

        View* GetFocusedView();

        class Tween;
        class CallbackColorTween;

        class View
        {
        public:
            View(LayoutParams* layoutParams = 0)
                : layoutParams_(layoutParams), visibility_(V_VISIBLE), measuredWidth_(0), measuredHeight_(0), enabledPtr_(0),
                  enabled_(true), enabledMeansDisabled_(false)
            {
                if (!layoutParams)
                {
                    layoutParams_.reset(new LayoutParams());
                }
            }
            virtual ~View();

            virtual bool Key(const SCREEN_KeyInput& input) { return false; }
            virtual bool Touch(const SCREEN_TouchInput& input) { return false; }
            virtual void Axis(const SCREEN_AxisInput& input) {}
            virtual void Update();

            virtual void DeviceLost() {}
            virtual void DeviceRestored(SCREEN_Draw::SCREEN_DrawContext* draw) {}

            virtual void Query(float x, float y, std::vector<View*>& list);
            virtual std::string Describe() const;

            virtual void FocusChanged(int focusFlags) {}
            virtual void PersistData(PersistStatus status, std::string anonId, PersistMap& storage);

            void Move(Bounds bounds) { bounds_ = bounds; }

            virtual void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert);
            virtual void Layout() {}
            virtual void Draw(SCREEN_UIContext& dc) {}

            virtual float GetMeasuredWidth() const { return measuredWidth_; }
            virtual float GetMeasuredHeight() const { return measuredHeight_; }

            virtual void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const;
            virtual void GetContentDimensionsBySpec(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert,
                                                    float& w, float& h) const;

            void SetBounds(Bounds bounds) { bounds_ = bounds; }
            virtual const LayoutParams* GetLayoutParams() const { return layoutParams_.get(); }
            virtual void ReplaceLayoutParams(LayoutParams* newLayoutParams) { layoutParams_.reset(newLayoutParams); }
            const Bounds& GetBounds() const { return bounds_; }

            virtual bool SetFocus();

            virtual bool CanBeFocused() const { return true; }
            virtual bool SubviewFocused(View* view) { return false; }

            bool HasFocus() const { return GetFocusedView() == this; }

            void SetEnabled(bool enabled)
            {
                enabledFunc_ = nullptr;
                enabledPtr_ = nullptr;
                enabled_ = enabled;
                enabledMeansDisabled_ = false;
            }
            bool IsEnabled() const
            {

                if (enabledFunc_)
                {
                    return enabledFunc_() != enabledMeansDisabled_;
                }
                if (enabledPtr_)
                {
                    return *enabledPtr_ != enabledMeansDisabled_;
                }
                return enabled_ != enabledMeansDisabled_;
            }
            void SetEnabledFunc(std::function<bool()> func)
            {
                enabledFunc_ = func;
                enabledPtr_ = nullptr;
                enabledMeansDisabled_ = false;
            }
            void SetEnabledPtr(bool* enabled)
            {
                enabledFunc_ = nullptr;
                enabledPtr_ = enabled;
                enabledMeansDisabled_ = false;
            }
            void SetDisabledPtr(bool* disabled)
            {
                enabledFunc_ = nullptr;
                enabledPtr_ = disabled;
                enabledMeansDisabled_ = true;
            }

            virtual void SetVisibility(Visibility visibility) { visibility_ = visibility; }
            Visibility GetVisibility() const { return visibility_; }

            const std::string& Tag() const { return tag_; }
            void SetTag(const std::string& str) { tag_ = str; }

            virtual bool IsViewGroup() const { return false; }

            Point GetFocusPosition(FocusDirection dir);

            template <class T> T* AddTween(T* t)
            {
                tweens_.push_back(t);
                return t;
            }

        protected:
            std::unique_ptr<LayoutParams> layoutParams_;

            std::string tag_;
            Visibility visibility_;

            float measuredWidth_;
            float measuredHeight_;

            Bounds bounds_;

            std::vector<Tween*> tweens_;

        private:
            std::function<bool()> enabledFunc_;
            bool* enabledPtr_;
            bool enabled_;
            bool enabledMeansDisabled_;
        };

        class InertView : public View
        {
        public:
            InertView(LayoutParams* layoutParams) : View(layoutParams) {}

            bool Key(const SCREEN_KeyInput& input) override { return false; }
            bool Touch(const SCREEN_TouchInput& input) override { return false; }
            bool CanBeFocused() const override { return false; }
        };

        class Clickable : public View
        {
        public:
            Clickable(LayoutParams* layoutParams);
            virtual ~Clickable() {}

            bool Key(const SCREEN_KeyInput& input) override;
            bool Touch(const SCREEN_TouchInput& input) override;

            void FocusChanged(int focusFlags) override;

            Event OnClick;

        protected:
            virtual void Click();
            void DrawBG(SCREEN_UIContext& dc, const Style& style);

            CallbackColorTween* bgColor_ = nullptr;
            float bgColorLast_ = 0.0f;
            int downCountDown_ = 0;
            bool dragging_ = false;
            bool down_ = false;
        };

        class Button : public Clickable
        {
        public:
            Button(const std::string& text, uint maxTextLength, LayoutParams* layoutParams = nullptr);
            Button(const Sprite& image, LayoutParams* layoutParams = nullptr) : Clickable(layoutParams), m_Image(image) {}
            Button(const std::string& text, const Sprite& image, LayoutParams* layoutParams = nullptr)
                : Clickable(layoutParams), text_(text), m_Image(image)
            {
            }

            void Draw(SCREEN_UIContext& dc) override;
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
            const std::string& GetText() const { return text_; }

            void SetPadding(int w, int h)
            {
                paddingW_ = w;
                paddingH_ = h;
            }

            void SetScale(float f) { scale_ = f; }

        private:
            Style style_;
            std::string text_;
            Sprite m_Image;
            int paddingW_ = 16;
            int paddingH_ = 8;
            float scale_ = 1.0f;
        };

        class Slider : public Clickable
        {
        public:
            Slider(int* value, int minValue, int maxValue, LayoutParams* layoutParams = 0)
                : Clickable(layoutParams), value_(value), showPercent_(false), minValue_(minValue), maxValue_(maxValue),
                  paddingLeft_(5), paddingRight_(70), step_(1), repeat_(-1)
            {
            }

            Slider(int* value, int minValue, int maxValue, int step = 1, LayoutParams* layoutParams = 0)
                : Clickable(layoutParams), value_(value), showPercent_(false), minValue_(minValue), maxValue_(maxValue),
                  paddingLeft_(5), paddingRight_(70), repeat_(-1)
            {
                step_ = step <= 0 ? 1 : step;
            }
            void Draw(SCREEN_UIContext& dc) override;
            bool Key(const SCREEN_KeyInput& input) override;
            bool Touch(const SCREEN_TouchInput& input) override;
            void Update() override;
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
            void SetShowPercent(bool s) { showPercent_ = s; }

            void Clamp();

            Event OnChange;

        private:
            bool ApplyKey(int keyCode);

            int* value_;
            bool showPercent_;
            int minValue_;
            int maxValue_;
            float paddingLeft_;
            float paddingRight_;
            int step_;
            int repeat_ = 0;
            int repeatCode_ = 0;
        };

        class SliderFloat : public Clickable
        {
        public:
            SliderFloat(float* value, float minValue, float maxValue, LayoutParams* layoutParams = 0)
                : Clickable(layoutParams), value_(value), minValue_(minValue), maxValue_(maxValue), paddingLeft_(5),
                  paddingRight_(70), repeat_(-1)
            {
            }
            void Draw(SCREEN_UIContext& dc) override;
            bool Key(const SCREEN_KeyInput& input) override;
            bool Touch(const SCREEN_TouchInput& input) override;
            void Update() override;
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;

            void Clamp();

            Event OnChange;

        private:
            bool ApplyKey(int keyCode);

            float* value_;
            float minValue_;
            float maxValue_;
            float paddingLeft_;
            float paddingRight_;
            int repeat_;
            int repeatCode_ = 0;
        };

        class Item : public InertView
        {
        public:
            Item(LayoutParams* layoutParams);
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
        };

        class ClickableItem : public Clickable
        {
        public:
            ClickableItem(LayoutParams* layoutParams);
            ClickableItem(LayoutParams* layoutParams, bool transparentBackground);
            virtual ~ClickableItem() {}
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;

            // Draws the item background.
            void Draw(SCREEN_UIContext& dc) override;

        private:
            bool transparentBackground_ = false;
        };

        // Use to trigger something or open a submenu screen.
        class Choice : public ClickableItem
        {
        public:
            Choice(const std::string& text, LayoutParams* layoutParams = nullptr)
                : Choice(text, std::string(), false, layoutParams)
            {
                numIcons_ = 0;
            }
            Choice(const std::string& text, bool transparentBackground, LayoutParams* layoutParams)
                : Choice(text, transparentBackground, std::string(), false, layoutParams)
            {
                numIcons_ = 0;
            }
            Choice(const std::string& text, const std::string& smallText, bool selected, LayoutParams* layoutParams)
                : ClickableItem(layoutParams), text_(text), smallText_(smallText), m_Image({}), centered_(false),
                  highlighted_(false), selected_(selected)
            {
                numIcons_ = 0;
            }
            Choice(const std::string& text, bool transparentBackground, const std::string& smallText, bool selected,
                   LayoutParams* layoutParams)
                : ClickableItem(layoutParams, transparentBackground), text_(text), smallText_(smallText), m_Image({}),
                  centered_(false), highlighted_(false), selected_(selected)
            {
                numIcons_ = 0;
            }
            Choice(const Sprite& image, LayoutParams* layoutParams = nullptr, bool hasHoldFeature = false)
                : ClickableItem(layoutParams), m_Image(image), centered_(false), highlighted_(false), selected_(false),
                  hasHoldFeature_(hasHoldFeature), heldDown_(false)
            {
                numIcons_ = 1;
            }
            Choice(const Sprite& image, const Sprite& image_active, const Sprite& image_depressed,
                   LayoutParams* layoutParams = nullptr, bool hasHoldFeature = false)
                : ClickableItem(layoutParams), centered_(false), highlighted_(false), selected_(false),
                  hasHoldFeature_(hasHoldFeature), heldDown_(false), m_Image(image), m_ImageActive(image_active),
                  m_ImageDepressed(image_depressed)
            {
                numIcons_ = 3;
            }
            Choice(const Sprite& image, const Sprite& image_active, const Sprite& image_depressed,
                   const Sprite& image_depressed_inactive, const std::string& text, LayoutParams* layoutParams = nullptr,
                   bool hasHoldFeature = false)
                : ClickableItem(layoutParams), centered_(false), highlighted_(false), selected_(false),
                  hasHoldFeature_(hasHoldFeature), heldDown_(false), text_(text), m_Image(image),
                  m_ImageActive(image_active), m_ImageDepressed(image_depressed),
                  m_ImageDepressedInactive(image_depressed_inactive)
            {
                numIcons_ = 4;
            }

            Event OnHold;
            Event OnHighlight;
            bool Key(const SCREEN_KeyInput& input) override;
            bool Touch(const SCREEN_TouchInput& touch) override;
            void Update() override;
            virtual void HighlightChanged(bool highlighted);
            void GetContentDimensionsBySpec(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert, float& w,
                                            float& h) const override;
            void Draw(SCREEN_UIContext& dc) override;
            virtual void SetCentered(bool c) { centered_ = c; }
            virtual void SetIcon(Sprite& iconImage) { m_Image = iconImage; }
            bool CanBeFocused() const override { return focusable_; }
            void SetFocusable(bool focusable) { focusable_ = focusable; }
            void SetText(const std::string& text) { text_ = text; }
            void SetName(const std::string& name) { m_Name = name; }
            std::string GetName() const { return m_Name; }

        protected:
            virtual bool IsSticky() const { return false; }
            virtual float CalculateTextScale(const SCREEN_UIContext& dc, float availWidth) const;

            std::string text_;
            std::string smallText_;
            Sprite m_Image;
            Sprite m_ImageActive;
            Sprite m_ImageDepressed;
            Sprite m_ImageDepressedInactive;
            int numIcons_;
            Padding textPadding_;
            bool centered_;
            bool highlighted_;
            double holdStart_ = 0.0f;
            bool heldDown_ = false;
            bool hasHoldFeature_;
            bool focusable_ = true;

        private:
            bool selected_;
            std::string m_Name;
        };

        class StickyChoice : public Choice
        {
        public:
            StickyChoice(const std::string& text, const std::string& smallText = "", LayoutParams* layoutParams = 0)
                : Choice(text, smallText, false, layoutParams)
            {
            }
            StickyChoice(const Sprite& buttonImage, LayoutParams* layoutParams = 0) : Choice(buttonImage, layoutParams) {}
            StickyChoice(const Sprite& icon, const Sprite& icon_active, const Sprite& icon_depressed,
                         const Sprite& icon_depressed_inactive, const std::string& text, LayoutParams* layoutParams = 0)
                : Choice(icon, icon_active, icon_depressed, icon_depressed_inactive, text, layoutParams)
            {
            }

            bool Key(const SCREEN_KeyInput& key) override;
            bool Touch(const SCREEN_TouchInput& touch) override;
            void FocusChanged(int focusFlags) override;

            void Press()
            {
                down_ = true;
                dragging_ = false;
            }
            void Release()
            {
                down_ = false;
                dragging_ = false;
            }
            bool IsDown() { return down_; }

        protected:
            bool IsSticky() const override { return true; }
        };

        class InfoItem : public Item
        {
        public:
            InfoItem(const std::string& text, const std::string& rightText, LayoutParams* layoutParams = nullptr);

            void Draw(SCREEN_UIContext& dc) override;

            bool CanBeFocused() const override { return true; }

            void SetText(const std::string& text) { text_ = text; }
            const std::string& GetText() const { return text_; }
            void SetRightText(const std::string& text) { rightText_ = text; }

        private:
            CallbackColorTween* bgColor_ = nullptr;
            CallbackColorTween* fgColor_ = nullptr;

            std::string text_;
            std::string rightText_;
        };

        class ItemHeader : public Item
        {
        public:
            ItemHeader(const std::string& text, LayoutParams* layoutParams = 0);
            void Draw(SCREEN_UIContext& dc) override;
            void GetContentDimensionsBySpec(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert, float& w,
                                            float& h) const override;

        private:
            std::string text_;
        };

        class PopupHeader : public Item
        {
        public:
            PopupHeader(const std::string& text, LayoutParams* layoutParams = 0) : Item(layoutParams), text_(text)
            {
                layoutParams_->width = FILL_PARENT;
                layoutParams_->height = 64.0f;
            }
            void Draw(SCREEN_UIContext& dc) override;

        private:
            std::string text_;
        };

        class Separator : public Item
        {
        public:
            Separator(LayoutParams* layoutParams = 0) : Item(layoutParams)
            {
                layoutParams_->width = FILL_PARENT;
                layoutParams_->height = 4.0f;
            }
            void Draw(SCREEN_UIContext& dc) override;
        };

        class CheckBox : public ClickableItem
        {
        public:
            CheckBox(bool* toggle, const std::string& text, const std::string& smallText = "",
                     LayoutParams* layoutParams = 0)
                : ClickableItem(layoutParams), toggle_(toggle), text_(text), smallText_(smallText)
            {
                OnClick.Handle(this, &CheckBox::OnClicked);
            }

            void Draw(SCREEN_UIContext& dc) override;
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;

            EventReturn OnClicked(EventParams& e);

            virtual void Toggle();
            virtual bool Toggled() const;

        private:
            float CalculateTextScale(const SCREEN_UIContext& dc, float availWidth) const;

            bool* toggle_;
            std::string text_;
            std::string smallText_;
        };

        class BitCheckBox : public CheckBox
        {
        public:
            BitCheckBox(uint32_t* bitfield, uint32_t bit, const std::string& text, const std::string& smallText = "",
                        LayoutParams* layoutParams = nullptr)
                : CheckBox(nullptr, text, smallText, layoutParams), bitfield_(bitfield), bit_(bit)
            {
            }

            void Toggle() override;
            bool Toggled() const override;

        private:
            uint32_t* bitfield_;
            uint32_t bit_;
        };

        class Spacer : public InertView
        {
        public:
            Spacer(LayoutParams* layoutParams = 0) : InertView(layoutParams), w_(0.0f), h_(0.0f) {}
            Spacer(float size, LayoutParams* layoutParams = 0) : InertView(layoutParams), w_(size), h_(size) {}
            Spacer(float w, float h, LayoutParams* layoutParams = 0) : InertView(layoutParams), w_(w), h_(h) {}
            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override
            {
                w = w_;
                h = h_;
            }
            void Draw(SCREEN_UIContext& dc) override {}

        private:
            float w_;
            float h_;
        };

        class TextView : public InertView
        {
        public:
            TextView(const std::string& text, LayoutParams* layoutParams = 0)
                : InertView(layoutParams), text_(text), textAlign_(0), textColor_(0xFFFFFFFF), shadow_(false),
                  focusable_(false), clip_(true)
            {
            }

            TextView(const std::string& text, int textAlign, bool big, LayoutParams* layoutParams = 0)
                : InertView(layoutParams), text_(text), textAlign_(textAlign), textColor_(0xFFFFFFFF),
                  shadow_(CoreSettings::m_UITheme == THEME_RETRO), focusable_(false), clip_(true)
            {
            }

            void GetContentDimensionsBySpec(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert, float& w,
                                            float& h) const override;
            void Draw(SCREEN_UIContext& dc) override;

            void SetText(const std::string& text) { text_ = text; }
            const std::string& GetText() const { return text_; }
            void SetTextColor(uint32_t color)
            {
                textColor_ = color;
                hasTextColor_ = true;
            }
            void SetShadow(bool shadow) { shadow_ = shadow; }
            void SetFocusable(bool focusable) { focusable_ = focusable; }
            void SetClip(bool clip) { clip_ = clip; }

            bool CanBeFocused() const override { return focusable_; }

        private:
            std::string text_;
            int textAlign_;
            uint32_t textColor_;
            bool hasTextColor_ = false;
            bool shadow_;
            bool focusable_;
            bool clip_;
        };

        class TextEdit : public View
        {
        public:
            TextEdit(const std::string& text, const std::string& placeholderText, LayoutParams* layoutParams = 0);
            void SetText(const std::string& text)
            {
                text_ = text;
                scrollPos_ = 0;
                caret_ = (int)text_.size();
            }
            void SetTextColor(uint32_t color)
            {
                textColor_ = color;
                hasTextColor_ = true;
            }
            const std::string& GetText() const { return text_; }
            void SetMaxLen(size_t maxLen) { maxLen_ = maxLen; }
            void SetTextAlign(int align) { align_ = align; }

            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
            void Draw(SCREEN_UIContext& dc) override;
            bool Key(const SCREEN_KeyInput& key) override;
            bool Touch(const SCREEN_TouchInput& touch) override;

            Event OnTextChange;
            Event OnEnter;

        private:
            void InsertAtCaret(const char* text);

            std::string text_;
            std::string undo_;
            std::string placeholderText_;
            uint32_t textColor_;
            bool hasTextColor_ = false;
            int caret_;
            int scrollPos_ = 0;
            size_t maxLen_;
            bool ctrlDown_ = false;
            int align_ = 0;
        };

        class ImageView : public InertView
        {
        public:
            ImageView(const Sprite& atlasImage, LayoutParams* layoutParams = 0)
                : InertView(layoutParams), m_Image(atlasImage)
            {
            }

            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
            void Draw(SCREEN_UIContext& dc) override;

        private:
            Sprite m_Image;
        };

        class ProgressBar : public InertView
        {
        public:
            ProgressBar(LayoutParams* layoutParams = 0) : InertView(layoutParams), progress_(0.0) {}

            void GetContentDimensions(const SCREEN_UIContext& dc, float& w, float& h) const override;
            void Draw(SCREEN_UIContext& dc) override;

            void SetProgress(float progress)
            {
                if (progress > 1.0f)
                {
                    progress_ = 1.0f;
                }
                else if (progress < 0.0f)
                {
                    progress_ = 0.0f;
                }
                else
                {
                    progress_ = progress;
                }
            }
            float GetProgress() const { return progress_; }

        private:
            float progress_;
        };

        void MeasureBySpec(Size sz, float contentWidth, MeasureSpec spec, float* measured);

        bool IsDPadKey(const SCREEN_KeyInput& key);
        bool IsAcceptKey(const SCREEN_KeyInput& key);
        bool IsEscapeKey(const SCREEN_KeyInput& key);
        bool IsTabLeftKey(const SCREEN_KeyInput& key);
        bool IsTabRightKey(const SCREEN_KeyInput& key);

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
