#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	class ImguiAppComponentImpl;

    class ImguiAppComponent final : public AppComponent
    {
    public:
		ImguiAppComponent(float fontSize = 18.0f);

		void init(App& app, const std::vector<std::string>& args) override;
		void shutdown() override;
		void beforeUpdate(const WindowHandle& window, const InputState& input) override;
		void afterUpdate(const WindowHandle& window, const InputState& input) override;
	private:
		std::unique_ptr<ImguiAppComponentImpl> _impl;
    };

}