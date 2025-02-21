

## Resources

* `[Something]Source`: structure that holds the source data for a resource, before being compiled or processed
* `[Something]Definition`: structure that holds the runtime data to load a resource
* `[Something]`: the runtime representation of a resource (usually loaded in memory and ready to use)

### For Example

* `TextureSource`: encoded image data + additional settings for the texture
* `TextureDefinition`: texture data ready to upload to the GPU + additional settings for the texture
* `Texture`: bgfx texture handler + methods to manipulate a texture