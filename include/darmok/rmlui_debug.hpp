#pragma once

#ifdef _DEBUG

#include <darmok/input.hpp>
#include <darmok/app.hpp>
#include <darmok/protobuf/rmlui.pb.h>
#include <memory>

namespace darmok
{
    class RmluiDebuggerComponentImpl;

	class DARMOK_EXPORT RmluiDebuggerComponent final : public ITypeAppComponent<RmluiDebuggerComponent>
    {
    public:
		using Definition = protobuf::RmluiDebuggerComponent;
		RmluiDebuggerComponent(const Definition& def = {}) noexcept;
		~RmluiDebuggerComponent() noexcept;

		void toggle() noexcept;

		bool isEnabled() const noexcept;

		expected<void, std::string> init(App& app) noexcept override;
		expected<void, std::string> shutdown() noexcept override;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def) noexcept;
	private:
		std::unique_ptr<RmluiDebuggerComponentImpl> _impl;
    };
}

#endif

