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
        expected<int32_t, std::string> setup(const CmdArgs& args) noexcept override;
        expected<void, std::string> init() noexcept override;
        expected<void, std::string> earlyShutdown() noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> render() const noexcept override;
    protected:
    private:
        std::unique_ptr<LuaAppDelegateImpl> _impl;
    };
}