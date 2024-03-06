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

#include "core.h"
#include "gui/Common/UI/view.h"

namespace GfxRenderEngine
{

    namespace SCREEN_UI
    {
        class Tween
        {
        public:
            explicit Tween(float duration, float (*curve)(float)) : duration_(duration), curve_(curve)
            {
                start_ = Engine::m_Engine->GetTimeDouble();
            }
            virtual ~Tween() {}

            void Apply(View* view);

            bool Finished() { return finishApplied_ && Engine::m_Engine->GetTimeDouble() >= start_ + delay_ + duration_; }

            void Persist() { persists_ = true; }
            bool Persists() { return persists_; }

            void Delay(float s) { delay_ = s; }

            virtual void PersistData(PersistStatus status, std::string anonId, PersistMap& storage) = 0;

            Event Finish;

        protected:
            float DurationOffset() { return (Engine::m_Engine->GetTimeDouble() - start_) - delay_; }

            float Position() { return curve_(std::min(1.0f, DurationOffset() / duration_)); }

            virtual void DoApply(View* view, float pos) = 0;

            double start_;
            float duration_;
            float delay_ = 0.0f;
            bool finishApplied_ = false;
            bool persists_ = false;
            bool valid_ = false;
            float (*curve_)(float);
        };

        template <typename Value> class TweenBase : public Tween
        {
        public:
            TweenBase(
                float duration, float (*curve)(float) = [](float f) { return f; })
                : Tween(duration, curve)
            {
            }
            TweenBase(
                Value from, Value to, float duration, float (*curve)(float) = [](float f) { return f; })
                : Tween(duration, curve), from_(from), to_(to)
            {
                valid_ = true;
            }

            void Divert(const Value& newTo, float newDuration = -1.0f)
            {
                const Value newFrom = valid_ ? Current(Position()) : newTo;

                if (Engine::m_Engine->GetTimeDouble() < start_ + delay_ + duration_ && valid_)
                {
                    if (newTo == to_)
                    {
                        return;
                    }
                    else if (newTo == from_ && duration_ > 0.0f)
                    {
                        float newOffset = duration_ - std::max(0.0f, DurationOffset());
                        if (newDuration >= 0.0f)
                        {
                            newOffset *= newDuration / duration_;
                        }
                        start_ = Engine::m_Engine->GetTimeDouble() - newOffset - delay_;
                    }
                    else if (Engine::m_Engine->GetTimeDouble() <= start_ + delay_)
                    {
                        start_ = Engine::m_Engine->GetTimeDouble();
                    }
                    else
                    {
                        start_ = Engine::m_Engine->GetTimeDouble() - delay_;
                    }
                }
                else
                {
                    start_ = Engine::m_Engine->GetTimeDouble();
                    finishApplied_ = false;
                }

                from_ = newFrom;
                to_ = newTo;
                valid_ = true;
                if (newDuration >= 0.0f)
                {
                    duration_ = newDuration;
                }
            }

            void Stop() { Reset(Current(Position())); }

            void Reset(const Value& newFrom)
            {
                from_ = newFrom;
                to_ = newFrom;
                valid_ = true;
            }

            const Value& FromValue() const { return from_; }
            const Value& ToValue() const { return to_; }
            Value CurrentValue() { return Current(Position()); }

            void PersistData(PersistStatus status, std::string anonId, PersistMap& storage) override;

        protected:
            virtual Value Current(float pos) = 0;

            Value from_;
            Value to_;
        };

        class ColorTween : public TweenBase<uint32_t>
        {
        public:
            using TweenBase::TweenBase;

        protected:
            uint32_t Current(float pos) override;
        };

        class TextColorTween : public ColorTween
        {
        public:
            using ColorTween::ColorTween;

        protected:
            void DoApply(View* view, float pos) override;
        };

        class CallbackColorTween : public ColorTween
        {
        public:
            using ColorTween::ColorTween;

            void SetCallback(const std::function<void(View* v, uint32_t c)>& cb) { callback_ = cb; }

        protected:
            void DoApply(View* view, float pos) override;

            std::function<void(View* v, uint32_t c)> callback_;
        };

        class VisibilityTween : public TweenBase<Visibility>
        {
        public:
            using TweenBase::TweenBase;

        protected:
            void DoApply(View* view, float pos) override;

            Visibility Current(float pos) override;
        };

        class AnchorTranslateTween : public TweenBase<Point>
        {
        public:
            using TweenBase::TweenBase;

        protected:
            void DoApply(View* view, float pos) override;

            Point Current(float pos) override;
        };

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
