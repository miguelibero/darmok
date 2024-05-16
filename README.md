darmok
====

![Dathon trying to explain the importance of Darmok](logo.png)

C++ game engine combining libraries & tools that I like

currently using:

* [CMake](https://cmake.org/) as the build system 
* [bgfx](https://github.com/bkaradzic/bgfx) as the renderer
* [glm](https://github.com/g-truc/glm) for 3d math
* [EnTT](https://github.com/skypjack/entt) as the entity component system
* [assimp](https://github.com/assimp/assimp) for generic asset loading (not in runtime)
* [sol2](https://github.com/ThePhD/sol2) modern C++ bindings for lua
* [nlohmann-json](https://github.com/nlohmann/json) for parsing json
* [pugixml](https://pugixml.org/) for parsing xml
* [imgui](https://github.com/ocornut/imgui) for editor UI
* [RmlUI](https://github.com/mikke89/RmlUi) for ingame UI to replace CEGUI

planned to use:

* [ozz](https://github.com/guillaumeblanc/ozz-animation/) for 3d skeletal animations
* [Jolt](https://github.com/jrouwe/JoltPhysics) for 3D physics
* [Box2D](https://box2d.org/) for 2D physics
* [ldtk](https://ldtk.io/) for 2D level editor
* [spine](https://github.com/EsotericSoftware/spine-runtimes) for 2d skeletal animations
* [fastgltf](https://github.com/spnda/fastgltf) for gltf asset loading (runtime)
* [cereal](https://uscilab.github.io/cereal/) for serialization

Trying to use modern C++ patterns where possible.

Trying to target the following platforms:
* desktop (windows, macos, linux)
* mobile (iOS, Android) (pending)

Some philosofical decisions (could be controversial)
* no game editor, will use external tools (blender, ldtk, etc...)
* use as much stl as possible (need to look into memory management at some point)
* no naked pointers
* throw exceptions for error handling
* try to keep the lua API as similar as possible to Unity3D so that it's easy to port game logic

**WARNING**: currently in early stages of development

## Frequently Asked Questions

* Will it have feature X?
> Depends on what I need. Anyone is welcome to fork and submit PRs.

## Current State

I'm still learning CMake, so if you see something that should be fixed please let me know.

### Working features

* bgfx window setup (GLFW on windows & linux)
* scene using entt
* update logic methods with delta time
* sprites and spritesheets
* loading meshes using assimp (FBX, etc...)
* renderer
    * unlit
    * forward with phong lighting (point, ambient)
* lua scripting
* multiple UI options
    * imgui for tooling
    * nuklear game UI
    * Crazy Eddi's UI for more advanced stuff 

### Next tasks

* separate Mesh classes (check if memory needs to be copied)
* use entt::dispatcher to manage platform events and commands
* skeletal animations
* physics
* support multiple imgui & rmlui app components with different transforms
* other types of lights
* pbr lighting
* deferred renderer
* spine animations loading
* distance field fonts
* binary mesh serialization
* unify use of std::allocator everywhere
* sound
* progressive data reader
* async?

## Interesting Related Projects

* [SuperNovaEngine](https://github.com/skaarj1989/SupernovaEngine) a very similar engine but much more advanced
* [dome engine](https://github.com/domeengine/dome) - minimalist engine with wren as the scripting language
* [RaZ engine](https://github.com/Razakhel/RaZ) - C++17 game engine
* [cluster](https://github.com/pezcode/Cluster) - PBR implementation for bgfx

## Example code

```lua
program = app.assets:load_standard_program(StandardProgramType.ForwardPhong)

camEntity = app.scene:create_entity()
camTrans = camEntity:add_component(Transform, { 0, 2, -2 })
camTrans:look_at({ 0, 0, 0 })
local cam = camEntity:add_component(Camera)
cam:set_projection(60, { 0.3, 1000 })
cam:set_forward_renderer(program)
cam:add_component(CameraComponentType.PhongLighting)

lightEntity = app.scene:create_entity()
lightEntity:add_component(Transform, { 1, 1, -2 })
lightEntity:add_component(PointLight)

cubeMesh = MeshCreator.new(program.vertex_layout):create_cube()
greenTex = app.assets:load_color_texture(Color.green)
app.scene:create_entity():add_component(Renderable, cubeMesh, greenTex)
```