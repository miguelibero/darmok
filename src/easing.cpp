#include <darmok/easing.hpp>
#include <algorithm>

namespace darmok
{
    const std::unordered_map<Easing::Type, std::string> Easing::_typeNames =
    {
        { EasingType::Linear, "Linear" }, 
        { EasingType::Stepped, "Stepped" }, 
        { EasingType::QuadraticIn, "QuadraticIn" }, 
        { EasingType::QuadraticOut, "QuadraticOut" }, 
        { EasingType::QuadraticInOut, "QuadraticInOut" }, 
        { EasingType::CubicIn, "CubicIn" }, 
        { EasingType::CubicOut, "CubicOut" }, 
        { EasingType::CubicInOut, "CubicInOut" }, 
        { EasingType::QuarticIn, "QuarticIn" }, 
        { EasingType::QuarticOut, "QuarticOut" }, 
        { EasingType::QuarticInOut, "QuarticInOut" }, 
        { EasingType::QuinticIn, "QuinticIn" }, 
        { EasingType::QuinticOut, "QuinticOut" }, 
        { EasingType::QuinticInOut, "QuinticInOut" }, 
        { EasingType::SinusoidalIn, "SinusoidalIn" }, 
        { EasingType::SinusoidalOut, "SinusoidalOut" }, 
        { EasingType::SinusoidalInOut, "SinusoidalInOut" }, 
        { EasingType::ExponentialIn, "ExponentialIn" }, 
        { EasingType::ExponentialOut, "ExponentialOut" }, 
        { EasingType::ExponentialInOut, "ExponentialInOut" }, 
        { EasingType::CircularIn, "CircularIn" }, 
        { EasingType::CircularOut, "CircularOut" }, 
        { EasingType::CircularInOut, "CircularInOut" }, 
        { EasingType::BounceIn, "BounceIn" }, 
        { EasingType::BounceOut, "BounceOut" }, 
        { EasingType::BounceInOut, "BounceInOut" }, 
        { EasingType::ElasticIn, "ElasticIn" }, 
        { EasingType::ElasticOut, "ElasticOut" }, 
        { EasingType::ElasticInOut, "ElasticInOut" }, 
        { EasingType::BackIn, "BackIn" }, 
        { EasingType::BackOut, "BackOut" }, 
        { EasingType::BackInOut, "BackInOut" }
    };

    const std::string& Easing::getTypeName(Type type) noexcept
    {
        auto itr = _typeNames.find(type);
        if (itr == _typeNames.end())
        {
            static const std::string empty;
            return empty;
        }
        return itr->second;
    }

    std::optional<Easing::Type> Easing::readType(std::string_view name) noexcept
    {
        auto itr = std::find_if(_typeNames.begin(), _typeNames.end(), [&name](auto& elm) { return elm.second == name; });
        if (itr == _typeNames.end())
        {
            return std::nullopt;
        }
        return itr->first;
    }
}