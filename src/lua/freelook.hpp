#pragma once

#include "lua/lua.hpp"

namespace darmok
{
    class FreelookController;
    class Camera;
    class Scene;

    namespace protobuf
    {
        class FreelookController;
	}

    class LuaFreelookController final
    {
    public:
    	static void bind(sol::state_view& lua) noexcept;
    private:
        using Definition = protobuf::FreelookController;
        static std::reference_wrapper<FreelookController> addSceneComponent1(Scene& scene, Camera& cam);
		static std::reference_wrapper<FreelookController> addSceneComponent2(Scene& scene, Camera& cam, const Definition& def);
    };
}