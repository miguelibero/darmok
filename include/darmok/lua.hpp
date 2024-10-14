#pragma once

#include <darmok/export.h>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
    class LuaAppDelegateImpl;

    class DARMOK_EXPORT LuaAppDelegate final : public IAppDelegate
    {
    public:
        LuaAppDelegate(App& app) noexcept;
        ~LuaAppDelegate() noexcept;
        std::optional<int32_t> setup(const std::vector<std::string>& args);
        void init() override;
        void earlyShutdown() override;
        void shutdown() override;
        void render() const override;
    protected:
    private:
        std::unique_ptr<LuaAppDelegateImpl> _impl;
    };
}