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
* [luajit](https://luajit.org/) as the scripting layer
* [sol2](https://github.com/ThePhD/sol2) modern C++ bindings for lua
* [rapidjson](https://github.com/Tencent/rapidjson) for parsing json
* [pugixml](https://pugixml.org/) for parsing xml
* [nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for ingame UI

planned to use:

* [ozz](https://github.com/guillaumeblanc/ozz-animation/) for 3d skeletal animations
* [Bullet](https://github.com/bulletphysics/bullet3) for 3D physics
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
* use as much stl as possible
* no naked pointers
* throw exceptions

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
* loading shader vertex layout from json
* sprites and spritesheets
* loading meshes using assimp (FBX, etc...)
* renderer
    * unlit
    * forward with phong lighting (point, ambient)
* lua scripting
* nuklear UI binding

### Next tasks

* skeletal animations
* physics
* other types of lights
* pbr lighting
* deferred renderer
* spine animations loading
* distance field fonts
* mesh serialization
* unify use of std::allocator everywhere

## Interesting Related Projects

* [dome engine](https://github.com/domeengine/dome) - minimalist engine with wren as the scripting language
* [RaZ engine](https://github.com/Razakhel/RaZ) - C++17 game engine
* [cluster](https://github.com/pezcode/Cluster) - PBR implementation for bgfx
* scripting alternatives
    * [pocketpy](https://pocketpy.dev/) - python as a scripting language
    * [wren](https://github.com/wren-lang/wren) as the scripting layer
    * [wrenbind17](https://github.com/matusnovak/wrenbind17) for modern C++ wren binding
* no clear UI library winner,
    it would be interesting to write a skinnable UI library based on (vg-renderer)[https://github.com/jdryg/vg-renderer] and using html and css to skin 

## Example code

```lua
program = app.assets:load_standard_program(StandardProgramType.ForwardPhong)

camEntity = app.scene:create_entity()
camTrans = camEntity:get_transform()
camTrans.position = { 0, 2, -2 }
camTrans:look_at({ 0, 0, 0 })
local cam = camEntity:get_camera()
cam:set_projection(60, Vec2.new(0.3, 1000))
cam:set_forward_phong_renderer(program)

lightEntity = app.scene:create_entity()
lightTrans = lightEntity:get_transform()
lightTrans.position = { 1, 1, -2 }
lightEntity:get_point_light()

cubeMesh = MeshCreator.new(program.vertex_layout):create_cube()
greenTex = app.assets:load_color_texture(Colors.green)
cubeMesh.material = Material.new(greenTex)
app.scene:create_entity():add_mesh(cubeMesh)
```