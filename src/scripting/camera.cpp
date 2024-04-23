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

	LuaCamera& LuaCamera::setWindowProjection1(float fovy, const VarVec2& range) noexcept
	{
		_camera->setWindowProjection(fovy, LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setWindowProjection2(float fovy, float near) noexcept
	{
		_camera->setWindowProjection(fovy, near);
		return *this;
	}

	LuaCamera& LuaCamera::setWindowProjection3(float fovy) noexcept
	{
		_camera->setWindowProjection(fovy);
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

	LuaCamera& LuaCamera::setWindowOrtho1(const VarVec2& range, float offset) noexcept
	{
		_camera->setWindowOrtho(LuaMath::tableToGlm(range), offset);
		return *this;
	}

	LuaCamera& LuaCamera::setWindowOrtho2(const VarVec2& range) noexcept
	{
		_camera->setWindowOrtho(LuaMath::tableToGlm(range));
		return *this;
	}

	LuaCamera& LuaCamera::setWindowOrtho3() noexcept
	{
		_camera->setWindowOrtho();
		return *this;
	}

	LuaCamera& LuaCamera::setTargetTexture(const LuaRenderTexture& texture) noexcept
	{
		_camera->setTargetTexture(texture.getReal());
		return *this;
	}

	LuaCamera& LuaCamera::setForwardPhongRenderer(const LuaProgram& program) noexcept
	{
		_camera->setRenderer<ForwardRenderer>(program.getReal(), _camera->addComponent<PhongLightingComponent>());
		return *this;
	}

	std::optional<LuaRenderTexture> LuaCamera::getTargetTexture() noexcept
	{
		auto tex = _camera->getTargetTexture();
		if (tex == nullptr)
		{
			return std::nullopt;
		}
		return LuaRenderTexture(tex);
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
		lua.new_usertype<LuaCamera>("Camera", sol::constructors<>(),
			"set_projection", sol::overload(
				&LuaCamera::setProjection1,
				&LuaCamera::setProjection2,
				&LuaCamera::setWindowProjection1,
				&LuaCamera::setWindowProjection2,
				&LuaCamera::setWindowProjection3
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2,
				&LuaCamera::setOrtho3,
				&LuaCamera::setWindowOrtho1,
				&LuaCamera::setWindowOrtho2,
				&LuaCamera::setWindowOrtho3
			),
			"set_forward_phong_renderer", &LuaCamera::setForwardPhongRenderer,
			"matrix", sol::property(&LuaCamera::getMatrix, &LuaCamera::setMatrix),
			"target_texture", sol::property(&LuaCamera::getTargetTexture, &LuaCamera::setTargetTexture),
			"screen_point_to_ray", &LuaCamera::screenPointToRay
		);
	}
}