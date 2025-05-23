#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <filesystem>
#include <memory>
#include <string>

namespace darmok
{
    class DARMOK_EXPORT BX_NO_VTABLE ISceneDefinitionLoader : public ILoader<protobuf::Scene>
    {
    };

	using DataSceneDefinitionLoader = DataProtobufLoader<ISceneDefinitionLoader>;

    class Scene;
    class SceneImporterImpl;

    class SceneImporter final
    {
    public:
        using Error = std::string;
		using Definition = protobuf::Scene;
        using Result = expected<void, Error>;
        SceneImporter();
		~SceneImporter();
        Result operator()(Scene& scene, const Definition& def);
    private:
		std::unique_ptr<SceneImporterImpl> _impl;
    };

    template<typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicSceneLoader
    {
    public:
        using Error = std::string;
        using Result = expected<void, Error>;
        using Argument = Arg;
        virtual ~IBasicSceneLoader() = default;
        virtual Result operator()(Scene& scene, Argument arg) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISceneLoader : public IBasicSceneLoader<std::filesystem::path>
    {
    };

    class DARMOK_EXPORT SceneLoader final : public ISceneLoader
    {
    public:
        SceneLoader(ISceneDefinitionLoader& defLoader);
        Result operator()(Scene& scene, std::filesystem::path path);

    private:
		ISceneDefinitionLoader& _defLoader;
    };

}