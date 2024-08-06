#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class LuaRunnerAppImpl;

    class DARMOK_EXPORT LuaRunnerApp final : public App
    {
    public:
        LuaRunnerApp() noexcept;
        ~LuaRunnerApp() noexcept;
        std::optional<int32_t> setup(const std::vector<std::string>& args);
        void init() override;
        void shutdown() override;
        void render() const override;
    protected:
        void update(float deltaTime) override;
    private:
        std::unique_ptr<LuaRunnerAppImpl> _impl;
    };
}