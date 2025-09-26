darmok
====

![Dathon trying to explain the importance of Darmok](logo.png)

C++ game engine combining opensource libraries & tools that I like

currently using:

* [bgfx](https://github.com/bkaradzic/bgfx) as the renderer
* [CMake](https://cmake.org/) as the build system 
* [vcpkg](https://vcpkg.io) as the package manager
* [glm](https://github.com/g-truc/glm) for 3d math
* [EnTT](https://github.com/skypjack/entt) as the entity component system
* [assimp](https://github.com/assimp/assimp) for generic asset loading (not in runtime)
* [sol2](https://github.com/ThePhD/sol2) modern C++ bindings for lua
* [nlohmann-json](https://github.com/nlohmann/json) for parsing json
* [pugixml](https://pugixml.org/) for parsing xml
* [imgui](https://github.com/ocornut/imgui) for editor UI
    * [imGuizmo](https://github.com/CedricGuillemet/ImGuizmo) for editor controls
* [RmlUI](https://github.com/mikke89/RmlUi) for ingame UI
* [ozz](https://github.com/guillaumeblanc/ozz-animation/) for 3d skeletal animations
* [protocol buffers](https://protobuf.dev/getting-started/cpptutorial/) for serialization
* [Jolt](https://github.com/jrouwe/JoltPhysics) for 3D physics
* [miniaudio](https://miniaud.io/) for audio
* [taskflow](https://github.com/taskflow/taskflow) for multithreading
* [tiny-process-lib](https://gitlab.com/eidheim/tiny-process-library) to run os processes
* [mikktspace](http://www.mikktspace.com/) - to calculate tangents
* [middleclass](https://github.com/kikito/middleclass) - small object orientation lua lib
* [cli11](https://github.com/CLIUtils/CLI11) - command line parser
* [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - native file dialogs
* [tl-expected](https://github.com/TartanLlama/expected) - expected for C++20
* [magic_enum](https://github.com/Neargye/magic_enum) automatically convert enum types
* [fmt](https://github.com/fmtlib/fmt) for string formatting
* [utfcpp](https://github.com/nemtrif/utfcpp) for unicode string encoding

planned to use:
* [tracy](https://github.com/wolfpld/tracy) for frame profiling
    (blocked because of [bgfx#3308](https://github.com/bkaradzic/bgfx/pull/3308)
    and the vcpkg bgfx build not having a profiler feature)
* [recast navigation](http://recastnav.com/) for pathfinding
* [BehaviorTree.CPP](https://www.behaviortree.dev/) for behavior trees
* [Box2D](https://box2d.org/) 2D physics
* [ldtk](https://ldtk.io/) for 2D level editor
* [spine](https://github.com/EsotericSoftware/spine-runtimes) for 2d skeletal animations
* [slang](https://github.com/shader-slang/slang) unified shader language that compiles to  the different render backends

Trying to target the following platforms:
* desktop (windows, macos, linux)
* mobile (iOS, Android) (pending)

Some philosofical decisions (could be controversial)
* use modern C++ (20) patterns where possible
* use as much stl as possible (need to look into memory management at some point)
* no naked pointers
* return `expected` for error handling
* try to keep the API similar to Unity3D (where it makes sense) so that it's easy to port game logic

**WARNING**: currently in early stages of development

![editor screenshot](editor_screenshot.png)

## Frequently Asked Questions

* Will it have feature X?
> Depends on what I need. Anyone is welcome to fork and submit PRs.

## Current State

I'm still learning CMake, so if you see something that should be fixed please let me know.

### Working features

* bgfx window setup (GLFW on windows & linux)
* update logic methods with delta time
* renderer
    * point, spot, directional & ambient lights
    * forward render
        * unlit, gui
        * basic phong
        * PBR metallic-roughness
    * cascaded shadow mapping
    * ambient, directional, spot & point light shadows
    * camera culling (frustum, occlusion seems to be broken)
* asset system
    * sprites and spritesheets
    * loading models using assimp (FBX, gltf, etc...)
    * serializing assets into protocol buffers
* entity component scene using entt
    * transform, camera
    * type filtering
    * serialization using protocol buffers
    * lights
    * free look
* lua scripting
    * coroutines with API similar to unity
* multiple UI options
    * imgui for tooling
    * RmlUI for ingame (support for multiple canvases)
* skeletal animations using ozz (reading from binary)
* 3d physics using jolt
    * rigidbodies
    * character controller
* tool to export asset folders 
    * shaders & vertex layouts
    * copy files
    * assimp to custom protoc model format
    * ozz skeleton & animations
* dynamic font texture generation
* play sounds and music (wav & mp3)
* editor using imgui

#### Upcoming
* finish editor
    * skeletal animator support
    * physics support
    * camera components support
    * create lua script component
    * fix scene view gizmos
* slang support
* move from exceptions to expected when possible
* move main thread to taskflow
* frame limiting
* more renderer features
    * bloom
    * deferred, clustered
    * SSAO
* luajit support (maybe compile using importer)
* text improvements
    * finish all the TextRenderConfig options
    * dynamic distance field rendering with border support
* clipboard text support (UTF8)
* asset loading progress (maybe implement progressive loading)
* possible refactors
    * maybe replace Data for std::vector<uint8_t> and DataView for std::span<uint8_t>
    * move lua bindings to separate library?

#### In the future
* run clang-tidy
* fix occlusion culling
* switch from exceptions to std::expected
* more unit tests
* performance profiling
* 2d physics
* instancing
* 3d physics materials
* baked shadowmaps
* multithreaded updates ([Ubisoft](https://www.youtube.com/watch?v=X1T3IQ4N-3g))
* custom UI module (rmlui is nice but slow)
* spine animations
* unify use of allocators everywhere
* progressive asset loaders
* lua debugging would be nice
* particle systems (maybe effekseer)
* more sound options (spatialization, effects)
* animation root motion 
* [openusd](https://github.com/PixarAnimationStudios/OpenUSD) scene format support
* [slang](https://github.com/shader-slang/slang) shading language support
* [microsoft cpp-async](https://github.com/microsoft/cpp-async) coroutines
    * async asset loading
    * editor actions

## Interesting Related Projects
* [SuperNovaEngine](https://github.com/skaarj1989/SupernovaEngine) a very similar engine but much more advanced
* [Cluster](https://github.com/pezcode/Cluster) - PBR shaders for bgfx
* [dome engine](https://github.com/domeengine/dome) - minimalist engine with wren as the scripting language
* [RaZ engine](https://github.com/Razakhel/RaZ) - C++17 game engine
* [meshoptimizer](https://github.com/zeux/meshoptimizer)
* [forward+](https://www.3dgep.com/forward-plus/) - description of tiled forward renderer in DirectX 11

## Example code

```lua
function init()
    local program = StandardProgramLoader.load(StandardProgramType.Forward)

    local scenes = app:add_component(SceneAppComponent)
    local scene = scenes.scene

    local camEntity = scene:create_entity()
    local camTrans = camEntity:add_component(Transform, { 0, 1, -1 })
    camTrans:look_at({ 0, 0, 0 })
    local cam = camEntity:add_component(Camera)
    cam:set_perspective(60, 0.3, 1000)
    cam:add_component(ForwardRenderer)
    cam:add_component(LightingRenderComponent)

    local lightEntity = scene:create_entity()
    lightEntity:add_component(AmbientLight, 0.2)
    lightEntity:add_component(Transform, { 1, 1, -1 })
    local light = lightEntity:add_component(PointLight, 2)
    light.range = 5

    local mesh = MeshData.new_sphere():create_mesh(program.vertex_layout)
    scene:create_entity():add_component(Renderable, mesh, program, Color.green)
end
```