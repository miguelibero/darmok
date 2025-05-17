#pragma once

#include <darmok/easing_fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <unordered_map>
#include <string>
#include <optional>

namespace darmok
{
   struct Easing final
    {
        using Type = EasingType;

        template<typename T>
        static constexpr T stepped(float position, T start, T end)
        {
            return start;
        }

        template<typename T>
        static constexpr T linear(float position, T start, T end)
        {
            return static_cast<T>((end - start) * position + start);
        }

        template<typename T>
        static constexpr T quadraticIn(float position, T start, T end)
        {
            return static_cast<T>((end - start) * position * position + start);
        }

        template<typename T>
        static constexpr T quadraticOut(float position, T start, T end)
        {
            return static_cast<T>((-(end - start)) * position * (position - 2) + start);
        }

        template<typename T>
        static constexpr T quadraticInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>(((end - start) / 2) * position * position + start);
            }

            --position;
            return static_cast<T>((-(end - start) / 2) * (position * (position - 2) - 1) + start);
        }

        template<typename T>
        static constexpr T cubicIn(float position, T start, T end)
        {
            return static_cast<T>((end - start) * position * position * position + start);
        }

        template<typename T>
        static constexpr T cubicOut(float position, T start, T end)
        {
            --position;
            return static_cast<T>((end - start) * (position * position * position + 1) + start);
        }

        template<typename T>
        static constexpr T cubicInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>(((end - start) / 2) * position * position * position + start);
            }
            position -= 2;
            return static_cast<T>(((end - start) / 2) * (position * position * position + 2) + start);
        }

        template<typename T>
        static constexpr T quarticIn(float position, T start, T end)
        {
            return static_cast<T>((end - start) * position * position * position * position + start);
        }

        template<typename T>
        static constexpr T quarticOut(float position, T start, T end)
        {
            --position;
            return static_cast<T>( -(end - start) * (position * position * position * position - 1) + start);
        }

        template<typename T>
        static constexpr T quarticInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>(((end - start) / 2) * (position * position * position * position) +
                                        start);
            }
            position -= 2;
            return static_cast<T>((-(end - start) / 2) * (position * position * position * position - 2) +
                                    start);
        }

        template<typename T>
        static constexpr T quinticIn(float position, T start, T end)
        {
            return static_cast<T>((end - start) * position * position * position * position * position + start);
        }

        template<typename T>
        static constexpr T quinticOut(float position, T start, T end)
        {
            position--;
            return static_cast<T>((end - start) * (position * position * position * position * position + 1) +
                                        start);
        }

        template<typename T>
        static constexpr T quinticInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>(
                    ((end - start) / 2) * (position * position * position * position * position) +
                    start);
            }
            position -= 2;
            return static_cast<T>(
                ((end - start) / 2) * (position * position * position * position * position + 2) +
                start);
        }

        template<typename T>
        static constexpr T sinusoidalIn(float position, T start, T end)
        {
            return static_cast<T>(-(end - start) * std::cosf(position * glm::half_pi<float>()) + (end - start) + start);
        }

        template<typename T>
        static constexpr T sinusoidalOut(float position, T start, T end)
        {
            return static_cast<T>((end - start) * std::sinf(position * glm::half_pi<float>()) + start);
        }

        template<typename T>
        static constexpr T sinusoidalInOut(float position, T start, T end)
        {
            return static_cast<T>((-(end - start) / 2) * (std::cosf(position * glm::pi<float>()) - 1) + start);
        }

        template<typename T>
        static constexpr T exponentialIn(float position, T start, T end)
        {
            return static_cast<T>((end - start) * std::powf(2, 10 * (position - 1)) + start);
        }

        template<typename T>
        static constexpr T exponentialOut(float position, T start, T end)
        {
            return static_cast<T>((end - start) * (-std::powf(2, -10 * position) + 1) + start);
        }

        template<typename T>
        static constexpr T exponentialInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>(((end - start) / 2) * std::powf(2, 10 * (position - 1)) + start);
            }
            --position;
            return static_cast<T>(((end - start) / 2) * (-std::powf(2, -10 * position) + 2) + start);
        }

        template<typename T>
        static constexpr T circularIn(float position, T start, T end)
        {
            return static_cast<T>( -(end - start) * (sqrtf(1 - position * position) - 1) + start );
        }

        template<typename T>
        static constexpr T circularOut(float position, T start, T end)
        {
            --position;
            return static_cast<T>((end - start) * (sqrtf(1 - position * position)) + start);
        }

        template<typename T>
        static constexpr T circularInOut(float position, T start, T end)
        {
            position *= 2;
            if (position < 1)
            {
                return static_cast<T>((-(end - start) / 2) * (sqrtf(1 - position * position) - 1) + start);
            }

            position -= 2;
            return static_cast<T>(((end - start) / 2) * (sqrtf(1 - position * position) + 1) + start);
        }

        template<typename T>
        static constexpr T bounceIn(float position, T start, T end)
        {
            return (end - start) - bounceOut((1 - position), T(), (end - start)) + start;
        }

        template<typename T>
        static constexpr T bounceOut(float position, T start, T end)
        {
            T c = end - start;
            if (position < (1 / 2.75f))
            {
                return static_cast<T>(c * (7.5625f * position * position) + start);
            }
            else if (position < (2.0f / 2.75f))
            {
                float postFix = position -= (1.5f / 2.75f);
                return static_cast<T>(c * (7.5625f * (postFix) * position + .75f) + start);
            }
            else if (position < (2.5f / 2.75f))
            {
                float postFix = position -= (2.25f / 2.75f);
                return static_cast<T>(c * (7.5625f * (postFix) * position + .9375f) + start);
            }
            else
            {
                float postFix = position -= (2.625f / 2.75f);
                return static_cast<T>(c * (7.5625f * (postFix) * position + .984375f) + start);
            }
        }

        template<typename T>
        static constexpr T bounceInOut(float position, T start, T end)
        {
            if (position < 0.5f)
            {
                return static_cast<T>(bounceIn(position * 2, T(), (end - start)) * .5f + start);
            }
            return static_cast<T>(bounceOut((position * 2 - 1), T(), (end - start)) * .5f + (end - start) * .5f + start);
        }

        template<typename T>
        static constexpr T elasticIn(float position, T start, T end)
        {
            if (position <= 0.00001f)
            {
                return start;
            }
            if (position >= 0.999f)
            {
                return end;
            }
            float p = .3f;
            auto a = end - start;
            float s = p / 4;
            float postFix = a * std::powf(2, 10 * (position -= 1));
            return static_cast<T>(-(postFix * std::sinf((position - s) * glm::two_pi<float>() / p)) + start);
        }

        template<typename T>
        static constexpr T elasticOut(float position, T start, T end)
        {
            if (position <= 0.00001f)
            {
                return start;
            }
            if (position >= 0.999f)
            {
                return end;
            }
            float p = .3f;
            auto a = end - start;
            float s = p / 4;
            return static_cast<T>(a * std::powf(2, -10 * position) * std::sinf((position - s) * glm::two_pi<float>() / p) + end);
        }

        template<typename T>
        static constexpr T elasticInOut(float position, T start, T end)
        {
            if (position <= 0.00001f)
            {
                return start;
            }
            if (position >= 0.999f)
            {
                return end;
            }
            position *= 2;
            float p = (.3f * 1.5f);
            auto a = end - start;
            float s = p / 4;
            float postFix;

            if (position < 1)
            {
                postFix = a * std::powf(2, 10 * (position -= 1));
                return static_cast<T>(-0.5f * (postFix * std::sinf((position - s) * glm::two_pi<float>() / p)) + start);
            }
            postFix = a * std::powf(2, -10 * (position -= 1));
            return static_cast<T>(postFix * std::sinf((position - s) * glm::two_pi<float>() / p) * .5f + end);
        }

        template<typename T>
        static constexpr T backIn(float position, T start, T end)
        {
            float s = 1.70158f;
            float postFix = position;
            return static_cast<T>((end - start) * (postFix) * position * ((s + 1) * position - s) + start);
        }

        template<typename T>
        static constexpr T backOut(float position, T start, T end)
        {
            float s = 1.70158f;
            position -= 1;
            return static_cast<T>((end - start) * ((position) * position * ((s + 1) * position + s) + 1) + start);
        }

        template<typename T>
        static constexpr T backInOut(float position, T start, T end)
        {
            float s = 1.70158f;
            float t = position;
            auto b = start;
            auto c = end - start;
            float d = 1;
            s *= (1.525f);
            if ((t /= d / 2) < 1)
            {
                return static_cast<T>(c / 2 * (t * t * (((s) + 1) * t - s)) + b);
            }
            float postFix = t -= 2;
            return static_cast<T>(c / 2 * ((postFix) * t * (((s) + 1) * t + s) + 2) + b);
        }


        static std::string_view getTypeName(Type type) noexcept;
        static std::optional<Type> readType(std::string_view name) noexcept;

        template<typename T>
        static constexpr T apply(Type type, float position, T start, T end)
        {
            switch (type)
            {
                case Type::Linear:
                    return linear(position, start, end);
                case Type::Stepped:
                    return stepped(position, start, end);
                case Type::QuadraticIn:
                    return quadraticIn(position, start, end);
                case Type::QuadraticOut:
                    return quadraticOut(position, start, end);
                case Type::QuadraticInOut:
                    return quadraticInOut(position, start, end);
                case Type::CubicIn:
                    return cubicIn(position, start, end);
                case Type::CubicOut:
                    return cubicOut(position, start, end);
                case Type::CubicInOut:
                    return cubicInOut(position, start, end);
                case Type::QuarticIn:
                    return quarticIn(position, start, end);
                case Type::QuarticOut:
                    return quarticOut(position, start, end);
                case Type::QuarticInOut:
                    return quarticInOut(position, start, end);
                case Type::QuinticIn:
                    return quinticIn(position, start, end);
                case Type::QuinticOut:
                    return quinticOut(position, start, end);
                case Type::QuinticInOut:
                    return quinticInOut(position, start, end);
                case Type::SinusoidalIn:
                    return sinusoidalIn(position, start, end);
                case Type::SinusoidalOut:
                    return sinusoidalOut(position, start, end);
                case Type::SinusoidalInOut:
                    return sinusoidalInOut(position, start, end);
                case Type::ExponentialIn:
                    return exponentialIn(position, start, end);
                case Type::ExponentialOut:
                    return exponentialOut(position, start, end);
                case Type::ExponentialInOut:
                    return exponentialInOut(position, start, end);
                case Type::CircularIn:
                    return circularIn(position, start, end);
                case Type::CircularOut:
                    return circularOut(position, start, end);
                case Type::CircularInOut:
                    return circularInOut(position, start, end);
                case Type::BounceIn:
                    return bounceIn(position, start, end);
                case Type::BounceOut:
                    return bounceOut(position, start, end);
                case Type::BounceInOut:
                    return bounceInOut(position, start, end);
                case Type::ElasticIn:
                    return elasticIn(position, start, end);
                case Type::ElasticOut:
                    return elasticOut(position, start, end);
                case Type::ElasticInOut:
                    return elasticInOut(position, start, end);
                case Type::BackIn:
                    return backIn(position, start, end);
                case Type::BackOut:
                    return backOut(position, start, end);
                case Type::BackInOut:
                    return backInOut(position, start, end);
                default:
                    return linear(position, start, end);
            }
        }
    };
}