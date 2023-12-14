darmok
====


![Dathon trying to explain the importance of Darmok](icon.jpg)

C++ game engine combining libraries & tools that I like

* [CMake](https://cmake.org/) as the build system 
* [bgfx](https://github.com/bkaradzic/bgfx) as the renderer
* [glm](https://github.com/g-truc/glm) for 3d math
* [EnTT](https://github.com/skypjack/entt) as the entity component system
* [assimp](https://github.com/assimp/assimp) for generic asset loading
* [ozz](https://github.com/guillaumeblanc/ozz-animation/) for 3d skeletal animations
* [wren](https://github.com/wren-lang/wren) as the scripting layer
* [spine](https://github.com/EsotericSoftware/spine-runtimes) for 2d skeletal animations
* [ldtk](https://ldtk.io/) for 2D level editor
* [Box2D](https://box2d.org/) for 2D physics
* [Bullet](https://github.com/bulletphysics/bullet3) for 3D physics

Trying to use modern C++ patterns where possible.

Trying to target the following platforms

* desktop (windows, macos, linux)
* mobile (iOS, Android)

Some philosofical decisions (could be controversial)
* use as much stl as possible
* no naked pointers (almost)
* throw exceptions

**WARNING**: currently in early stages of development

## Frequently Asked Questions

* Will it have feature X?
> Depends on what I need. Anyone is welcome to fork and submit PRs.

## Current State

### Working features
* bgfx window setup
* scene using entt
* update logic methods with delta time
* sprites and spritesheets

### Pending tasks
* assimp importing
* find animations library
* spine loading
* wren scripting

## Interesting Related Projects
* [dome engine](https://github.com/domeengine/dome) - minimalist engine with wren as the scripting language