
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