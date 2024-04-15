#include "scripting.hpp"
#include <darmok/asset.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/scene.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/window.hpp>
#include <darmok/render_forward.hpp>

#include <filesystem>
#include <memory>

namespace darmok
{
	ScriptingApp::ScriptingApp()
		: _impl(std::make_unique<ScriptingAppImpl>())
	{
	}

	ScriptingApp::~ScriptingApp()
	{
		// intentionally left blank for the unique_ptr<AppImpl> forward declaration
	}

	void ScriptingApp::init(const std::vector<std::string>& args)
	{
		App::init(args);
		_impl->init(*this, args);
	}

	OptionalRef<App> ScriptingAppImpl::_app = nullptr;

	ScriptingAppImpl::ScriptingAppImpl()
	{
	}

	/*
	template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
	struct glmVecOperations
	{
		using vec = glm::vec<L, T, Q>;
		static vec add(const vec& self, const vec& value)
		{
			return self + value;
		}

		static vec sub(vec& self, const vec& value)
		{
			return self - value;
		}

		static vec mul(vec& self, const vec& value)
		{
			return self * value;
		}

		static vec div(vec& self, const vec& value)
		{
			return self / value;
		}

		static bool eq(vec& self, const vec& value)
		{
			return self == value;
		}

		static bool neq(vec& self, const vec& value)
		{
			return self != value;
		}

		static bool neg(vec& self, const vec& value)
		{
			return self != value;
		}

		static const T& get(vec& self, glm::length_t idx)
		{
			return self[idx];
		}

		static T& set(vec& self, glm::length_t idx)
		{
			return self[idx];
		}

		static const vec& zero()
		{
			static vec v(0);
			return v;
		}
	};

	template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
	static void configureScriptingGlmVector(wren::ForeignKlassImpl<glm::vec<L, T, Q>>& cls) noexcept
	{
		cls.funcExt<&glmVecOperations<L, T, Q>::add>(wren::OPERATOR_ADD);
		cls.funcExt<&glmVecOperations<L, T, Q>::sub>(wren::OPERATOR_SUB);
		cls.funcExt<&glmVecOperations<L, T, Q>::mul>(wren::OPERATOR_MUL);
		cls.funcExt<&glmVecOperations<L, T, Q>::div>(wren::OPERATOR_DIV);
		cls.funcExt<&glmVecOperations<L, T, Q>::eq>(wren::OPERATOR_EQUAL);
		cls.funcExt<&glmVecOperations<L, T, Q>::neq>(wren::OPERATOR_NOT_EQUAL);
		cls.funcExt<&glmVecOperations<L, T, Q>::neg>(wren::OPERATOR_NEG);
		cls.funcExt<&glmVecOperations<L, T, Q>::get>(wren::OPERATOR_GET_INDEX);
		cls.funcExt<&glmVecOperations<L, T, Q>::set>(wren::OPERATOR_SET_INDEX);
		cls.funcStaticExt<&glmVecOperations<L, T, Q>::zero>("zero");
	}

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	struct glmMatOperations
	{
		using mat = glm::mat<L1, L2, T, Q>;
		using vec = glm::vec<L2, T, Q>;
		static mat add(mat& self, const mat& value)
		{
			return self + value;
		}

		static mat sub(mat& self, const mat& value)
		{
			return self - value;
		}

		static mat mul(mat& self, const mat& value)
		{
			return self * value;
		}

		static mat div(mat& self, const mat& value)
		{
			return self / value;
		}

		static bool eq(mat& self, const mat& value)
		{
			return self == value;
		}

		static bool neq(mat& self, const mat& value)
		{
			return self != value;
		}

		static bool neg(mat& self, const mat& value)
		{
			return self != value;
		}

		static const vec& get(mat& self, glm::length_t idx)
		{
			return self[idx];
		}

		static vec& set(mat& self, glm::length_t idx)
		{
			return self[idx];
		}
	};

	class ScriptingTransform final
	{
	public:
		WrenTransform(Transform& transform) noexcept;
		std::optional<WrenTransform> getParent() const noexcept;
		void setParent(std::optional<WrenTransform>& parent) noexcept;

		const glm::vec3& getPosition() const noexcept;
		const glm::vec3& getRotation() const noexcept;
		const glm::vec3& getScale() const noexcept;
		const glm::vec3& getPivot() const noexcept;

		void setPosition(const glm::vec3& v) noexcept;
		void setRotation(const glm::vec3& v) noexcept;
		void setScale(const glm::vec3& v) noexcept;
		void setPivot(const glm::vec3& v) noexcept;

	private:
		Transform& _transform;
	};

	class WrenCamera final
	{
	public:
		WrenCamera(Camera& camera) noexcept;
		WrenCamera& setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept;
		void setForwardPhongRenderer(const std::shared_ptr<Program>& program) noexcept;

	private:
		Camera& _camera;
	};

	class WrenPointLight final
	{
	public:
		WrenPointLight(PointLight& light) noexcept;
	private:
		PointLight _light;
	};

	class WrenMeshComponent final
	{
	public:
		WrenMeshComponent(MeshComponent& comp) noexcept;
	private:
		MeshComponent& _comp;
	};

	class WrenEntity;

	class WrenScene final
	{
	public:
		WrenScene(Scene& scene) noexcept;
		EntityRegistry& getRegistry() noexcept;
		WrenEntity newEntity() noexcept;
	private:
		Scene& _scene;
	};

	class WrenComponent final
	{
	public:
		WrenComponent(wren::Variable derived) noexcept
			: _derived(derived)
		{
		}
	private:
		wren::Variable _derived;
	};

	class WrenEntity final
	{
	public:
		WrenEntity(Entity entity, WrenScene scene) noexcept;
		WrenComponent addComponent(wren::Variable derived) noexcept;
		WrenTransform addTransformComponent() noexcept;
		WrenCamera addCameraComponent() noexcept;
		WrenPointLight addPointLightComponent() noexcept;
		WrenMeshComponent addMeshComponent(const std::shared_ptr<Mesh>& mesh) noexcept;
	private:
		Entity _entity;
		WrenScene _scene;
	};

	class WrenAssets final
	{
	public:
		WrenAssets(AssetContext& assets) noexcept;
		std::shared_ptr<Program> loadProgram(const std::string& name);
		std::shared_ptr<Program> loadStandardProgram(const std::string& name);
		std::shared_ptr<Texture> loadColorTexture(const Color& color);
	private:
		AssetContext& _assets;
	};

	class WrenWindow
	{
	public:
		WrenWindow(Window& win) noexcept;
		const glm::uvec2& getSize() const noexcept;
	private:
		Window& _win;
	};

	class WrenApp final
	{
	public:
		WrenApp(App& app) noexcept;
		WrenScene getScene();
		WrenAssets getAssets();
		WrenWindow getWindow();
	private:
		App& _app;
		OptionalRef<Scene> _scene;
	};

	WrenTransform::WrenTransform(Transform& transform) noexcept
		: _transform(transform)
	{
	}

	std::optional<WrenTransform> WrenTransform::getParent() const noexcept
	{
		auto parent = _transform.getParent();
		if (parent)
		{
			return parent.value();
		}
		return std::nullopt;
	}

	void WrenTransform::setParent(std::optional<WrenTransform>& parent) noexcept
	{
		OptionalRef<Transform> p = nullptr;
		if (parent.has_value())
		{
			p = parent.value()._transform;
		}
		_transform.setParent(p);
	}

	const glm::vec3& WrenTransform::getPosition() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& WrenTransform::getRotation() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& WrenTransform::getScale() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& WrenTransform::getPivot() const noexcept
	{
		return _transform.getPosition();
	}

	void WrenTransform::setPosition(const glm::vec3& v) noexcept
	{
		_transform.setPosition(v);
	}

	void WrenTransform::setRotation(const glm::vec3& v) noexcept
	{
		_transform.setRotation(v);
	}

	void WrenTransform::setScale(const glm::vec3& v) noexcept
	{
		_transform.setScale(v);
	}

	void WrenTransform::setPivot(const glm::vec3& v) noexcept
	{
		_transform.setPivot(v);
	}

	WrenCamera::WrenCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	WrenCamera& WrenCamera::setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept
	{
		_camera.setProjection(fovy, size, near, far);
		return *this;
	}

	void WrenCamera::setForwardPhongRenderer(const std::shared_ptr<Program>& program) noexcept
	{
		_camera.setRenderer<ForwardRenderer>(program, _camera.addComponent<PhongLightingComponent>());
	}

	WrenPointLight::WrenPointLight(PointLight& light) noexcept
		: _light(light)
	{
	}

	WrenMeshComponent::WrenMeshComponent(MeshComponent& comp) noexcept
		: _comp(comp)
	{
	}

	WrenEntity::WrenEntity(Entity entity, WrenScene scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	WrenComponent WrenEntity::addComponent(wren::Variable derived) noexcept
	{
		return WrenComponent(derived);
	}

	WrenTransform WrenEntity::addTransformComponent() noexcept
	{
		return WrenTransform(_scene.getRegistry().emplace<Transform>(_entity));
	}

	WrenCamera WrenEntity::addCameraComponent() noexcept
	{
		return WrenCamera(_scene.getRegistry().emplace<Camera>(_entity));
	}

	WrenPointLight WrenEntity::addPointLightComponent() noexcept
	{
		return WrenPointLight(_scene.getRegistry().emplace<PointLight>(_entity));
	}

	WrenMeshComponent WrenEntity::addMeshComponent(const std::shared_ptr<Mesh>& mesh) noexcept
	{
		return WrenMeshComponent(_scene.getRegistry().emplace<MeshComponent>(_entity, mesh));
	}

	WrenScene::WrenScene(Scene& scene) noexcept
		: _scene(scene)
	{
	}

	EntityRegistry& WrenScene::getRegistry() noexcept
	{
		return _scene.getRegistry();
	}

	WrenEntity WrenScene::newEntity() noexcept
	{
		return WrenEntity(_scene.getRegistry().create(), *this);
	}

	WrenAssets::WrenAssets(AssetContext& assets) noexcept
		: _assets(assets)
	{
	}

	std::shared_ptr<Program> WrenAssets::loadProgram(const std::string& name)
	{
		return _assets.getProgramLoader()(name);
	}

	std::shared_ptr<Program> WrenAssets::loadStandardProgram(const std::string& name)
	{
		auto type = StandardProgramType::Unlit;
		if (name == "ForwardPhong")
		{
			type = StandardProgramType::ForwardPhong;
		}
		else if (name == "Unlit")
		{
			type = StandardProgramType::ForwardPhong;
		}
		else
		{
			throw std::exception("unknown standard program");
		}
		return _assets.getStandardProgramLoader()(type);
	}

	std::shared_ptr<Texture> WrenAssets::loadColorTexture(const Color& color)
	{
		return _assets.getColorTextureLoader()(color);
	}

	WrenApp::WrenApp(App& app) noexcept
		: _app(app)
	{
	}

	WrenScene WrenApp::getScene()
	{
		if (!_scene)
		{
			_scene = _app.addComponent<SceneAppComponent>().getScene();
		}
		return WrenScene(_scene.value());
	}

	WrenAssets WrenApp::getAssets()
	{
		return WrenAssets(_app.getAssets());
	}

	WrenWindow WrenApp::getWindow()
	{
		return WrenWindow(_app.getWindow());
	}


	WrenWindow::WrenWindow(Window& win) noexcept
		: _win(win)
	{
	}

	const glm::uvec2& WrenWindow::getSize() const noexcept
	{
		return _win.getSize();
	}

	struct WrenColors
	{
		Color::value_type maxValue = Colors::maxValue;
		Color black = Colors::black;
		Color white = Colors::white;
		Color red = Colors::red;
		Color green = Colors::green;
		Color blue = Colors::blue;
		Color yellow = Colors::yellow;
		Color cyan = Colors::cyan;
		Color magenta = Colors::magenta;
	};

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	static void configureScriptingGlmMatrix(wren::ForeignKlassImpl<glm::mat<L1, L2, T, Q>>& cls) noexcept
	{
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::add>(wren::OPERATOR_ADD);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::sub>(wren::OPERATOR_SUB);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::mul>(wren::OPERATOR_MUL);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::div>(wren::OPERATOR_DIV);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::eq>(wren::OPERATOR_EQUAL);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::neq>(wren::OPERATOR_NOT_EQUAL);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::neg>(wren::OPERATOR_NEG);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::get>(wren::OPERATOR_GET_INDEX);
		cls.funcExt<&glmMatOperations<L1, L2, T, Q>::set>(wren::OPERATOR_SET_INDEX);
	}

	static void configureScriptingMath(wren::ForeignModule& mod) noexcept
	{
		{
			auto& cls = mod.klass<glm::vec4>("Vec4");
			cls.ctor<float, float, float, float>();
			cls.var<&glm::vec4::x>("x");
			cls.var<&glm::vec4::y>("y");
			cls.var<&glm::vec4::z>("z");
			cls.var<&glm::vec4::z>("w");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::vec3>("Vec3");
			cls.ctor<float, float, float>();
			cls.var<&glm::vec3::x>("x");
			cls.var<&glm::vec3::y>("y");
			cls.var<&glm::vec3::z>("z");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::vec2>("Vec2");
			cls.ctor<float, float>();
			cls.var<&glm::vec2::x>("x");
			cls.var<&glm::vec2::y>("y");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::uvec2>("Uvec2");
			cls.ctor<unsigned int, unsigned int>();
			cls.var<&glm::uvec2::x>("x");
			cls.var<&glm::uvec2::y>("y");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::mat4>("Mat4");
			cls.ctor<glm::vec4, glm::vec4, glm::vec4, glm::vec4>();
			configureScriptingGlmMatrix(cls);
		}
		{
			auto& cls = mod.klass<glm::mat3>("Mat3");
			cls.ctor<glm::vec3, glm::vec3, glm::vec3>();
			configureScriptingGlmMatrix(cls);
		}
		{
			auto& cls = mod.klass<Color3>("Color3");
			cls.ctor<float, float, float>();
			cls.var<&Color3::r>("r");
			cls.var<&Color3::g>("g");
			cls.var<&Color3::b>("b");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<Color>("Color");
			cls.ctor<float, float, float, float>();
			cls.var<&Color::r>("r");
			cls.var<&Color::g>("g");
			cls.var<&Color::b>("b");
			cls.var<&Color::a>("a");
			configureScriptingGlmVector(cls);
		}
	}

	static void configureScriptingBase(wren::ForeignModule& mod) noexcept
	{
		mod.append("import \"scene\"\n");

		{
			auto& cls = mod.klass<WrenApp>("App");
			cls.propReadonly<&WrenApp::getScene>("scene");
			cls.propReadonly<&WrenApp::getAssets>("assets");
			cls.propReadonly<&WrenApp::getWindow>("window");
		}
		{
			auto& cls = mod.klass<bgfx::VertexLayout>("VertexLayout");
		}
		{
			auto& cls = mod.klass<Program>("Program");
			cls.propReadonly<&Program::getVertexLayout>("vertexLayout");
		}
		{
			auto& cls = mod.klass<WrenAssets>("Assets");
			cls.func<&WrenAssets::loadProgram>("loadProgram");
			cls.func<&WrenAssets::loadStandardProgram>("loadStandardProgram");
			cls.func<&WrenAssets::loadColorTexture>("loadColorTexture");
		}
		{
			auto& cls = mod.klass<WrenWindow>("Window");
			cls.propReadonly<&WrenWindow::getSize>("size");
		}
		{
			auto& cls = mod.klass<Mesh>("Mesh");
			cls.prop<&Mesh::getMaterial, &Mesh::setMaterial>("material");
			cls.funcStatic<&Mesh::createCube>("newCube");
			cls.funcStatic<&Mesh::createSphere>("newSphere");
			cls.funcStatic<&Mesh::createQuad>("newQuad");
			cls.funcStatic<&Mesh::createLineQuad>("newLineQuad");
			cls.funcStatic<&Mesh::createSprite>("newSprite");
		}
		{
			auto& cls = mod.klass<Texture>("Texture");
		}
		{
			auto& cls = mod.klass<Material>("Material");
			cls.ctor<std::shared_ptr<Texture>>();
		}
		{
			auto& cls = mod.klass<WrenColors>("WrenColors");
			cls.ctor<>();
			cls.varReadonly<&WrenColors::black>("black");
			cls.varReadonly<&WrenColors::white>("white");
			cls.varReadonly<&WrenColors::red>("red");
			cls.varReadonly<&WrenColors::green>("green");
			cls.varReadonly<&WrenColors::blue>("blue");
			cls.varReadonly<&WrenColors::yellow>("yellow");
			cls.varReadonly<&WrenColors::cyan>("cyan");
			cls.varReadonly<&WrenColors::magenta>("magenta");
		}

		mod.append("var Colors = WrenColors.new()\n");
	}

	static void configureScriptingScene(wren::ForeignModule& mod) noexcept
	{
		{
			auto& cls = mod.klass<WrenTransform>("Transform");
			cls.prop<&WrenTransform::getParent, &WrenTransform::setParent>("parent");
			cls.prop<&WrenTransform::getPosition, &WrenTransform::setPosition>("position");
			cls.prop<&WrenTransform::getRotation, &WrenTransform::setRotation>("rotation");
			cls.prop<&WrenTransform::getScale, &WrenTransform::setScale>("scale");
			cls.prop<&WrenTransform::getPivot, &WrenTransform::setPivot>("pivot");
		}
		{
			auto& cls = mod.klass<WrenCamera>("Camera");
			cls.func<&WrenCamera::setProjection>("setProjection");
			cls.func<&WrenCamera::setForwardPhongRenderer>("setForwardPhongRenderer");
		}
		{
			auto& cls = mod.klass<WrenMeshComponent>("MeshComponent");
		}
		{
			auto& cls = mod.klass<WrenPointLight>("PointLight");
		}
		{
			auto& cls = mod.klass<WrenComponent>("Component");
		}
		{
			auto& cls = mod.klass<WrenEntity>("Entity");
			cls.func<&WrenEntity::addTransformComponent>("addTransformComponent");
			cls.func<&WrenEntity::addCameraComponent>("addCameraComponent");
			cls.func<&WrenEntity::addMeshComponent>("addMeshComponent");
			cls.func<&WrenEntity::addPointLightComponent>("addPointLightComponent");
			cls.func<&WrenEntity::addComponent>("addComponent");
		}
		{
			auto& cls = mod.klass<WrenScene>("Scene");
			cls.func<&WrenScene::newEntity>("newEntity");
		}
	}

	static void configureScriptingMath(wren::ForeignModule& mod) noexcept
	{
		{
			auto& cls = mod.klass<glm::vec4>("Vec4");
			cls.ctor<float, float, float, float>();
			cls.var<&glm::vec4::x>("x");
			cls.var<&glm::vec4::y>("y");
			cls.var<&glm::vec4::z>("z");
			cls.var<&glm::vec4::z>("w");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::vec3>("Vec3");
			cls.ctor<float, float, float>();
			cls.var<&glm::vec3::x>("x");
			cls.var<&glm::vec3::y>("y");
			cls.var<&glm::vec3::z>("z");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::vec2>("Vec2");
			cls.ctor<float, float>();
			cls.var<&glm::vec2::x>("x");
			cls.var<&glm::vec2::y>("y");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::uvec2>("Uvec2");
			cls.ctor<unsigned int, unsigned int>();
			cls.var<&glm::uvec2::x>("x");
			cls.var<&glm::uvec2::y>("y");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<glm::mat4>("Mat4");
			cls.ctor<glm::vec4, glm::vec4, glm::vec4, glm::vec4>();
			configureScriptingGlmMatrix(cls);
		}
		{
			auto& cls = mod.klass<glm::mat3>("Mat3");
			cls.ctor<glm::vec3, glm::vec3, glm::vec3>();
			configureScriptingGlmMatrix(cls);
		}
		{
			auto& cls = mod.klass<Color3>("Color3");
			cls.ctor<float, float, float>();
			cls.var<&Color3::r>("r");
			cls.var<&Color3::g>("g");
			cls.var<&Color3::b>("b");
			configureScriptingGlmVector(cls);
		}
		{
			auto& cls = mod.klass<Color>("Color");
			cls.ctor<float, float, float, float>();
			cls.var<&Color::r>("r");
			cls.var<&Color::g>("g");
			cls.var<&Color::b>("b");
			cls.var<&Color::a>("a");
			configureScriptingGlmVector(cls);
		}
	}
	*/


	void ScriptingAppImpl::init(App& app, const std::vector<std::string>& args)
	{
		_app = app;
		_lua = std::make_unique<sol::state>();
	}

	OptionalRef<App>& ScriptingAppImpl::getApp() noexcept
	{
		return _app;
	}

	void ScriptingAppImpl::shutdown() noexcept
	{
		_app = nullptr;
		_lua.reset();
	}
}