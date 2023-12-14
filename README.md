darmok
====

C++ engine using libraries that I like

* [CMake](https://cmake.org/) as the build system 
* [bgfx](https://github.com/bkaradzic/bgfx) as the renderer
* [glm](https://github.com/g-truc/glm) for 3d math
* [EnTT](https://github.com/skypjack/entt) as the entity component system
* [assimp](https://github.com/assimp/assimp) generic asset loading
* [ozz](https://github.com/guillaumeblanc/ozz-animation/) for 3d skeletal animations
* [wren](https://github.com/wren-lang/wren) as the scripting layer
* [spine](https://github.com/EsotericSoftware/spine-runtimes) for 2d skeletal animations
* [ldtk](https://ldtk.io/) for 2D level editor

Trying to use modern C++ patterns where possible.

Trying to target the following platforms

* desktop (windows, macos, linux)
* mobile (iOS, Android)

Pending tasks
====
**WARNING**: currently in early stages of development

* assimp importing
* find animations library
* spritesheet loading
* spine loading
* wren scripting

Phylosofical decisions
====
* use as much stl as possible
* no naked pointers (almost)
* throw exceptions (I know it's controversial)

Frequently Asked Questions
====

Will it have feature X?

Depends on what I need. Anyone is welcome to fork and submit PRs.

Interesting Related Projects
====

* [dome engine](https://github.com/domeengine/dome) minimalist engine with wren as the scripting language