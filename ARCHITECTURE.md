
# App

The main class of a darmok app is called `darmok::App`.
To create a new app, implement the `darmok::IAppDelegate` interface
and pass it to the `DARMOK_RUN_APP` macro. See `darmok/app.hpp`
for all the available app delegate methods.

```c++
#include <darmok/app.hpp>

class MyAppDelegate final : public darmok::IAppDelegate
{
public:
    expected<void, std::string> init() noexcept override
    {
        // add stuff
        return {};
    }

    expected<void, std::string> update(float deltaTime) noexcept override
    {
        // update stuff
        return {};
    }
};

DARMOK_RUN_APP(MyAppDelegate);
```

## App Components

The app contains a list of components that can be added or removed. Right now
darmok supports the following

* imgui: add imgui support
* scene: adds scene rendering

# Scene

The scene can be accessed by adding the scene app component.

```cpp
    expected<void, std::string> init() noexcept override
    {
        OptionalRef<SceneAppComponent> sceneComp;
        DARMOK_TRY_VALUE_PREFIX(sceneComp, _app.addComponent<SceneAppComponent>(), "adding scene component");
        auto scene = sceneComp->getScene();
    }
```

The scene behaves similarly to a scene in Unity3d but using [entt](https://github.com/skypjack/entt) as de backend.
In the scene contains a list of entities, each entity can have multiple components of different types.
The following components are implemented:

* transform
* camera
* renderable
* lights
    * AmbientLight
    * DirectionalLight
    * PointLight
    * SpotLight
* text
* skeletal animation
    * armature
    * animator
* rmlui canvas

## Scene Components

The same way as the app has components, the scene has components too, inheriting from `ISceneComponent`.

Provided scene components are:

* frame animator
* freelook controller
* lua
    * script runner
    * component
* physics system (jolt)
* rmlui system
* skeletal animation system (ozz)

## Camera

The camera entity component tries to have a similar api to the Unity3d camera. 

The following events are passed from the renderers to the camera components:

* `shouldEntityBeCulled` called to check if an entity should be culled
* `beforeRenderEntity` called before an entity is rendered
* `beforeRenderLight` called before the light is rendered
* `beforeRenderSubmit` called before any render submit (draw call)
* `afterRender` called on the final forward render pass

### Camera Components

* renderers
    * forward renderer
    * deferred renderer
* components
    * light renderer
    * shadow renderer
    * skybox renderer
    * physics debug renderer
    * rmlui renderer
    * text renderer
    * skeletal animation

### Render Chain

The camera has a render chain that allows the engine to add postprocessing passes.

# Assets

Each asset type can be loaded using a loader. The loader accepts a path parameter
and returns a shared pointer to the asset or an error string.

```c++
 template<typename Type>
class ILoader
{
public:
    using Resource = Type;
    using Error = std::string;
    using Result = expected<std::shared_ptr<Resource>, Error>;
    using Argument = Arg;
    virtual ~ILoader() = default;
    [[nodiscard]] virtual Result operator()(std::filesystem::path arg) noexcept = 0;
};
```

See the `IAssetContext` interface in `darmok/asset.hpp` for a full list of asset loaders.

## Types of assets

* `[Something]Source`: structure that holds the source data for a resource, before being compiled or processed
* `[Something]Definition`: structure that holds the runtime data to load a resource
* `[Something]`: the runtime representation of a resource (usually loaded in memory and ready to use)

## Full asset list

* program
    * `ProgramSource`: text source for the shaders + varying definition
    * `ProgramDefinition`: compiled variants for the shaders for the shaders + varying definition
    * `Program`: bgfx program handler + varying definition
* texture
    * `TextureSource`: encoded image data + additional settings for the texture
    * `TextureDefinition`: texture data ready to upload to the GPU + additional settings for the texture
    * `Texture`: bgfx texture handler + methods to manipulate a texture
* mesh
* material
* armature
* texture atlas
* font
* scene
* skeleton
* skeletal animator
* sound
* music

 

# Entities & Components