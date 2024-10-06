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

#include <mutex>
#include <cfloat>

#include "engine.h"
#include "gui/Common/Math/geom2d.h"
#include "gui/Common/UI/viewGroup.h"
#include "gui/Common/stringUtils.h"
#include "gui/Common/UI/context.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/Math/curves.h"
#include "gui/Common/UI/tween.h"

namespace GfxRenderEngine
{

    namespace SCREEN_UI
    {

        const float ITEM_HEIGHT = 64.0f;

        void ApplyGravity(const Bounds outer, const Margins& margins, float w, float h, int gravity, Bounds& inner)
        {
            inner.w = w;
            inner.h = h;

            switch (gravity & G_HORIZMASK)
            {
                case G_LEFT:
                    inner.x = outer.x + margins.left;
                    break;
                case G_RIGHT:
                    inner.x = outer.x + outer.w - w - margins.right;
                    break;
                case G_HCENTER:
                    inner.x = outer.x + (outer.w - w) / 2;
                    break;
            }

            switch (gravity & G_VERTMASK)
            {
                case G_TOP:
                    inner.y = outer.y + margins.top;
                    break;
                case G_BOTTOM:
                    inner.y = outer.y + outer.h - h - margins.bottom;
                    break;
                case G_VCENTER:
                    inner.y = outer.y + (outer.h - h) / 2;
                    break;
            }
        }

        ViewGroup::~ViewGroup() { Clear(); }

        void ViewGroup::RemoveSubview(View* view)
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (size_t i = 0; i < views_.size(); ++i)
            {
                if (views_[i] == view)
                {
                    views_.erase(views_.begin() + i);
                    delete view;
                    return;
                }
            }
        }

        void ViewGroup::Clear()
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (size_t i = 0; i < views_.size(); ++i)
            {
                delete views_[i];
                views_[i] = nullptr;
            }
            views_.clear();
        }

        void ViewGroup::PersistData(PersistStatus status, std::string anonId, PersistMap& storage)
        {
            std::lock_guard<std::mutex> guard(modifyLock_);

            std::string tag = Tag();
            if (tag.empty())
            {
                tag = anonId;
            }

            for (size_t i = 0; i < views_.size(); ++i)
            {
                views_[i]->PersistData(status, tag + "/" + SCREEN_StringFromInt((int)i), storage);
            }
        }

        void ViewGroup::Query(float x, float y, std::vector<View*>& list)
        {
            if (bounds_.Contains(x, y))
            {
                list.push_back(this);
                for (auto iter = views_.begin(); iter != views_.end(); ++iter)
                {
                    (*iter)->Query(x, y, list);
                }
            }
        }

        bool ViewGroup::Key(const SCREEN_KeyInput& input)
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            bool ret = false;
            for (auto iter = views_.begin(); iter != views_.end(); ++iter)
            {
                if ((*iter)->GetVisibility() == V_VISIBLE)
                {
                    ret = ret || (*iter)->Key(input);
                }
            }
            return ret;
        }

        bool ViewGroup::Touch(const SCREEN_TouchInput& input)
        {
            bool clicked = false;
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (auto iter = views_.begin(); iter != views_.end(); ++iter)
            {
                if ((*iter)->GetVisibility() == V_VISIBLE)
                {
                    clicked = (*iter)->Touch(input);
                    if (clicked)
                        return true;
                }
            }
            return clicked;
        }

        void ViewGroup::Axis(const SCREEN_AxisInput& input)
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (auto iter = views_.begin(); iter != views_.end(); ++iter)
            {
                if ((*iter)->GetVisibility() == V_VISIBLE)
                {
                    (*iter)->Axis(input);
                }
            }
        }

        void ViewGroup::DeviceLost()
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (auto iter = views_.begin(); iter != views_.end(); ++iter)
            {
                (*iter)->DeviceLost();
            }
        }

        void ViewGroup::DeviceRestored(SCREEN_Draw::SCREEN_DrawContext* draw)
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            for (auto iter = views_.begin(); iter != views_.end(); ++iter)
            {
                (*iter)->DeviceRestored(draw);
            }
        }

        void ViewGroup::Draw(SCREEN_UIContext& dc)
        {
            if (hasDropShadow_)
            {
                dc.FillRect(SCREEN_UI::Drawable(0x60000000), dc.GetBounds().Expand(dropShadowExpand_));
                float dropsize = 40.0f;
                dc.Draw()->DrawImage4Grid(dc.theme->dropShadow4Grid, bounds_.x - dropsize, bounds_.y - dropsize * 1.5f,
                                          bounds_.x2() + dropsize, bounds_.y2() + dropsize * 1.5f,
                                          /*Color*/ 0xFF000000, /*corner_scale*/ 3.0f);
            }

            if (clip_)
            {
                dc.PushScissor(bounds_);
            }

            dc.FillRect(bg_, bounds_);

            for (View* view : views_)
            {
                if (view->GetVisibility() == V_VISIBLE)
                {
                    if (dc.GetScissorBounds().Intersects(dc.TransformBounds(view->GetBounds())))
                    {
                        view->Draw(dc);
                    }
                }
            }

            if (clip_)
            {
                dc.PopScissor();
            }
        }

        void ViewGroup::Update()
        {
            View::Update();
            for (View* view : views_)
            {
                if (view->GetVisibility() != V_GONE)
                {
                    view->Update();
                }
            }
        }

        bool ViewGroup::SetFocus()
        {
            std::lock_guard<std::mutex> guard(modifyLock_);
            if (!CanBeFocused() && !views_.empty())
            {
                for (size_t i = 0; i < views_.size(); ++i)
                {
                    if (views_[i]->SetFocus())
                        return true;
                }
            }
            return false;
        }

        bool ViewGroup::SubviewFocused(View* view)
        {
            for (size_t i = 0; i < views_.size(); ++i)
            {
                if (views_[i] == view)
                    return true;
                if (views_[i]->SubviewFocused(view))
                    return true;
            }
            return false;
        }

        static float HorizontalOverlap(const Bounds& a, const Bounds& b)
        {
            if (a.x2() < b.x || b.x2() < a.x)
            {
                return 0.0f;
            }

            float maxMin = std::max(a.x, b.x);
            float minMax = std::min(a.x2(), b.x2());
            float overlap = minMax - maxMin;
            if (overlap < 0.0f)
            {
                return 0.0f;
            }
            else
            {
                return std::min(1.0f, overlap / std::min(a.w, b.w));
            }
        }

        static float VerticalOverlap(const Bounds& a, const Bounds& b)
        {
            if (a.y2() < b.y || b.y2() < a.y)
            {
                return 0.0f;
            }

            float maxMin = std::max(a.y, b.y);
            float minMax = std::min(a.y2(), b.y2());
            float overlap = minMax - maxMin;
            if (overlap < 0.0f)
            {
                return 0.0f;
            }
            else
            {
                return std::min(1.0f, overlap / std::min(a.h, b.h));
            }
        }

        float GetDirectionScore(View* origin, View* destination, FocusDirection direction)
        {
            if (!destination->CanBeFocused())
            {
                return 0.0f;
            }
            if (destination->IsEnabled() == false)
            {
                return 0.0f;
            }
            if (destination->GetVisibility() != V_VISIBLE)
            {
                return 0.0f;
            }

            Point originPos = origin->GetFocusPosition(direction);
            Point destPos = destination->GetFocusPosition(Opposite(direction));

            float dx = destPos.x - originPos.x;
            float dy = (destPos.y - originPos.y) * 10.0f;

            float distance = sqrtf(dx * dx + dy * dy);
            float overlap = 0.0f;
            float dirX = dx / distance;
            float dirY = dy / distance;

            bool wrongDirection = false;
            bool vertical = false;
            float horizOverlap = HorizontalOverlap(origin->GetBounds(), destination->GetBounds());
            float vertOverlap = VerticalOverlap(origin->GetBounds(), destination->GetBounds());
            if (horizOverlap == 1.0f && vertOverlap == 1.0f)
            {
                return 0.0;
            }
            float originSize = 0.0f;
            switch (direction)
            {
                case FOCUS_LEFT:
                    overlap = vertOverlap;
                    originSize = origin->GetBounds().w;
                    if (dirX > 0.0f)
                    {
                        wrongDirection = true;
                    }
                    break;
                case FOCUS_UP:
                    overlap = horizOverlap;
                    originSize = origin->GetBounds().h;
                    if (dirY > 0.0f)
                    {
                        wrongDirection = true;
                    }
                    vertical = true;
                    break;
                case FOCUS_RIGHT:
                    overlap = vertOverlap;
                    originSize = origin->GetBounds().w;
                    if (dirX < 0.0f)
                    {
                        wrongDirection = true;
                    }
                    break;
                case FOCUS_DOWN:
                    overlap = horizOverlap;
                    originSize = origin->GetBounds().h;
                    if (dirY < 0.0f)
                    {
                        wrongDirection = true;
                    }
                    vertical = true;
                    break;
                case FOCUS_PREV:
                case FOCUS_NEXT:
                    LOG_CORE_WARN("Invalid focus direction");
                    break;
            }

            float distanceBonus = 0.0f;
            if (vertical)
            {
                float widthDifference = origin->GetBounds().w - destination->GetBounds().w;
                if (widthDifference == 0)
                {
                    distanceBonus = 40.0f;
                }
            }
            else
            {
                float heightDifference = origin->GetBounds().h - destination->GetBounds().h;
                if (heightDifference == 0)
                {
                    distanceBonus = 40.0f;
                }
            }

            if (distance > 2 * originSize)
            {
                overlap = 0;
            }

            if (wrongDirection)
            {
                return 0.0f;
            }
            else
            {
                return 10.0f / std::max(1.0f, distance - distanceBonus) + overlap;
            }
        }

        NeighborResult ViewGroup::FindNeighbor(View* view, FocusDirection direction, NeighborResult result)
        {
            if (!IsEnabled())
            {
                return result;
            }
            if (GetVisibility() != V_VISIBLE)
            {
                return result;
            }

            int num = -1;
            for (size_t i = 0; i < views_.size(); ++i)
            {
                if (views_[i] == view)
                {
                    num = (int)i;
                    break;
                }
            }

            switch (direction)
            {
                case FOCUS_PREV:
                    if (num == -1)
                    {
                        return NeighborResult(0, 0.0f);
                    }
                    return NeighborResult(views_[(num + views_.size() - 1) % views_.size()], 0.0f);

                case FOCUS_NEXT:
                    if (num == -1)
                    {
                        return NeighborResult(0, 0.0f);
                    }
                    return NeighborResult(views_[(num + 1) % views_.size()], 0.0f);

                case FOCUS_UP:
                case FOCUS_LEFT:
                case FOCUS_RIGHT:
                case FOCUS_DOWN:
                {
                    for (size_t i = 0; i < views_.size(); ++i)
                    {
                        if (views_[i] == view)
                        {
                            continue;
                        }

                        float score = GetDirectionScore(view, views_[i], direction);
                        if (score > result.score)
                        {
                            result.score = score;
                            result.view = views_[i];
                        }
                    }

                    for (auto iter = views_.begin(); iter != views_.end(); ++iter)
                    {
                        if ((*iter)->IsViewGroup())
                        {
                            ViewGroup* vg = static_cast<ViewGroup*>(*iter);
                            if (vg)
                            {
                                result = vg->FindNeighbor(view, direction, result);
                            }
                        }
                    }
                    return result;
                }
                default:
                    return result;
            }
        }

        void LinearLayout::Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);
            MeasureBySpec(layoutParams_->height, 0.0f, vert, &measuredHeight_);

            if (views_.empty())
            {
                return;
            }

            float maxOther = 0.0f;
            float weightSum = 0.0f;
            float weightZeroSum = 0.0f;

            int numVisible = 0;

            for (View* view : views_)
            {
                if (view->GetVisibility() == V_GONE)
                {
                    continue;
                }
                ++numVisible;

                const LinearLayoutParams* linLayoutParams = view->GetLayoutParams()->As<LinearLayoutParams>();

                Margins margins = defaultMargins_;

                if (linLayoutParams)
                {
                    if (linLayoutParams->HasMargins())
                    {
                        margins = linLayoutParams->margins;
                    }
                }

                if (orientation_ == ORIENT_HORIZONTAL)
                {
                    MeasureSpec v = vert;
                    if (v.type == UNSPECIFIED && measuredHeight_ != 0.0f)
                    {
                        v = MeasureSpec(AT_MOST, measuredHeight_);
                    }
                    view->Measure(dc, MeasureSpec(UNSPECIFIED, measuredWidth_), v - (float)margins.vert());
                    if (horiz.type == AT_MOST && view->GetMeasuredWidth() + margins.horiz() > horiz.size - weightZeroSum)
                    {
                        view->Measure(dc, horiz, v - (float)margins.vert());
                    }
                }
                else if (orientation_ == ORIENT_VERTICAL)
                {
                    MeasureSpec h = horiz;
                    if (h.type == UNSPECIFIED && measuredWidth_ != 0.0f)
                    {
                        h = MeasureSpec(AT_MOST, measuredWidth_);
                    }
                    view->Measure(dc, h - (float)margins.horiz(), MeasureSpec(UNSPECIFIED, measuredHeight_));
                    if (vert.type == AT_MOST && view->GetMeasuredHeight() + margins.vert() > vert.size - weightZeroSum)
                    {
                        view->Measure(dc, h - (float)margins.horiz(), vert);
                    }
                }

                float amount;
                if (orientation_ == ORIENT_HORIZONTAL)
                {
                    amount = view->GetMeasuredWidth() + margins.horiz();
                    maxOther = std::max(maxOther, view->GetMeasuredHeight() + margins.vert());
                }
                else
                {
                    amount = view->GetMeasuredHeight() + margins.vert();
                    maxOther = std::max(maxOther, view->GetMeasuredWidth() + margins.horiz());
                }

                if (linLayoutParams)
                {
                    if (linLayoutParams->weight == 0.0f)
                        weightZeroSum += amount;

                    weightSum += linLayoutParams->weight;
                }
                else
                {
                    weightZeroSum += amount;
                }
            }

            weightZeroSum += spacing_ * (numVisible - 1);

            if (orientation_ == ORIENT_HORIZONTAL)
            {
                MeasureBySpec(layoutParams_->width, weightZeroSum, horiz, &measuredWidth_);

                float allowedWidth = measuredWidth_;
                if (horiz.type == AT_MOST && measuredWidth_ < horiz.size)
                {
                    allowedWidth = horiz.size;
                }

                float usedWidth = 0.0f;

                for (View* view : views_)
                {
                    if (view->GetVisibility() == V_GONE)
                    {
                        continue;
                    }
                    const LinearLayoutParams* linLayoutParams = view->GetLayoutParams()->As<LinearLayoutParams>();

                    if (linLayoutParams && linLayoutParams->weight > 0.0f)
                    {
                        Margins margins = defaultMargins_;
                        if (linLayoutParams->HasMargins())
                        {
                            margins = linLayoutParams->margins;
                        }
                        MeasureSpec v = vert;
                        if (v.type == UNSPECIFIED && measuredHeight_ != 0.0f)
                        {
                            v = MeasureSpec(AT_MOST, measuredHeight_);
                        }
                        float unit = (allowedWidth - weightZeroSum) / weightSum;
                        MeasureSpec h(AT_MOST, unit * linLayoutParams->weight - margins.horiz());
                        if (horiz.type == EXACTLY)
                        {
                            h.type = EXACTLY;
                        }
                        view->Measure(dc, h, v - (float)margins.vert());
                        usedWidth += view->GetMeasuredWidth();
                        maxOther = std::max(maxOther, view->GetMeasuredHeight() + margins.vert());
                    }
                }

                if (horiz.type == AT_MOST && measuredWidth_ < horiz.size)
                {
                    measuredWidth_ += usedWidth;
                }

                MeasureBySpec(layoutParams_->height, maxOther, vert, &measuredHeight_);
            }
            else
            {
                MeasureBySpec(layoutParams_->height, weightZeroSum, vert, &measuredHeight_);

                // If we've got stretch, allow growing to fill the parent.
                float allowedHeight = measuredHeight_;
                if (vert.type == AT_MOST && measuredHeight_ < vert.size)
                {
                    allowedHeight = vert.size;
                }

                float usedHeight = 0.0f;

                for (View* view : views_)
                {
                    if (view->GetVisibility() == V_GONE)
                    {
                        continue;
                    }
                    const LinearLayoutParams* linLayoutParams = view->GetLayoutParams()->As<LinearLayoutParams>();

                    if (linLayoutParams && linLayoutParams->weight > 0.0f)
                    {
                        Margins margins = defaultMargins_;
                        if (linLayoutParams->HasMargins())
                        {
                            margins = linLayoutParams->margins;
                        }
                        MeasureSpec h = horiz;
                        if (h.type == UNSPECIFIED && measuredWidth_ != 0.0f)
                        {
                            h = MeasureSpec(AT_MOST, measuredWidth_);
                        }
                        float unit = (allowedHeight - weightZeroSum) / weightSum;
                        MeasureSpec v(AT_MOST, unit * linLayoutParams->weight - margins.vert());
                        if (vert.type == EXACTLY)
                        {
                            v.type = EXACTLY;
                        }
                        view->Measure(dc, h - (float)margins.horiz(), v);
                        usedHeight += view->GetMeasuredHeight();
                        maxOther = std::max(maxOther, view->GetMeasuredWidth() + margins.horiz());
                    }
                }

                if (vert.type == AT_MOST && measuredHeight_ < vert.size)
                {
                    measuredHeight_ += usedHeight;
                }

                MeasureBySpec(layoutParams_->width, maxOther, horiz, &measuredWidth_);
            }
        }

        void LinearLayout::Layout()
        {
            const Bounds& bounds = bounds_;

            Bounds itemBounds;
            float pos;

            if (orientation_ == ORIENT_HORIZONTAL)
            {
                pos = bounds.x;
                itemBounds.y = bounds.y;
                itemBounds.h = measuredHeight_;
            }
            else
            {
                pos = bounds.y;
                itemBounds.x = bounds.x;
                itemBounds.w = measuredWidth_;
            }

            for (size_t i = 0; i < views_.size(); ++i)
            {
                if (views_[i]->GetVisibility() == V_GONE)
                {
                    continue;
                }

                const LinearLayoutParams* linLayoutParams = views_[i]->GetLayoutParams()->As<LinearLayoutParams>();

                Gravity gravity = G_TOPLEFT;
                Margins margins = defaultMargins_;
                if (linLayoutParams)
                {
                    if (linLayoutParams->HasMargins())
                        margins = linLayoutParams->margins;
                    gravity = linLayoutParams->gravity;
                }

                if (orientation_ == ORIENT_HORIZONTAL)
                {
                    itemBounds.x = pos;
                    itemBounds.w = views_[i]->GetMeasuredWidth() + margins.horiz();
                }
                else
                {
                    itemBounds.y = pos;
                    itemBounds.h = views_[i]->GetMeasuredHeight() + margins.vert();
                }

                Bounds innerBounds;
                ApplyGravity(itemBounds, margins, views_[i]->GetMeasuredWidth(), views_[i]->GetMeasuredHeight(), gravity,
                             innerBounds);

                views_[i]->SetBounds(innerBounds);
                views_[i]->Layout();

                pos += spacing_ + (orientation_ == ORIENT_HORIZONTAL ? itemBounds.w : itemBounds.h);
            }
        }

        //    void FrameLayout::Measure(const SCREEN_UIContext &dc, MeasureSpec horiz, MeasureSpec vert)
        //    {
        //        if (views_.empty()) {
        //            MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);
        //            MeasureBySpec(layoutParams_->height, 0.0f, vert, &measuredHeight_);
        //            return;
        //        }
        //
        //        for (size_t i = 0; i < views_.size(); i++)
        //        {
        //            if (views_[i]->GetVisibility() == V_GONE)
        //                continue;
        //            views_[i]->Measure(dc, horiz, vert);
        //        }
        //    }
        //
        //    void FrameLayout::Layout()
        //    {
        //        for (size_t i = 0; i < views_.size(); i++) {
        //            if (views_[i]->GetVisibility() == V_GONE)
        //                continue;
        //            float w = views_[i]->GetMeasuredWidth();
        //            float h = views_[i]->GetMeasuredHeight();
        //
        //            Bounds bounds;
        //            bounds.w = w;
        //            bounds.h = h;
        //
        //            bounds.x = bounds_.x + (measuredWidth_ - w) / 2;
        //            bounds.y = bounds_.y + (measuredWidth_ - h) / 2;
        //            views_[i]->SetBounds(bounds);
        //        }
        //    }
        //
        void ScrollView::Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            Margins margins;
            if (views_.size())
            {
                const LinearLayoutParams* linLayoutParams = views_[0]->GetLayoutParams()->As<LinearLayoutParams>();
                if (linLayoutParams)
                {
                    margins = linLayoutParams->margins;
                }
            }

            MeasureBySpec(layoutParams_->width, horiz.size, horiz, &measuredWidth_);
            MeasureBySpec(layoutParams_->height, vert.size, vert, &measuredHeight_);

            if (views_.size())
            {
                if (orientation_ == ORIENT_HORIZONTAL)
                {
                    MeasureSpec v = MeasureSpec(AT_MOST, measuredHeight_ - margins.vert());
                    if (measuredHeight_ == 0.0f && (vert.type == UNSPECIFIED || layoutParams_->height == WRAP_CONTENT))
                    {
                        v.type = UNSPECIFIED;
                    }
                    views_[0]->Measure(dc, MeasureSpec(UNSPECIFIED, measuredWidth_), v);
                    MeasureBySpec(layoutParams_->height, views_[0]->GetMeasuredHeight(), vert, &measuredHeight_);
                }
                else
                {
                    MeasureSpec h = MeasureSpec(AT_MOST, measuredWidth_ - margins.horiz());
                    if (measuredWidth_ == 0.0f && (horiz.type == UNSPECIFIED || layoutParams_->width == WRAP_CONTENT))
                    {
                        h.type = UNSPECIFIED;
                    }
                    views_[0]->Measure(dc, h, MeasureSpec(UNSPECIFIED, measuredHeight_));
                    MeasureBySpec(layoutParams_->width, views_[0]->GetMeasuredWidth(), horiz, &measuredWidth_);
                }
                if (orientation_ == ORIENT_VERTICAL && !vert_type_exactly_)
                {
                    if (measuredHeight_ < views_[0]->GetMeasuredHeight())
                    {
                        measuredHeight_ = views_[0]->GetMeasuredHeight();
                    }
                    if (measuredHeight_ < views_[0]->GetBounds().h)
                    {
                        measuredHeight_ = views_[0]->GetBounds().h;
                    }
                    if (vert.type == AT_MOST && measuredHeight_ > vert.size)
                    {
                        measuredHeight_ = vert.size;
                    }
                }
            }
        }

        void ScrollView::Layout()
        {
            if (!views_.size())
                return;
            Bounds scrolled;

            // Respect margins
            Margins margins;
            const LinearLayoutParams* linLayoutParams = views_[0]->GetLayoutParams()->As<LinearLayoutParams>();
            if (linLayoutParams)
            {
                margins = linLayoutParams->margins;
            }

            scrolled.w = views_[0]->GetMeasuredWidth() - margins.horiz();
            scrolled.h = views_[0]->GetMeasuredHeight() - margins.vert();

            float layoutScrollPos = ClampedScrollPos(scrollPos_);

            switch (orientation_)
            {
                case ORIENT_HORIZONTAL:
                    if (scrolled.w != lastViewSize_)
                    {
                        ScrollTo(0.0f);
                        lastViewSize_ = scrolled.w;
                    }
                    scrolled.x = bounds_.x - layoutScrollPos;
                    scrolled.y = bounds_.y + margins.top;
                    break;
                case ORIENT_VERTICAL:
                    if (scrolled.h != lastViewSize_ && scrollToTopOnSizeChange_)
                    {
                        ScrollTo(0.0f);
                        lastViewSize_ = scrolled.h;
                    }
                    scrolled.x = bounds_.x + margins.left;
                    scrolled.y = bounds_.y - layoutScrollPos;
                    break;
            }

            views_[0]->SetBounds(scrolled);
            views_[0]->Layout();
        }

        bool ScrollView::Key(const SCREEN_KeyInput& input)
        {
            if (visibility_ != V_VISIBLE)
                return ViewGroup::Key(input);

            if (input.flags & KEY_DOWN)
            {
                switch (input.keyCode)
                {
                    case NKCODE_EXT_MOUSEWHEEL_UP:
                        ScrollRelative(-250);
                        break;
                    case NKCODE_EXT_MOUSEWHEEL_DOWN:
                        ScrollRelative(250);
                        break;
                    case NKCODE_PAGE_DOWN:
                        ScrollRelative((orientation_ == ORIENT_VERTICAL ? bounds_.h : bounds_.w) - 50);
                        break;
                    case NKCODE_PAGE_UP:
                        ScrollRelative(-(orientation_ == ORIENT_VERTICAL ? bounds_.h : bounds_.w) + 50);
                        break;
                    case NKCODE_MOVE_HOME:
                        ScrollTo(0.0f);
                        break;
                    case NKCODE_MOVE_END:
                        if (views_.size())
                            ScrollTo(orientation_ == ORIENT_VERTICAL ? views_[0]->GetBounds().h : views_[0]->GetBounds().w);
                        break;
                }
            }
            return ViewGroup::Key(input);
        }

        bool ScrollView::Touch(const SCREEN_TouchInput& touch)
        {
            bool clicked = false;
            if ((touch.flags & TOUCH_WHEEL) && (visibility_ == V_VISIBLE))
            {
                if (touch.y < 0)
                {
                    ScrollRelative(55.0f);
                }
                else
                {
                    ScrollRelative(-55.0f);
                }
            }
            else
            {
                if ((!bounds_.Contains(touch.x, touch.y)) && (touch.flags & TOUCH_DOWN))
                {
                    return clicked;
                }
                return ViewGroup::Touch(touch);
            }
            return clicked;
        }

        const float friction = 0.92f;

        void ScrollView::Draw(SCREEN_UIContext& dc)
        {
            if (!views_.size())
            {
                ViewGroup::Draw(dc);
                return;
            }

            dc.PushScissor(bounds_);
            views_[0]->Draw(dc);
            dc.PopScissor();

            float childHeight = views_[0]->GetBounds().h;
            float scrollMax = std::max(0.0f, childHeight - bounds_.h);

            float ratio = bounds_.h / views_[0]->GetBounds().h;

            float bobWidth = 5;
            if (ratio < 1.0f && scrollMax > 0.0f)
            {
                float bobHeight = ratio * bounds_.h;
                float bobOffset = (ClampedScrollPos(scrollPos_) / scrollMax) * (bounds_.h - bobHeight);

                Bounds bob(bounds_.x2() - bobWidth, bounds_.y + bobOffset, bobWidth, bobHeight);
                dc.FillRect(Drawable(0x80FFFFFF), bob);
            }
        }

        bool ScrollView::SubviewFocused(View* view)
        {

            if (!ViewGroup::SubviewFocused(view))
                return false;

            const Bounds& vBounds = view->GetBounds();
            const float overscroll = std::min(view->GetBounds().h / 1.5f, GetBounds().h / 4.0f);

            float pos = ClampedScrollPos(scrollPos_);
            switch (orientation_)
            {
                case ORIENT_HORIZONTAL:
                    if (vBounds.x2() > bounds_.x2())
                    {
                        ScrollTo(pos + vBounds.x2() - bounds_.x2() + overscroll);
                    }
                    if (vBounds.x < bounds_.x)
                    {
                        ScrollTo(pos + (vBounds.x - bounds_.x) - overscroll);
                    }
                    break;
                case ORIENT_VERTICAL:
                    if (vBounds.y2() > bounds_.y2())
                    {
                        ScrollTo(pos + vBounds.y2() - bounds_.y2() + overscroll);
                    }
                    if (vBounds.y < bounds_.y)
                    {
                        ScrollTo(pos + (vBounds.y - bounds_.y) - overscroll);
                    }
                    break;
            }
            return true;
        }

        void ScrollView::PersistData(PersistStatus status, std::string anonId, PersistMap& storage)
        {
            ViewGroup::PersistData(status, anonId, storage);

            std::string tag = Tag();
            if (tag.empty())
            {
                tag = anonId;
            }

            PersistBuffer& buffer = storage["ScrollView::" + tag];
            switch (status)
            {
                case PERSIST_SAVE:
                {
                    buffer.resize(1);
                    float pos = scrollToTarget_ ? scrollTarget_ : scrollPos_;
                    buffer[0] = static_cast<int>(pos);
                }
                break;

                case PERSIST_RESTORE:
                    if (buffer.size() == 1)
                    {
                        float pos = *(float*)&buffer[0];
                        scrollPos_ = pos;
                        scrollTarget_ = pos;
                        scrollToTarget_ = false;
                    }
                    break;
            }
        }

        void ScrollView::SetVisibility(Visibility visibility)
        {
            ViewGroup::SetVisibility(visibility);

            if (visibility == V_GONE)
            {
                ScrollTo(0.0f);
            }
        }

        float ScrollView::GetScrollPosition() { return scrollPos_; }

        void ScrollView::ScrollTo(float newScrollPos)
        {
            scrollTarget_ = newScrollPos;
            scrollToTarget_ = true;
        }

        void ScrollView::ScrollRelative(float distance)
        {
            scrollTarget_ = scrollPos_ + distance;
            scrollToTarget_ = true;
        }

        float ScrollView::ClampedScrollPos(float pos)
        {

            if (!views_.size())
            {
                return 0.0f;
            }

            float childSize = orientation_ == ORIENT_VERTICAL ? views_[0]->GetBounds().h : views_[0]->GetBounds().w;
            float scrollMax = std::max(0.0f, childSize - (orientation_ == ORIENT_VERTICAL ? bounds_.h : bounds_.w));

            if (pos < 0.0f && pos < pull_)
            {
                pos = pull_;
            }
            if (pos > scrollMax && pos > scrollMax + pull_)
            {
                pos = scrollMax + pull_;
            }

            return pos;
        }

        void ScrollView::ScrollToBottom()
        {
            float childHeight = views_[0]->GetBounds().h;
            float scrollMax = std::max(0.0f, childHeight - bounds_.h);
            scrollPos_ = scrollMax;
            scrollTarget_ = scrollMax;
        }

        bool ScrollView::CanScroll() const
        {
            if (!views_.size())
                return false;
            switch (orientation_)
            {
                case ORIENT_VERTICAL:
                    return views_[0]->GetBounds().h > bounds_.h;
                case ORIENT_HORIZONTAL:
                    return views_[0]->GetBounds().w > bounds_.w;
                default:
                    return false;
            }
        }

        void ScrollView::Update()
        {
            if (visibility_ != V_VISIBLE)
            {
                inertia_ = 0.0f;
            }
            ViewGroup::Update();

            if (scrollToTarget_)
            {
                float target = ClampedScrollPos(scrollTarget_);

                inertia_ = 0.0f;
                if (fabsf(target - scrollPos_) < 0.5f)
                {
                    scrollPos_ = target;
                    scrollToTarget_ = false;
                }
                else
                {
                    scrollPos_ += (target - scrollPos_) * 0.3f;
                }
            }
            scrollPos_ = ClampedScrollPos(scrollPos_);

            pull_ *= friction;
            if (fabsf(pull_) < 0.01f)
            {
                pull_ = 0.0f;
            }
        }

        void AnchorLayout::Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);
            MeasureBySpec(layoutParams_->height, 0.0f, vert, &measuredHeight_);

            MeasureViews(dc, horiz, vert);

            const bool unspecifiedWidth = layoutParams_->width == WRAP_CONTENT && (overflow_ || horiz.type == UNSPECIFIED);
            const bool unspecifiedHeight = layoutParams_->height == WRAP_CONTENT && (overflow_ || vert.type == UNSPECIFIED);
            if (unspecifiedWidth || unspecifiedHeight)
            {
                MeasureSpec h = unspecifiedWidth ? MeasureSpec(AT_MOST, measuredWidth_) : horiz;
                MeasureSpec v = unspecifiedHeight ? MeasureSpec(AT_MOST, measuredHeight_) : vert;
                MeasureViews(dc, h, v);
            }
        }

        void AnchorLayout::MeasureViews(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            for (size_t i = 0; i < views_.size(); ++i)
            {
                Size width = WRAP_CONTENT;
                Size height = WRAP_CONTENT;

                MeasureSpec specW(UNSPECIFIED, measuredWidth_);
                MeasureSpec specH(UNSPECIFIED, measuredHeight_);

                if (!overflow_)
                {
                    if (horiz.type != UNSPECIFIED)
                    {
                        specW = MeasureSpec(AT_MOST, horiz.size);
                    }
                    if (vert.type != UNSPECIFIED)
                    {
                        specH = MeasureSpec(AT_MOST, vert.size);
                    }
                }

                const AnchorLayoutParams* params = views_[i]->GetLayoutParams()->As<AnchorLayoutParams>();
                if (params)
                {
                    width = params->width;
                    height = params->height;

                    if (!params->center)
                    {
                        if (params->left > NONE && params->right > NONE)
                        {
                            width = measuredWidth_ - params->left - params->right;
                        }
                        if (params->top > NONE && params->bottom > NONE)
                        {
                            height = measuredHeight_ - params->top - params->bottom;
                        }
                    }
                    if (width >= 0)
                    {
                        specW = MeasureSpec(EXACTLY, width);
                    }
                    if (height >= 0)
                    {
                        specH = MeasureSpec(EXACTLY, height);
                    }
                }

                views_[i]->Measure(dc, specW, specH);

                if (layoutParams_->width == WRAP_CONTENT)
                {
                    measuredWidth_ = std::max(measuredWidth_, views_[i]->GetMeasuredWidth());
                }
                if (layoutParams_->height == WRAP_CONTENT)
                {
                    measuredHeight_ = std::max(measuredHeight_, views_[i]->GetMeasuredHeight());
                }
            }
        }

        void AnchorLayout::Layout()
        {
            for (size_t i = 0; i < views_.size(); ++i)
            {
                const AnchorLayoutParams* params = views_[i]->GetLayoutParams()->As<AnchorLayoutParams>();

                Bounds vBounds;
                vBounds.w = views_[i]->GetMeasuredWidth();
                vBounds.h = views_[i]->GetMeasuredHeight();

                if (vBounds.w > bounds_.w)
                    vBounds.w = bounds_.w;
                if (vBounds.h > bounds_.h)
                    vBounds.h = bounds_.h;

                float left = 0, top = 0, right = 0, bottom = 0, center = false;
                if (params)
                {
                    left = params->left;
                    top = params->top;
                    right = params->right;
                    bottom = params->bottom;
                    center = params->center;
                }

                if (left > NONE)
                {
                    vBounds.x = bounds_.x + left;
                    if (center)
                    {
                        vBounds.x -= vBounds.w * 0.5f;
                    }
                }
                else if (right > NONE)
                {
                    vBounds.x = bounds_.x2() - right - vBounds.w;
                    if (center)
                    {
                        vBounds.x += vBounds.w * 0.5f;
                    }
                }

                if (top > NONE)
                {
                    vBounds.y = bounds_.y + top;
                    if (center)
                    {
                        vBounds.y -= vBounds.h * 0.5f;
                    }
                }
                else if (bottom > NONE)
                {
                    vBounds.y = bounds_.y2() - bottom - vBounds.h;
                    if (center)
                    {
                        vBounds.y += vBounds.h * 0.5f;
                    }
                }
                views_[i]->SetBounds(vBounds);
                views_[i]->Layout();
            }
        }

        GridLayout::GridLayout(GridLayoutSettings settings, LayoutParams* layoutParams)
            : ViewGroup(layoutParams), settings_(settings), numColumns_(1)
        {
        }

        void GridLayout::Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            MeasureSpecType measureType = settings_.fillCells ? EXACTLY : AT_MOST;

            for (size_t i = 0; i < views_.size(); ++i)
            {
                views_[i]->Measure(dc, MeasureSpec(measureType, settings_.columnWidth),
                                   MeasureSpec(measureType, settings_.rowHeight));
            }

            MeasureBySpec(layoutParams_->width, 0.0f, horiz, &measuredWidth_);

            numColumns_ = (measuredWidth_ - settings_.spacing) / (settings_.columnWidth + settings_.spacing);
            if (!numColumns_)
                numColumns_ = 1;
            int numRows = (int)(views_.size() + (numColumns_ - 1)) / numColumns_;

            float estimatedHeight = (settings_.rowHeight + settings_.spacing) * numRows;

            MeasureBySpec(layoutParams_->height, estimatedHeight, vert, &measuredHeight_);
        }

        void GridLayout::Layout()
        {
            int y = 0;
            int x = 0;
            int count = 0;
            for (size_t i = 0; i < views_.size(); ++i)
            {
                Bounds itemBounds, innerBounds;

                itemBounds.x = bounds_.x + x;
                itemBounds.y = bounds_.y + y;
                itemBounds.w = settings_.columnWidth;
                itemBounds.h = settings_.rowHeight;

                ApplyGravity(itemBounds, Margins(0.0f), views_[i]->GetMeasuredWidth(), views_[i]->GetMeasuredHeight(),
                             G_HCENTER | G_VCENTER, innerBounds);

                views_[i]->SetBounds(innerBounds);
                views_[i]->Layout();

                ++count;
                if (count == numColumns_)
                {
                    count = 0;
                    x = 0;
                    y += itemBounds.h + settings_.spacing;
                }
                else
                {
                    x += itemBounds.w + settings_.spacing;
                }
            }
        }

        TabHolder::TabHolder(Orientation orientation, float stripSize, LayoutParams* layoutParams, float leftMargin)
            : LinearLayout(Opposite(orientation), layoutParams)
        {
            if (orientation == ORIENT_HORIZONTAL)
            {
                LinearLayout* horizontalSpacer =
                    new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
                horizontalSpacer->SetTag("TabHolder::horizontalSpacer");
                horizontalSpacer->SetSpacing(0.0f);
                horizontalSpacer->Add(new Spacer(leftMargin, 0.0f));
                tabStrip_ = new ChoiceStrip(orientation, new LayoutParams(WRAP_CONTENT, WRAP_CONTENT));
                tabStrip_->SetTag("TabHolder::tabStrip_");
                tabStrip_->SetTopTabs(true);
                tabScroll_ = new ScrollView(orientation, new LayoutParams(FILL_PARENT, WRAP_CONTENT));
                tabScroll_->SetTag("TabHolder::tabScroll_");
                tabScroll_->Add(tabStrip_);
                horizontalSpacer->Add(tabScroll_);
                Add(horizontalSpacer);
            }
            else
            {
                tabStrip_ = new ChoiceStrip(orientation, new LayoutParams(stripSize, WRAP_CONTENT));
                tabStrip_->SetTopTabs(true);
                Add(tabStrip_);
            }
            tabStrip_->OnChoice.Handle(this, &TabHolder::OnTabClick);

            contents_ = new AnchorLayout(new LinearLayoutParams(FILL_PARENT, FILL_PARENT, 1.0f));
            contents_->SetTag("TabHolder::contents_");
            Add(contents_)->SetClip(true);
        }

        void TabHolder::AddTabContents(const std::string& title, View* tabContents)
        {
            tabContents->ReplaceLayoutParams(new AnchorLayoutParams(FILL_PARENT, FILL_PARENT));
            tabs_.push_back(tabContents);

            if (useIcons_)
            {
                tabStrip_->AddChoice(title, m_Icon, m_Icon_active, m_Icon_depressed, m_Icon_depressed_inactive, title);
            }
            else
            {
                tabStrip_->AddChoice(title);
            }

            contents_->Add(tabContents);
            if (tabs_.size() > 1)
            {
                tabContents->SetVisibility(V_GONE);
            }

            tabTweens_.push_back(nullptr);
        }

        void TabHolder::SetCurrentTab(int tab, bool skipTween)
        {
            if (tab >= (int)tabs_.size())
            {
                return;
            }

            auto setupTween = [&](View* view, AnchorTranslateTween*& tween)
            {
                if (tween)
                {
                    return;
                }

                tween = new AnchorTranslateTween(0.15f, bezierEaseInOut);
                tween->Finish.Add(
                    [&](EventParams& e)
                    {
                        e.v->SetVisibility(tabs_[currentTab_] == e.v ? V_VISIBLE : V_GONE);
                        return EVENT_DONE;
                    });
                view->AddTween(tween)->Persist();
            };

            if (tab != currentTab_)
            {
                Orientation orient = Opposite(orientation_);
                // Direction from which the new tab will come.
                float dir = tab < currentTab_ ? -1.0f : 1.0f;

                // First, setup any missing tweens.
                setupTween(tabs_[currentTab_], tabTweens_[currentTab_]);
                setupTween(tabs_[tab], tabTweens_[tab]);

                // Currently displayed, so let's reset it.
                if (skipTween)
                {
                    tabs_[currentTab_]->SetVisibility(V_GONE);
                    tabTweens_[tab]->Reset(Point(0.0f, 0.0f));
                    tabTweens_[tab]->Apply(tabs_[tab]);
                }
                else
                {
                    tabTweens_[currentTab_]->Reset(Point(0.0f, 0.0f));

                    if (orient == ORIENT_HORIZONTAL)
                    {
                        tabTweens_[tab]->Reset(Point(bounds_.w * dir, 0.0f));
                        tabTweens_[currentTab_]->Divert(Point(bounds_.w * -dir, 0.0f));
                    }
                    else
                    {
                        tabTweens_[tab]->Reset(Point(0.0f, bounds_.h * dir));
                        tabTweens_[currentTab_]->Divert(Point(0.0f, bounds_.h * -dir));
                    }
                    // Actually move it to the initial position now, just to avoid any flicker.
                    tabTweens_[tab]->Apply(tabs_[tab]);
                    tabTweens_[tab]->Divert(Point(0.0f, 0.0f));
                }
                tabs_[tab]->SetVisibility(V_VISIBLE);

                currentTab_ = tab;
            }
            tabStrip_->SetSelection(tab);
        }

        EventReturn TabHolder::OnTabClick(EventParams& e)
        {
            if (e.b != 0)
            {
                SetCurrentTab((int)e.a);
            }
            return EVENT_DONE;
        }

        void TabHolder::PersistData(PersistStatus status, std::string anonId, PersistMap& storage)
        {
            ViewGroup::PersistData(status, anonId, storage);

            std::string tag = Tag();
            if (tag.empty())
            {
                tag = anonId;
            }

            PersistBuffer& buffer = storage["TabHolder::" + tag];
            switch (status)
            {
                case PERSIST_SAVE:
                    buffer.resize(1);
                    buffer[0] = currentTab_;
                    break;

                case PERSIST_RESTORE:
                    if (buffer.size() == 1)
                    {
                        SetCurrentTab(buffer[0], true);
                    }
                    break;
            }
        }

        void TabHolder::SetIcon(const Sprite& icon, const Sprite& icon_active, const Sprite& icon_depressed,
                                const Sprite& icon_depressed_inactive)
        {
            m_Icon = icon;
            m_Icon_active = icon_active;
            m_Icon_depressed = icon_depressed;
            m_Icon_depressed_inactive = icon_depressed_inactive;
            useIcons_ = true;
        }

        bool TabHolder::HasFocus(int& tab) { return tabStrip_->AnyTabHasFocus(tab); }

        void TabHolder::enableAllTabs() { tabStrip_->enableAllTabs(); }

        void TabHolder::disableAllTabs() { tabStrip_->disableAllTabs(); }

        void TabHolder::SetEnabled(int tab) { tabStrip_->SetEnabled(tab); }

        ChoiceStrip::ChoiceStrip(Orientation orientation, LayoutParams* layoutParams)
            : LinearLayout(orientation, layoutParams), selected_(0), topTabs_(false)
        {
            SetSpacing(0.0f);
        }

        void ChoiceStrip::enableAllTabs()
        {
            for (unsigned int choice = 0; choice < (unsigned int)views_.size(); ++choice)
            {
                Choice(choice)->SetEnabled(true);
            }
        }

        void ChoiceStrip::disableAllTabs()
        {
            for (unsigned int choice = 0; choice < (unsigned int)views_.size(); ++choice)
            {
                Choice(choice)->SetEnabled(false);
            }
        }

        void ChoiceStrip::SetEnabled(int tab) { Choice(tab)->SetEnabled(true); }

        bool ChoiceStrip::AnyTabHasFocus(int& tab)
        {
            bool anyTabHasFocus = false;
            for (unsigned int choice = 0; choice < (unsigned int)views_.size(); ++choice)
            {
                if (Choice(choice)->HasFocus())
                {
                    tab = choice;
                    anyTabHasFocus = true;
                    break;
                }
            }
            return anyTabHasFocus;
        }

        void ChoiceStrip::AddChoice(const std::string& title)
        {
            StickyChoice* c = new StickyChoice(
                title, "", orientation_ == ORIENT_HORIZONTAL ? nullptr : new LinearLayoutParams(FILL_PARENT, ITEM_HEIGHT));
            c->OnClick.Handle(this, &ChoiceStrip::OnChoiceClick);
            c->SetTag("ChoiceStrip::c");
            Add(c);
            if (selected_ == (int)views_.size() - 1)
                c->Press();
        }

        void ChoiceStrip::AddChoice(const std::string& title, const Sprite& icon, const Sprite& icon_active,
                                    const Sprite& icon_depressed, const Sprite& icon_depressed_inactive,
                                    const std::string& text)
        {
            StickyChoice* c = new StickyChoice(
                icon, icon_active, icon_depressed, icon_depressed_inactive, text,
                orientation_ == ORIENT_HORIZONTAL ? nullptr : new LinearLayoutParams(FILL_PARENT, ITEM_HEIGHT));
            c->OnClick.Handle(this, &ChoiceStrip::OnChoiceClick);
            c->SetCentered(true);
            c->SetTag("ChoiceStrip::c");
            Add(c);
            if (selected_ == (int)views_.size() - 1)
                c->Press();
        }

        bool ChoiceStrip::Touch(const SCREEN_TouchInput& input)
        {
            bool clicked = false;
            for (unsigned int choice = 0; choice < (unsigned int)views_.size(); ++choice)
            {
                bool isEnabled = Choice(choice)->IsEnabled();
                Choice(choice)->SetEnabled(true);
                clicked |= Choice(choice)->Touch(input);
                Choice(choice)->SetEnabled(isEnabled);
            }
            return clicked;
        }

        EventReturn ChoiceStrip::OnChoiceClick(EventParams& e)
        {
            for (int i = 0; i < (int)views_.size(); ++i)
            {
                if (views_[i] != e.v)
                {
                    Choice(i)->Release();
                }
                else
                {
                    selected_ = i;
                }
            }

            EventParams e2{};
            e2.v = views_[selected_];
            e2.a = selected_;
            e2.b = 1;
            return OnChoice.Dispatch(e2);
        }

        void ChoiceStrip::SetSelection(int sel)
        {
            int prevSelected = selected_;
            StickyChoice* prevChoice = Choice(selected_);
            if (prevChoice)
            {
                prevChoice->Release();
            }
            selected_ = sel;
            StickyChoice* newChoice = Choice(selected_);
            if (newChoice)
            {
                newChoice->Press();

                if (topTabs_ && prevSelected != selected_)
                {
                    EventParams e{};
                    e.v = views_[selected_];
                    e.a = selected_;
                    e.b = 0;
                    OnChoice.Trigger(e);
                }
            }
        }

        void ChoiceStrip::HighlightChoice(unsigned int choice)
        {
            if (choice < (unsigned int)views_.size())
            {
                Choice(choice)->HighlightChanged(true);
            }
        }

        bool ChoiceStrip::Key(const SCREEN_KeyInput& input)
        {
            bool ret = false;
            if (input.flags & KEY_DOWN)
            {
                if (IsTabLeftKey(input))
                {
                    if (selected_ > 0)
                    {
                        SetSelection(selected_ - 1);
                    }
                    ret = true;
                }
                else if (IsTabRightKey(input))
                {
                    if (selected_ < (int)views_.size() - 1)
                    {
                        SetSelection(selected_ + 1);
                    }
                    ret = true;
                }
            }
            return ret || ViewGroup::Key(input);
        }

        void ChoiceStrip::Draw(SCREEN_UIContext& dc)
        {
            ViewGroup::Draw(dc);
            if (topTabs_ && CoreSettings::m_UITheme != THEME_RETRO)
            {
                if (orientation_ == ORIENT_HORIZONTAL)
                {
                    dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x, bounds_.y2() - 4.0f, bounds_.x2(),
                                                bounds_.y2(), dc.theme->itemDownStyle.background.color);
                }
                else if (orientation_ == ORIENT_VERTICAL)
                {
                    dc.Draw()->DrawImageStretch(dc.theme->whiteImage, bounds_.x2() - 4.0f, bounds_.y, bounds_.x2(),
                                                bounds_.y2(), dc.theme->itemDownStyle.background.color);
                }
            }
        }

        StickyChoice* ChoiceStrip::Choice(int index)
        {
            if ((size_t)index < views_.size())
            {
                return static_cast<StickyChoice*>(views_[index]);
            }
            return nullptr;
        }

        ListView::ListView(ListAdaptor* a, float popupWidth, std::set<int> hidden, LayoutParams* layoutParams)
            : ScrollView(ORIENT_VERTICAL, layoutParams), adaptor_(a), maxHeight_(0), hidden_(hidden), m_Width(popupWidth)
        {

            linLayout_ = new LinearLayout(ORIENT_VERTICAL);
            linLayout_->SetSpacing(0.0f);
            Add(linLayout_);
            CreateAllItems();
        }

        void ListView::CreateAllItems()
        {
            linLayout_->Clear();
            for (int i = 0; i < adaptor_->GetNumItems(); ++i)
            {
                if (hidden_.find(i) == hidden_.end())
                {
                    View* v = linLayout_->Add(adaptor_->CreateItemView(i, m_Width));
                    adaptor_->AddEventCallback(v, std::bind(&ListView::OnItemCallback, this, i, std::placeholders::_1));
                }
            }
        }

        void ListView::Measure(const SCREEN_UIContext& dc, MeasureSpec horiz, MeasureSpec vert)
        {
            ScrollView::Measure(dc, horiz, vert);
            if (maxHeight_ > 0 && measuredHeight_ > maxHeight_)
            {
                measuredHeight_ = maxHeight_;
            }
        }

        EventReturn ListView::OnItemCallback(int num, EventParams& e)
        {
            EventParams ev{};
            ev.v = nullptr;
            ev.a = num;
            adaptor_->SetSelected(num);
            OnChoice.Trigger(ev);
            CreateAllItems();
            return EVENT_DONE;
        }

        //    View *ChoiceListAdaptor::CreateItemView(int index)
        //    {
        //        return new Choice(items_[index]);
        //    }
        //
        //    bool ChoiceListAdaptor::AddEventCallback(View *view, std::function<EventReturn(EventParams&)> callback)
        //    {
        //        Choice *choice = (Choice *)view;
        //        choice->OnClick.Add(callback);
        //        return EVENT_DONE;
        //    }

#define TRANSPARENT_BACKGROUND true
        View* StringVectorListAdaptor::CreateItemView(int index, float width)
        {
            if (CoreSettings::m_UITheme == THEME_RETRO)
            {
                return new Choice(items_[index], TRANSPARENT_BACKGROUND, "", index == selected_,
                                  new LinearLayoutParams(width, 64.0f));
            }
            else
            {
                return new Choice(items_[index], "", index == selected_, new LinearLayoutParams(width, 64.0f));
            }
        }

        bool StringVectorListAdaptor::AddEventCallback(View* view, std::function<EventReturn(EventParams&)> callback)
        {
            Choice* choice = (Choice*)view;
            choice->OnClick.Add(callback);
            return EVENT_DONE;
        }

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
