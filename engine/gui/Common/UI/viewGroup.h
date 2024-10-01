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

#include <set>

#include "gui/Common/UI/view.h"
#include "sprite/spritesheet.h"

namespace GfxRenderEngine
{

    namespace SCREEN_UI
    {

        class AnchorTranslateTween;

        struct NeighborResult
        {
            NeighborResult() : view(0), score(0) {}
            NeighborResult(View* v, float s) : view(v), score(s) {}

            View* view;
            float score;
        };

        class ViewGroup : public View
        {
        public:
            ViewGroup(LayoutParams* layoutParams = 0) : View(layoutParams) {}
            virtual ~ViewGroup();

            virtual bool Key(const SCREEN_KeyInput& input) override;
            virtual bool Touch(const SCREEN_TouchInput& input) override;
            virtual void Axis(const SCREEN_AxisInput& input) override;

            virtual void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override = 0;
            virtual void Layout() override = 0;
            virtual void Update() override;
            virtual void Query(float x, float y, std::vector<View*>& list) override;

            virtual void DeviceLost() override;
            virtual void DeviceRestored(SCREEN_Draw::SCREEN_DrawContext* draw) override;

            virtual void Draw(SCREEN_UIContext& dc) override;

            virtual float GetContentWidth() const { return 0.0f; }
            virtual float GetContentHeight() const { return 0.0f; }

            template <class T> T* Add(T* view)
            {
                std::lock_guard<std::mutex> guard(modifyLock_);
                views_.push_back(view);
                return view;
            }

            virtual bool SetFocus() override;
            virtual bool SubviewFocused(View* view) override;
            virtual void RemoveSubview(View* view);

            void SetDefaultFocusView(View* view) { defaultFocusView_ = view; }
            View* GetDefaultFocusView() { return defaultFocusView_; }

            NeighborResult FindNeighbor(View* view, FocusDirection direction, NeighborResult best);

            virtual bool CanBeFocused() const override { return false; }
            virtual bool IsViewGroup() const override { return true; }

            virtual void SetBG(const Drawable& bg) { bg_ = bg; }

            virtual void Clear();
            void PersistData(PersistStatus status, std::string anonId, PersistMap& storage) override;
            View* GetViewByIndex(int index) { return views_[index]; }
            int GetNumSubviews() const { return (int)views_.size(); }
            void SetHasDropShadow(bool has) { hasDropShadow_ = has; }
            void SetDropShadowExpand(float s) { dropShadowExpand_ = s; }

            void Lock() { modifyLock_.lock(); }
            void Unlock() { modifyLock_.unlock(); }

            void SetClip(bool clip) { clip_ = clip; }
            std::string Describe() const override { return "ViewGroup: " + View::Describe(); }

        protected:
            std::mutex modifyLock_;
            std::vector<View*> views_;
            View* defaultFocusView_ = nullptr;
            Drawable bg_;
            float dropShadowExpand_ = 0.0f;
            bool hasDropShadow_ = false;
            bool clip_ = false;
        };

        class FrameLayout : public ViewGroup
        {
        public:
            void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            void Layout() override;
        };

        const float NONE = -FLT_MAX;

        class AnchorLayoutParams : public LayoutParams
        {

        public:
            AnchorLayoutParams(Size w, Size h, float l, float t, float r, float b, bool c = false)
                : LayoutParams(w, h, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c)
            {
            }

            AnchorLayoutParams(Size w, Size h, bool c = false)
                : LayoutParams(w, h, LP_ANCHOR), left(0), top(0), right(NONE), bottom(NONE), center(c)
            {
            }

            AnchorLayoutParams(float l, float t, float r, float b, bool c = false)
                : LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_ANCHOR), left(l), top(t), right(r), bottom(b), center(c)
            {
            }

            float left, top, right, bottom;
            bool center;

            static LayoutParamsType StaticType() { return LP_ANCHOR; }
        };

        class AnchorLayout : public ViewGroup
        {
        public:
            AnchorLayout(LayoutParams* layoutParams = 0) : ViewGroup(layoutParams), overflow_(true) {}
            void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            void Layout() override;
            void Overflow(bool allow) { overflow_ = allow; }
            std::string Describe() const override { return "AnchorLayout: " + View::Describe(); }

        private:
            void MeasureViews(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert);
            bool overflow_;
        };

        class LinearLayoutParams : public LayoutParams
        {
        public:
            LinearLayoutParams() : LayoutParams(LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), hasMargins_(false) {}
            explicit LinearLayoutParams(float wgt, Gravity grav = G_TOPLEFT)
                : LayoutParams(LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false)
            {
            }
            LinearLayoutParams(float wgt, const Margins& mgn)
                : LayoutParams(LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true)
            {
            }
            LinearLayoutParams(Size w, Size h, float wgt = 0.0f, Gravity grav = G_TOPLEFT)
                : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), hasMargins_(false)
            {
            }
            LinearLayoutParams(Size w, Size h, float wgt, Gravity grav, const Margins& mgn)
                : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(grav), margins(mgn), hasMargins_(true)
            {
            }
            LinearLayoutParams(Size w, Size h, const Margins& mgn)
                : LayoutParams(w, h, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true)
            {
            }
            LinearLayoutParams(Size w, Size h, float wgt, const Margins& mgn)
                : LayoutParams(w, h, LP_LINEAR), weight(wgt), gravity(G_TOPLEFT), margins(mgn), hasMargins_(true)
            {
            }
            LinearLayoutParams(const Margins& mgn)
                : LayoutParams(WRAP_CONTENT, WRAP_CONTENT, LP_LINEAR), weight(0.0f), gravity(G_TOPLEFT), margins(mgn),
                  hasMargins_(true)
            {
            }

            float weight;
            Gravity gravity;
            Margins margins;

            bool HasMargins() const { return hasMargins_; }

            static LayoutParamsType StaticType() { return LP_LINEAR; }

        private:
            bool hasMargins_;
        };

        class LinearLayout : public ViewGroup
        {
        public:
            LinearLayout(Orientation orientation, LayoutParams* layoutParams = 0)
                : ViewGroup(layoutParams), orientation_(orientation), defaultMargins_(0), spacing_(0)
            {
            }

            void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            void Layout() override;
            void SetSpacing(float spacing) { spacing_ = spacing; }
            std::string Describe() const override
            {
                return (orientation_ == ORIENT_HORIZONTAL ? "LinearLayoutHoriz: " : "LinearLayoutVert: ") + View::Describe();
            }

        protected:
            Orientation orientation_;

        private:
            Margins defaultMargins_;
            float spacing_;
        };

        struct GridLayoutSettings
        {
            GridLayoutSettings()
                : orientation(ORIENT_HORIZONTAL), columnWidth(100), rowHeight(50), spacing(5), fillCells(false)
            {
            }
            GridLayoutSettings(int colW, int colH, int spac = 5)
                : orientation(ORIENT_HORIZONTAL), columnWidth(colW), rowHeight(colH), spacing(spac), fillCells(false)
            {
            }

            Orientation orientation;
            int columnWidth;
            int rowHeight;
            int spacing;
            bool fillCells;
        };

        class GridLayout : public ViewGroup
        {
        public:
            GridLayout(GridLayoutSettings settings, LayoutParams* layoutParams = 0);

            void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            void Layout() override;
            std::string Describe() const override { return "GridLayout: " + View::Describe(); }

        private:
            GridLayoutSettings settings_;
            int numColumns_;
        };

        class ScrollView : public ViewGroup
        {
        public:
            ScrollView(Orientation orientation, LayoutParams* layoutParams = 0, bool exactly = false)
                : ViewGroup(layoutParams), orientation_(orientation), vert_type_exactly_(exactly)
            {
            }

            void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            void Layout() override;

            bool Key(const SCREEN_KeyInput& input) override;
            bool Touch(const SCREEN_TouchInput& touch) override;
            void Draw(SCREEN_UIContext& dc) override;
            std::string Describe() const override { return "ScrollView: " + View::Describe(); }

            void ScrollTo(float newScrollPos);
            void ScrollToBottom();
            void ScrollRelative(float distance);
            float GetScrollPosition();
            bool CanScroll() const;
            void Update() override;

            bool SubviewFocused(View* view) override;
            void PersistData(PersistStatus status, std::string anonId, PersistMap& storage) override;
            void SetVisibility(Visibility visibility) override;

            void SetScrollToTop(bool t) { scrollToTopOnSizeChange_ = t; }

        private:
            float ClampedScrollPos(float pos);

            Orientation orientation_;
            float scrollPos_ = 0.0f;
            float scrollTarget_ = 0.0f;
            bool scrollToTarget_ = false;
            float inertia_ = 0.0f;
            float pull_ = 0.0f;
            float lastViewSize_ = 0.0f;
            bool scrollToTopOnSizeChange_ = false;
            bool vert_type_exactly_ = false;
        };

        class ViewPager : public ScrollView
        {
        public:
        };

        class ChoiceStrip : public LinearLayout
        {
        public:
            ChoiceStrip(Orientation orientation, LayoutParams* layoutParams = 0);

            void AddChoice(const std::string& title);
            void AddChoice(const std::string& title, const Sprite& icon, const Sprite& icon_active,
                           const Sprite& icon_depressed, const Sprite& icon_depressed_inactive, const std::string& text);

            int GetSelection() const { return selected_; }
            void SetSelection(int sel);
            void HighlightChoice(unsigned int choice);

            virtual bool Touch(const SCREEN_TouchInput& input) override;
            bool Key(const SCREEN_KeyInput& input) override;

            void SetTopTabs(bool tabs) { topTabs_ = tabs; }
            void Draw(SCREEN_UIContext& dc) override;
            bool AnyTabHasFocus(int& tab);
            void SetEnabled(int tab);
            void disableAllTabs();
            void enableAllTabs();
            std::string Describe() const override { return "ChoiceStrip: " + View::Describe(); }

            Event OnChoice;

        private:
            StickyChoice* Choice(int index);
            EventReturn OnChoiceClick(EventParams& e);
            int selected_;
            bool topTabs_;
        };

        class TabHolder : public LinearLayout
        {
        public:
            TabHolder(Orientation orientation, float stripSize, LayoutParams* layoutParams = 0, float leftMargin = 0.0f);

            template <class T> T* AddTab(const std::string& title, T* tabContents)
            {
                AddTabContents(title, (View*)tabContents);
                return tabContents;
            }

            std::string Describe() const override { return "TabHolder: " + View::Describe(); }
            void PersistData(PersistStatus status, std::string anonId, PersistMap& storage) override;

            int GetCurrentTab() const { return currentTab_; }
            void SetCurrentTab(int tab, bool skipTween = false);
            void SetIcon(const Sprite& icon, const Sprite& icon_active, const Sprite& icon_depressed,
                         const Sprite& icon_depressed_inactive);

            bool HasFocus(int& tab);
            void enableAllTabs();
            void disableAllTabs();
            void SetEnabled(int tab);

        private:
            bool useIcons_ = false;
            Sprite m_Icon;
            Sprite m_Icon_active;
            Sprite m_Icon_depressed;
            Sprite m_Icon_depressed_inactive;
            void AddTabContents(const std::string& title, View* tabContents);
            EventReturn OnTabClick(EventParams& e);

            ChoiceStrip* tabStrip_ = nullptr;
            ScrollView* tabScroll_ = nullptr;
            AnchorLayout* contents_ = nullptr;

            int currentTab_ = 0;
            std::vector<View*> tabs_;
            std::vector<AnchorTranslateTween*> tabTweens_;
        };

        class ListAdaptor
        {
        public:
            virtual ~ListAdaptor() {}
            virtual View* CreateItemView(int index, float width = 800.0f) = 0;
            virtual int GetNumItems() = 0;
            virtual bool AddEventCallback(View* view, std::function<EventReturn(EventParams&)> callback) { return false; }
            virtual std::string GetTitle(int index) const { return ""; }
            virtual void SetSelected(int sel) {}
            virtual int GetSelected() { return -1; }
        };

        class ChoiceListAdaptor : public ListAdaptor
        {
        public:
            ChoiceListAdaptor(const char* items[], int numItems) : items_(items), numItems_(numItems) {}
            virtual View* CreateItemView(int index, float width = 800.0f);
            virtual int GetNumItems() { return numItems_; }
            virtual bool AddEventCallback(View* view, std::function<EventReturn(EventParams&)> callback);

        private:
            const char** items_;
            int numItems_;
        };

        class StringVectorListAdaptor : public ListAdaptor
        {
        public:
            StringVectorListAdaptor() : selected_(-1) {}
            StringVectorListAdaptor(const std::vector<std::string>& items, int selected = -1)
                : items_(items), selected_(selected)
            {
            }
            virtual View* CreateItemView(int index, float width = 800.0f) override;
            virtual int GetNumItems() override { return (int)items_.size(); }
            virtual bool AddEventCallback(View* view, std::function<EventReturn(EventParams&)> callback) override;
            void SetSelected(int sel) override { selected_ = sel; }
            virtual std::string GetTitle(int index) const override { return items_[index]; }
            virtual int GetSelected() override { return selected_; }

        private:
            std::vector<std::string> items_;
            int selected_;
        };

        class ListView : public ScrollView
        {
        public:
            ListView(ListAdaptor* a, float popupWidth, std::set<int> hidden = std::set<int>(),
                     LayoutParams* layoutParams = 0);

            int GetSelected() { return adaptor_->GetSelected(); }
            virtual void Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert) override;
            virtual void SetMaxHeight(float mh) { maxHeight_ = mh; }
            Event OnChoice;
            std::string Describe() const override { return "ListView: " + View::Describe(); }

        private:
            void CreateAllItems();
            EventReturn OnItemCallback(int num, EventParams& e);
            ListAdaptor* adaptor_;
            LinearLayout* linLayout_;
            float maxHeight_;
            std::set<int> hidden_;
            float m_Width;
        };

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
