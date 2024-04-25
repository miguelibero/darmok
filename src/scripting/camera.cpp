#include "camera.hpp"
#include "program.hpp"
#include "texture.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/light.hpp>

namespace darmok
{
    LuaCamera::LuaCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	const Camera& LuaCamera::getReal() const
	{
		return _camera.value();
	}

	Camera& LuaCamera::getReal()
	{
		return _camera.value();
	}

	LuaCamera& LuaCamera::setProjection1(float fovy, float aspect, const VarVec2& range) noexcept
	{
		_camera->setProjection(fovy, aspect, LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setProjection2(float fovy, float aspect, float near) noexcept
	{
		_camera->setProjection(fovy, aspect, near);
		return *this;
	}

	LuaCamera& LuaCamera::setProjection3(float fovy, const VarUvec2& size, const VarVec2& range) noexcept
	{
		_camera->setProjection(fovy, LuaMath::tableToGlm(size), LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setProjection4(float fovy, const VarUvec2& size, float near) noexcept
	{
		_camera->setProjection(fovy, LuaMath::tableToGlm(size), near);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho1(const VarVec4& edges, const VarVec2& range, float offset) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges), LuaMath::tableToGlm(range), offset);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho2(const VarVec4& edges, const VarVec2& range) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges), LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho3(const VarVec4& edges) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho4(const VarUvec2& size, const VarVec2& range, float offset) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(size), LuaMath::tableToGlm(range), offset);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho5(const VarUvec2& size, const VarVec2& range) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(size), LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho6(const VarUvec2& size) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(size));
		return *this;
	}

	LuaCamera& LuaCamera::setTargetTextures(const sol::table& textures) noexcept
	{
		std::vector<std::shared_ptr<Texture>> realTextures;
		realTextures.reserve(textures.size());
		for (auto& elm : textures)
		{
			if (elm.second.is<LuaTexture>())
			{
				auto luaTexture = elm.second.as<LuaTexture>();
				realTextures.push_back(luaTexture.getReal());
			}
		}
		_camera->setTargetTextures(realTextures);
		return *this;
	}

	LuaCamera& LuaCamera::addNativeComponent(LuaNativeCameraComponentType type) noexcept
	{
		switch (type)
		{
		case LuaNativeCameraComponentType::PhongLighting:
			_camera->addComponent<PhongLightingComponent>();
			break;
		}
		return *this;
	}

	LuaCamera& LuaCamera::setForwardRenderer(const LuaProgram& program) noexcept
	{
		_camera->setRenderer<ForwardRenderer>(program.getReal());
		return *this;
	}

	std::vector<LuaTexture> LuaCamera::getTargetTextures() noexcept
	{
		auto& textures = _camera->getTargetTextures();
		std::vector<LuaTexture> luaTextures;
		luaTextures.reserve(textures.size());
		for (auto& tex : textures)
		{
			luaTextures.push_back(LuaTexture(tex));
		}
		return luaTextures;
	}

	const glm::mat4& LuaCamera::getMatrix() const noexcept
	{
		return _camera->getMatrix();
	}

	void LuaCamera::setMatrix(const glm::mat4& matrix) noexcept
	{
		_camera->setMatrix(matrix);
	}

	std::optional<Ray> LuaCamera::screenPointToRay(const VarVec2& point) const noexcept
	{
		return _camera->screenPointToRay(LuaMath::tableToGlm(point));
	}

	void LuaCamera::configure(sol::state_view& lua) noexcept
	{
		lua.new_enum<LuaNativeCameraComponentType>("CameraComponentType", {
			{ "PhongLighting", LuaNativeCameraComponentType::PhongLighting },
		});

		lua.new_usertype<LuaCamera>("Camera", sol::constructors<>(),
			"set_projection", sol::overload(
				&LuaCamera::setProjection1,
				&LuaCamera::setProjection2,
				&LuaCamera::setProjection3,
				&LuaCamera::setProjection4
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2,
				&LuaCamera::setOrtho3,
				&LuaCamera::setOrtho4,
				&LuaCamera::setOrtho5,
				&LuaCamera::setOrtho6
			),
			"set_forward_renderer", &LuaCamera::setForwardRenderer,
			"add_component", sol::overload(&LuaCamera::addNativeComponent),
			"matrix", sol::property(&LuaCamera::getMatrix, &LuaCamera::setMatrix),
			"target_textures", sol::property(&LuaCamera::getTargetTextures, &LuaCamera::setTargetTextures),
			"screen_point_to_ray", &LuaCamera::screenPointToRay
		);
	}
}