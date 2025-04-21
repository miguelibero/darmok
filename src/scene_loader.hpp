#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>

#include <string>

namespace darmok
{
    class SceneImporterImpl final
    {
    public:
        using Error = std::string;
        using Definition = protobuf::Scene;
        using Result = expected<void, Error>;
        Result operator()(Scene& scene, const Definition& def);
    };
}