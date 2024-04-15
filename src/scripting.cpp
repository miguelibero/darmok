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

	class LuaProgram final
	{
	public:
		LuaProgram(const std::shared_ptr<Program>& program) noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;
		const std::shared_ptr<Program>& getReal() const noexcept;
	private:
		std::shared_ptr<Program> _program;
	};

	class LuaTexture final
	{
	public:
		LuaTexture(const std::shared_ptr<Texture>& texture) noexcept;
		const std::shared_ptr<Texture>& getReal() const noexcept;
	private:
		std::shared_ptr<Texture> _texture;
	};

	class LuaMaterial final
	{
	public:
		LuaMaterial() noexcept;
		LuaMaterial(const std::shared_ptr<Material>& material) noexcept;
		LuaMaterial(const LuaTexture& texture) noexcept;
		const std::shared_ptr<Material>& getReal() const noexcept;
	private:
		std::shared_ptr<Material> _material;
	};

	class LuaMesh final
	{
	public:
		LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
		static LuaMesh createCube(const bgfx::VertexLayout& layout) noexcept;
		const std::shared_ptr<Mesh>& getReal() const noexcept;
		LuaMaterial getMaterial() const noexcept;
		void setMaterial(const LuaMaterial& material) noexcept;
	private:
		std::shared_ptr<Mesh> _mesh;
	};

	class LuaTransform final
	{
	public:
		LuaTransform(Transform& transform) noexcept;
		sol::optional<LuaTransform> getParent() const noexcept;
		void setParent(sol::optional<LuaTransform> parent) noexcept;

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

	class LuaCamera final
	{
	public:
		LuaCamera(Camera& camera) noexcept;
		void setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept;
		void setForwardPhongRenderer(const LuaProgram& program) noexcept;

	private:
		Camera& _camera;
	};

	class LuaPointLight final
	{
	public:
		LuaPointLight(PointLight& light) noexcept;
	private:
		PointLight& _light;
	};

	class LuaMeshComponent final
	{
	public:
		LuaMeshComponent(MeshComponent& comp) noexcept;
	private:
		MeshComponent& _comp;
	};

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, Scene& scene) noexcept;
		LuaTransform addTransformComponent() noexcept;
		LuaCamera addCameraComponent() noexcept;
		LuaPointLight addPointLightComponent() noexcept;
		LuaMeshComponent addMeshComponent(const LuaMesh& mesh) noexcept;
	private:
		Entity _entity;
		Scene& _scene;
	};

	class LuaScene final
	{
	public:
		LuaScene(Scene& scene) noexcept;
		EntityRegistry& getRegistry() noexcept;
		LuaEntity createEntity() noexcept;
	private:
		Scene& _scene;
	};

	class LuaAssets final
	{
	public:
		LuaAssets(AssetContext& assets) noexcept;
		LuaProgram loadProgram(const std::string& name);
		LuaProgram loadStandardProgram(const std::string& name);
		LuaTexture loadColorTexture(const Color& color);
	private:
		AssetContext& _assets;
	};

	class LuaWindow
	{
	public:
		LuaWindow(Window& win) noexcept;
		const glm::uvec2& getSize() const noexcept;
	private:
		Window& _win;
	};

	class LuaApp final
	{
	public:
		LuaApp(App& app) noexcept;
		LuaScene getScene() noexcept;
		LuaAssets getAssets() noexcept;
		LuaWindow getWindow() noexcept;
	private:
		App& _app;
		OptionalRef<Scene> _scene;
	};

	LuaProgram::LuaProgram(const std::shared_ptr<Program>& program) noexcept
		: _program(program)
	{
	}

	const bgfx::VertexLayout& LuaProgram::getVertexLayout() const noexcept
	{
		return _program->getVertexLayout();
	}

	const std::shared_ptr<Program>& LuaProgram::getReal() const noexcept
	{
		return _program;
	}

	LuaTexture::LuaTexture(const std::shared_ptr<Texture>& texture) noexcept
		: _texture(texture)
	{
	}

	const std::shared_ptr<Texture>& LuaTexture::getReal() const noexcept
	{
		return _texture;
	}

	LuaMaterial::LuaMaterial() noexcept
		: _material(std::make_shared<Material>())
	{
	}

	LuaMaterial::LuaMaterial(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
	{
	}

	LuaMaterial::LuaMaterial(const LuaTexture& texture) noexcept
		: _material(std::make_shared<Material>(texture.getReal()))
	{
	}

	const std::shared_ptr<Material>& LuaMaterial::getReal() const noexcept
	{
		return _material;
	}

	LuaMesh::LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept
		: _mesh(mesh)
	{
	}

	const std::shared_ptr<Mesh>& LuaMesh::getReal() const noexcept
	{
		return _mesh;
	}

	LuaMaterial LuaMesh::getMaterial() const noexcept
	{
		return LuaMaterial(_mesh->getMaterial());
	}

	void LuaMesh::setMaterial(const LuaMaterial& material) noexcept
	{
		_mesh->setMaterial(material.getReal());
	}

	LuaMesh LuaMesh::createCube(const bgfx::VertexLayout& layout) noexcept
	{
		return LuaMesh(Mesh::createCube(layout));
	}

	LuaTransform::LuaTransform(Transform& transform) noexcept
		: _transform(transform)
	{
	}

	sol::optional<LuaTransform> LuaTransform::getParent() const noexcept
	{
		auto parent = _transform.getParent();
		if (parent)
		{
			return sol::optional<LuaTransform>(parent.value());
		}
		return std::nullopt;
	}

	void LuaTransform::setParent(sol::optional<LuaTransform> parent) noexcept
	{
		OptionalRef<Transform> p = nullptr;
		if (parent.has_value())
		{
			p = parent.value()._transform;
		}
		_transform.setParent(p);
	}

	const glm::vec3& LuaTransform::getPosition() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& LuaTransform::getRotation() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& LuaTransform::getScale() const noexcept
	{
		return _transform.getPosition();
	}

	const glm::vec3& LuaTransform::getPivot() const noexcept
	{
		return _transform.getPosition();
	}

	void LuaTransform::setPosition(const glm::vec3& v) noexcept
	{
		_transform.setPosition(v);
	}

	void LuaTransform::setRotation(const glm::vec3& v) noexcept
	{
		_transform.setRotation(v);
	}

	void LuaTransform::setScale(const glm::vec3& v) noexcept
	{
		_transform.setScale(v);
	}

	void LuaTransform::setPivot(const glm::vec3& v) noexcept
	{
		_transform.setPivot(v);
	}

	LuaCamera::LuaCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	void LuaCamera::setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept
	{
		_camera.setProjection(fovy, size, near, far);
	}

	void LuaCamera::setForwardPhongRenderer(const LuaProgram& program) noexcept
	{
		_camera.setRenderer<ForwardRenderer>(program.getReal(), _camera.addComponent<PhongLightingComponent>());
	}

	LuaPointLight::LuaPointLight(PointLight& light) noexcept
		: _light(light)
	{
	}

	LuaMeshComponent::LuaMeshComponent(MeshComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaEntity::LuaEntity(Entity entity, Scene& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	LuaTransform LuaEntity::addTransformComponent() noexcept
	{
		return LuaTransform(_scene.getRegistry().emplace<Transform>(_entity));
	}

	LuaCamera LuaEntity::addCameraComponent() noexcept
	{
		return LuaCamera(_scene.getRegistry().emplace<Camera>(_entity));
	}

	LuaPointLight LuaEntity::addPointLightComponent() noexcept
	{
		return LuaPointLight(_scene.getRegistry().emplace<PointLight>(_entity));
	}

	LuaMeshComponent LuaEntity::addMeshComponent(const LuaMesh& mesh) noexcept
	{
		return LuaMeshComponent(_scene.getRegistry().emplace<MeshComponent>(_entity, mesh.getReal()));
	}

	LuaScene::LuaScene(Scene& scene) noexcept
		: _scene(scene)
	{
	}

	EntityRegistry& LuaScene::getRegistry() noexcept
	{
		return _scene.getRegistry();
	}

	LuaEntity LuaScene::createEntity() noexcept
	{
		return LuaEntity(_scene.getRegistry().create(), _scene);
	}

	LuaAssets::LuaAssets(AssetContext& assets) noexcept
		: _assets(assets)
	{
	}

	LuaProgram LuaAssets::loadProgram(const std::string& name)
	{
		return LuaProgram(_assets.getProgramLoader()(name));
	}

	LuaProgram LuaAssets::loadStandardProgram(const std::string& name)
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
		return LuaProgram(_assets.getStandardProgramLoader()(type));
	}

	LuaTexture LuaAssets::loadColorTexture(const Color& color)
	{
		return LuaTexture(_assets.getColorTextureLoader()(color));
	}

	LuaApp::LuaApp(App& app) noexcept
		: _app(app)
	{
	}

	LuaScene LuaApp::getScene() noexcept
	{
		if (!_scene)
		{
			_scene = _app.addComponent<SceneAppComponent>().getScene();
		}
		return LuaScene(_scene.value());
	}

	LuaAssets LuaApp::getAssets() noexcept
	{
		return LuaAssets(_app.getAssets());
	}

	LuaWindow LuaApp::getWindow() noexcept
	{
		return LuaWindow(_app.getWindow());
	}


	LuaWindow::LuaWindow(Window& win) noexcept
		: _win(win)
	{
	}

	const glm::uvec2& LuaWindow::getSize() const noexcept
	{
		return _win.getSize();
	}

	template<typename T, typename C>
	static sol::usertype<T> configureScriptingGlmVector(sol::state_view& lua, std::string_view name, const C& constructors) noexcept
	{
		using value_type = T::value_type;
		return lua.new_usertype<T>(name, constructors,
			sol::meta_function::addition, sol::resolve<T(const T&, const T&)>(glm::operator+),
			sol::meta_function::subtraction, sol::resolve<T(const T&, const T&)>(glm::operator-),
			sol::meta_function::multiplication, sol::resolve<T(const T&, const T&)>(glm::operator*),
			sol::meta_function::division, sol::resolve<T(const T&, const T&)>(glm::operator/),
			"dot", sol::resolve<value_type(const T&, const T&)>(glm::dot)
		);
	}

	void ScriptingAppImpl::init(App& app, const std::vector<std::string>& args)
	{
		_lua = std::make_unique<sol::state>();
		auto& lua = *_lua;
		lua.open_libraries();
		{
			configureScriptingGlmVector<glm::vec3>(lua, "vec3", sol::constructors<glm::vec3(float), glm::vec3(float, float, float)>());
		}
		{
			auto usertype = lua.new_usertype<Color>("Color",
				sol::constructors<Color(uint8_t, uint8_t, uint8_t, uint8_t)>()
			);
			lua.create_named_table("colors",
				"green", &Colors::green
			);
		}
		{
			auto usertype = lua.new_usertype<LuaProgram>("Program");
			usertype["vertex_layout"] = sol::property(&LuaProgram::getVertexLayout);
		}
		{
			auto usertype = lua.new_usertype<LuaMaterial>("Material",
				sol::constructors<LuaMaterial(), LuaMaterial(LuaTexture)>());
		}
		{
			auto usertype = lua.new_usertype<LuaMesh>("Mesh");
			usertype["new_cube"] = &LuaMesh::createCube;
			usertype["material"] = sol::property(&LuaMesh::getMaterial, &LuaMesh::setMaterial);
		}
		{
			auto usertype = lua.new_usertype<LuaAssets>("Assets");
			usertype["load_program"] = &LuaAssets::loadProgram;
			usertype["load_standard_program"] = &LuaAssets::loadStandardProgram;
			usertype["load_color_texture"] = &LuaAssets::loadColorTexture;
		}
		{
			auto usertype = lua.new_usertype<LuaTransform>("Transform");
			usertype["position"] = sol::property(&LuaTransform::getPosition, &LuaTransform::setPosition);
			usertype["rotation"] = sol::property(&LuaTransform::getRotation, &LuaTransform::setRotation);
			usertype["scale"] = sol::property(&LuaTransform::getScale, &LuaTransform::setScale);
			usertype["pivot"] = sol::property(&LuaTransform::getPivot, &LuaTransform::setPivot);
			usertype["parent"] = sol::property(&LuaTransform::getParent, &LuaTransform::setParent);
		}
		{
			auto usertype = lua.new_usertype<LuaCamera>("Camera");
			usertype["set_projection"] = &LuaCamera::setProjection;
			usertype["set_forward_phong_renderer"] = &LuaCamera::setForwardPhongRenderer;
		}
		{
			auto usertype = lua.new_usertype<LuaEntity>("Entity");
			usertype["add_camera_component"] = &LuaEntity::addCameraComponent;
			usertype["add_transform_component"] = &LuaEntity::addTransformComponent;
			usertype["add_point_light_component"] = &LuaEntity::addPointLightComponent;
			usertype["add_mesh_component"] = &LuaEntity::addMeshComponent;
		}
		{
			auto usertype = lua.new_usertype<LuaScene>("Scene");
			usertype["create_entity"] = &LuaScene::createEntity;
		}
		{
			auto usertype = lua.new_usertype<LuaWindow>("Window");
			usertype["size"] = sol::property(&LuaWindow::getSize);
		}
		{
			auto usertype = lua.new_usertype<LuaApp>("App");
			usertype["scene"] = sol::property(&LuaApp::getScene);
			usertype["assets"] = sol::property(&LuaApp::getAssets);
			usertype["window"] = sol::property(&LuaApp::getWindow);
		}


		lua["app"] = LuaApp(app);

		lua.script_file("main.lua");
	}

	void ScriptingAppImpl::shutdown() noexcept
	{
		_lua.reset();
	}
}