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

#include "gui/Common/UI/tween.h"
#include "gui/Common/UI/viewGroup.h"

namespace GfxRenderEngine
{

    template <typename T> static T clamp(T f, T low, T high)
    {
        if (f < low)
        {
            return low;
        }
        if (f > high)
        {
            return high;
        }
        return f;
    }

    uint32_t colorBlend(uint32_t rgb1, uint32_t rgb2, float alpha)
    {
        float invAlpha = (1.0f - alpha);
        int r = (int)(((rgb1 >> 0) & 0xFF) * alpha + ((rgb2 >> 0) & 0xFF) * invAlpha);
        int g = (int)(((rgb1 >> 8) & 0xFF) * alpha + ((rgb2 >> 8) & 0xFF) * invAlpha);
        int b = (int)(((rgb1 >> 16) & 0xFF) * alpha + ((rgb2 >> 16) & 0xFF) * invAlpha);
        int a = (int)(((rgb1 >> 24) & 0xFF) * alpha + ((rgb2 >> 24) & 0xFF) * invAlpha);

        uint32_t c = clamp(a, 0, 255) << 24;
        c |= clamp(b, 0, 255) << 16;
        c |= clamp(g, 0, 255) << 8;
        c |= clamp(r, 0, 255);
        return c;
    }

    namespace SCREEN_UI
    {
        void Tween::Apply(View* view)
        {
            if (!valid_)
            {
                return;
            }

            if (DurationOffset() >= duration_)
            {
                finishApplied_ = true;
            }

            float pos = Position();
            DoApply(view, pos);

            if (finishApplied_)
            {
                SCREEN_UI::EventParams e{};
                e.v = view;
                e.f = DurationOffset() - duration_;
                Finish.Trigger(e);
            }
        }

        template <typename Value>
        void TweenBase<Value>::PersistData(PersistStatus status, std::string anonId, PersistMap& storage)
        {
            struct TweenData
            {
                float start;
                float duration;
                float delay;
                Value from;
                Value to;
                bool valid;
            };

            PersistBuffer& buffer = storage["TweenBase::" + anonId];

            switch (status)
            {
                case SCREEN_UI::PERSIST_SAVE:
                    buffer.resize(sizeof(TweenData) / sizeof(int));
                    {
                        TweenData& data = *(TweenData*)&buffer[0];
                        data.start = start_;
                        data.duration = duration_;
                        data.delay = delay_;
                        data.from = from_;
                        data.to = to_;
                        data.valid = valid_;
                    }
                    break;
                case SCREEN_UI::PERSIST_RESTORE:
                    if (buffer.size() >= sizeof(TweenData) / sizeof(int))
                    {
                        TweenData data = *(TweenData*)&buffer[0];
                        start_ = data.start;
                        duration_ = data.duration;
                        delay_ = data.delay;
                        from_ = data.from;
                        to_ = data.to;
                        valid_ = data.valid;
                    }
                    break;
            }
        }

        template void TweenBase<uint32_t>::PersistData(PersistStatus status, std::string anonId, PersistMap& storage);
        template void TweenBase<Visibility>::PersistData(PersistStatus status, std::string anonId, PersistMap& storage);
        template void TweenBase<Point>::PersistData(PersistStatus status, std::string anonId, PersistMap& storage);

        uint32_t ColorTween::Current(float pos) { return colorBlend(to_, from_, pos); }

        //    void TextColorTween::DoApply(View *view, float pos)
        //    {
        //        TextView *tv = (TextView *)view;
        //        tv->SetTextColor(Current(pos));
        //    }

        void CallbackColorTween::DoApply(View* view, float pos)
        {
            if (callback_)
            {
                callback_(view, Current(pos));
            }
        }

        //    void VisibilityTween::DoApply(View *view, float pos)
        //    {
        //        view->SetVisibility(Current(pos));
        //    }
        //
        //    Visibility VisibilityTween::Current(float p)
        //    {
        //        if (from_ == V_VISIBLE && p < 1.0f)
        //            return from_;
        //        if (to_ == V_VISIBLE && p > 0.0f)
        //            return to_;
        //        return p >= 1.0f ? to_ : from_;
        //    }

        void AnchorTranslateTween::DoApply(View* view, float pos)
        {
            Point cur = Current(pos);

            auto prev = view->GetLayoutParams()->As<AnchorLayoutParams>();
            auto lp = new AnchorLayoutParams(prev ? *prev : AnchorLayoutParams(FILL_PARENT, FILL_PARENT));
            lp->left = cur.x;
            lp->top = cur.y;
            view->ReplaceLayoutParams(lp);
        }

        Point AnchorTranslateTween::Current(float p)
        {
            float inv = 1.0f - p;
            return Point(from_.x * inv + to_.x * p, from_.y * inv + to_.y * p);
        }

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
