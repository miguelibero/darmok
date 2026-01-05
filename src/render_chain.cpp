#include <darmok/render_chain.hpp>
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/string.hpp>
#include <darmok/glm_serialize.hpp>


namespace darmok
{
	Texture::Config FrameBuffer::createColorConfig(const glm::uvec2& size) noexcept
	{
		Texture::Config config;
		*config.mutable_size() = convert<protobuf::Uvec2>(size);
		config.set_format(Texture::Definition::RGBA16F);
		config.set_type(Texture::Definition::Texture2D);
		return config;
	}

	Texture::Config FrameBuffer::createDepthConfig(const glm::uvec2& size) noexcept
	{
		Texture::Config config;
		*config.mutable_size() = convert<protobuf::Uvec2>(size);
		config.set_format(Texture::Definition::D16F);
		config.set_type(Texture::Definition::Texture2D);
		return config;
	}

	FrameBuffer::FrameBuffer(FrameBufferOwnedHandle handle, std::shared_ptr<Texture> colorTex, std::shared_ptr<Texture> depthTex) noexcept
		: _handle{ std::move(handle) }
		, _colorTex{ colorTex }
		, _depthTex{ depthTex }
	{
	}

	expected<FrameBuffer, std::string> FrameBuffer::load(const glm::uvec2& size, bool depth) noexcept
	{
		if (size.x == 0 || size.y == 0)
		{
			return FrameBuffer{};
		}
		auto texResult = Texture::load(createColorConfig(size), BGFX_TEXTURE_RT);
		if(!texResult)
		{
			return unexpected{ std::move(texResult).error() };
		}
		auto colorTex = std::make_shared<Texture>(std::move(texResult).value());
		std::vector<bgfx::TextureHandle> handles{ colorTex->getHandle() };
		std::shared_ptr<Texture> depthTex = nullptr;
		if(depth)
		{
			auto depthTexResult = Texture::load(createDepthConfig(size), BGFX_TEXTURE_RT);
			if (!depthTexResult)
			{
				return unexpected{ std::move(depthTexResult).error() };
			}
			depthTex =  std::make_shared<Texture>(std::move(depthTexResult).value());
			handles.push_back(depthTex->getHandle());
		}
		auto handle = bgfx::createFrameBuffer(handles.size(), &handles.front());
		return FrameBuffer{ handle, colorTex, depthTex };
	}

	void FrameBuffer::reset() noexcept
	{
		_handle.reset();
	}

	FrameBuffer::operator bool() const noexcept
	{
		return valid();
	}

	bool FrameBuffer::valid() const noexcept
	{
		return _handle.valid();
	}

	uint16_t FrameBuffer::idx() const noexcept
	{
		return _handle.idx();
	}

	const std::shared_ptr<Texture>& FrameBuffer::getTexture() const noexcept
	{
		return _colorTex;
	}

	const std::shared_ptr<Texture>& FrameBuffer::getDepthTexture() const noexcept
	{
		return _depthTex;
	}

	FrameBufferHandle FrameBuffer::getHandle() const noexcept
	{
		return _handle;
	}

	glm::uvec2 FrameBuffer::getSize() const noexcept
	{
		if (!_colorTex)
		{
			return glm::uvec2{ 0 };
		}
		return _colorTex->getSize();
	}

	void FrameBuffer::configureView(bgfx::ViewId viewId) const noexcept
	{
		bgfx::setViewFrameBuffer(viewId, _handle);
	}

	RenderChain::RenderChain(IRenderChainDelegate& dlg) noexcept
		: _delegate{ dlg }
		, _running{ false }
	{
	}

	RenderChain::RenderChain(std::unique_ptr<IRenderChainDelegate>&& dlg) noexcept
		: _delegate{ *dlg }
		, _delegatePointer{ std::move(dlg) }
		, _running{ false }
	{
	}

	expected<void, std::string> RenderChain::init() noexcept
	{
		_running = true;
		std::vector<std::string> errors;
		for (size_t i = 0; i < _steps.size(); ++i)
		{
			auto& step = _steps[i];
			auto result = step->init(*this);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
			else
			{
				result = step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
				if (!result)
				{
					errors.push_back(std::move(result).error());
				}
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> RenderChain::shutdown() noexcept
	{
		std::vector<std::string> errors;
		for (auto itr = _steps.rbegin(); itr != _steps.rend(); ++itr)
		{
			auto result = (*itr)->shutdown();
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		_running = false;
		_viewId.reset();
		_steps.clear();
		_buffers.clear();
		_output.reset();
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> RenderChain::update(float deltaTime) noexcept
	{
		std::vector<std::string> errors;
		for (auto& step : _steps)
		{
			auto result = step->update(deltaTime);
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> RenderChain::setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept
	{
		if (_output == fb)
		{
			return {};
		}
		_output = fb;
		if (!_running)
		{
			return {};
		}
		auto size = _steps.size();
		if (size > 0)
		{
			auto result = updateStep(size - 1);
			if (!result)
			{
				return result;
			}
		}
		else if (_output && _viewId)
		{
			_output->configureView(*_viewId);
		}
		return {};
	}

	expected<bool, std::string> RenderChain::setOutputSize(const glm::uvec2& size) noexcept
	{
		if (_output && size == _output->getSize())
		{
			return false;
		}
		auto fbResult = FrameBuffer::load(size);
		if (!fbResult)
		{
			return unexpected{ std::move(fbResult).error() };
		}
		auto setResult = setOutput(std::make_shared<FrameBuffer>(std::move(fbResult).value()));
		if (!setResult)
		{
			return unexpected{ std::move(setResult).error() };
		}
		return true;
	}

	std::shared_ptr<FrameBuffer> RenderChain::getOutput() const noexcept
	{
		return _output;
	}

	expected<bgfx::ViewId, std::string> RenderChain::beforeRenderReset(bgfx::ViewId viewId) noexcept
	{
		if (_output)
		{
			bgfx::setViewName(viewId, getViewName("clear").c_str());

			static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL | BGFX_CLEAR_COLOR;
			static const uint8_t clearColor = 1;
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U,
				clearColor, clearColor, clearColor, clearColor,
				clearColor, clearColor, clearColor, clearColor);

			if (auto input = getInput())
			{
				input->configureView(viewId);
			}

			_viewId = viewId;
			updateViewport();

			++viewId;
		}
		else
		{
			_viewId.reset();
		}

		auto amount = _buffers.size();
		_buffers.clear();
		for (size_t i = 0; i < amount; ++i)
		{
			auto result = addBuffer();
			if(!result)
			{
				return unexpected{ std::move(result).error() };
			}
		}

		return viewId;
	}

	std::string RenderChain::getViewName(const std::string& baseName) const noexcept
	{
		return _delegate.getRenderChainViewName("Render Chain: " + baseName);
	}

	expected<bgfx::ViewId, std::string> RenderChain::renderReset(bgfx::ViewId viewId) noexcept
	{
		if (_running)
		{
			for (size_t i = 0; i < _steps.size(); ++i)
			{
				auto& step = _steps[i];
				auto updateResult = step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
				if(!updateResult)
				{
					return unexpected{ std::move(updateResult).error() };
				}
				auto result = step->renderReset(viewId);
				if (!result)
				{
					return result;
				}
				viewId = result.value();
			}
		}

		return viewId;
	}

	bool RenderChain::updateViewport() noexcept
	{
		if (!_viewId)
		{
			return false;
		}
		auto vp = _delegate.getRenderChainViewport();
		vp.configureView(*_viewId);
		return true;
	}

	OptionalRef<FrameBuffer> RenderChain::getReadBuffer(size_t i) const noexcept
	{
		if (i < 0 || i >= _buffers.size())
		{
			return std::nullopt;
		}
		return *_buffers[i];
	}

	OptionalRef<FrameBuffer> RenderChain::getWriteBuffer(size_t i) const noexcept
	{
		if (i < 0 || i >= _buffers.size())
		{
			return std::nullopt;
		}
		if (i == _buffers.size() - 1)
		{
			return getOutput().get();
		}
		return *_buffers[i + 1];
	}

	OptionalRef<FrameBuffer> RenderChain::getInput() noexcept
	{
		if (!_buffers.empty())
		{
			return *_buffers.front();
		}
		if (_output)
		{
			return *_output;
		}
		if (auto parent = _delegate.getRenderChainParent())
		{
			return parent->getInput();
		}
		return nullptr;
	}

	OptionalRef<const FrameBuffer> RenderChain::getInput() const noexcept
	{
		if (!_buffers.empty())
		{
			return *_buffers.front();
		}
		if (_output)
		{
			return *_output;
		}
		if (auto parent = _delegate.getRenderChainParent())
		{
			return parent->getInput();
		}
		return nullptr;
	}

	void RenderChain::configureView(bgfx::ViewId viewId, const std::string& name, OptionalRef<const FrameBuffer> writeBuffer) const noexcept
	{
		bgfx::setViewName(viewId, getViewName(name).c_str());
		uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
		if (writeBuffer)
		{
			writeBuffer->configureView(viewId);
		}
		auto vp = _delegate.getRenderChainViewport();
		vp.configureView(viewId);
	}

	expected<std::reference_wrapper<FrameBuffer>, std::string> RenderChain::addBuffer() noexcept
	{
		auto vp = _delegate.getRenderChainViewport();
		auto size = vp.origin + vp.size;
		auto fbResult = FrameBuffer::load(size, true);
		if(!fbResult)
		{
			return unexpected{ std::move(fbResult).error() };
		}
		return *_buffers.emplace_back(std::make_unique<FrameBuffer>(std::move(fbResult).value()));
	}

	expected<void, std::string> RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		auto readBufferResult = addBuffer();
		if(!readBufferResult)
		{
			return unexpected{ std::move(readBufferResult).error() };
		}
		auto& readBuffer = readBufferResult.value().get();
		auto& ref = *step;
		auto i = _steps.size();
		_steps.push_back(std::move(step));
		if (!_running)
		{
			return {};
		}

		auto result = ref.init(*this);
		if(!result)
		{
			return result;
		}
		result = ref.updateRenderChain(readBuffer, getWriteBuffer(i));
		if(!result)
		{
			return result;
		}

		if (i > 0)
		{
			result = updateStep(i - 1);
			if (!result)
			{
				return result;
			}
		}
		_delegate.onRenderChainChanged();
		return {};
	}

	expected<void, std::string> RenderChain::updateStep(size_t i) noexcept
	{
		if (i < 0 || i >= _steps.size())
		{
			return {};
		}
		auto& step = _steps[i];
		return step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
	}

	expected<bool, std::string> RenderChain::removeStep(const IRenderChainStep& step) noexcept
	{
		auto ptr = &step;
		auto itr = std::find_if(_steps.begin(), _steps.end(),
			[ptr](auto& elm) { return elm.get() == ptr; });
		if (itr == _steps.end())
		{
			return false;
		}

		if(_running)
		{
			auto result = (*itr)->shutdown();
			if(!result)
			{
				return unexpected{ std::move(result).error() };
			}
		}

		auto i = std::distance(_steps.begin(), itr);
		_steps.erase(itr);
		_buffers.erase(_buffers.begin() + i);

		if (!_running)
		{
			return true;
		}

		for (auto j = i; j < _steps.size(); ++j)
		{
			auto result = updateStep(j);
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
		}
		if (i > 0)
		{
			auto result = updateStep(i - 1);
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
		}
		_delegate.onRenderChainChanged();
		return true;
	}

	bool RenderChain::empty() const noexcept
	{
		return _buffers.empty() && !_output;
	}

	bool RenderChain::valid() const noexcept
	{
		return !_output || _output->valid();
	}

	expected<void, std::string> RenderChain::render() noexcept
	{
		auto encoder = bgfx::begin();

		if (_viewId)
		{
			encoder->touch(_viewId.value());
		}

		std::vector<std::string> errors;
		for (auto& step : _steps)
		{
			auto result = step->render(*encoder);
			if(!result)
			{
				errors.push_back(std::move(result).error());
			}
		}

		bgfx::end(encoder);
		return StringUtils::joinExpectedErrors(errors);
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name, int priority) noexcept
		: _program{ prog }
		, _name{ name }
		, _priority{ priority }
	{
		if (_name.empty())
		{
			_name = "ScreenSpaceRenderPass";
		}
	}

	ScreenSpaceRenderPass::~ScreenSpaceRenderPass() noexcept = default;

	expected<void, std::string> ScreenSpaceRenderPass::init(RenderChain& chain) noexcept
	{
		_chain = chain;
		_texUniform = { "s_texColor", bgfx::UniformType::Sampler };

		static const Rectangle screen{ glm::uvec2{2} };
		auto meshResult = MeshData{ screen }.createMesh(_program->getVertexLayout());
		if (!meshResult)
		{
			return unexpected{ std::move(meshResult).error() };
		}
		_mesh = std::make_unique<Mesh>(std::move(meshResult).value());
		return {};
	}

	expected<void, std::string> ScreenSpaceRenderPass::shutdown() noexcept
	{
		_viewId.reset();
		_mesh.reset();
		_chain.reset();
		return {};
	}

	expected<void, std::string> ScreenSpaceRenderPass::updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept
	{
		_readTex = read;
		_writeTex = write;
		if (write && _viewId)
		{
			write->configureView(_viewId.value());
		}
		return {};
	}

	expected<bgfx::ViewId, std::string> ScreenSpaceRenderPass::renderReset(bgfx::ViewId viewId) noexcept
	{
		if (_chain)
		{
			_chain->configureView(viewId, _name, _writeTex);
		}
		_viewId = viewId;
		return ++viewId;
	}

	expected<void, std::string> ScreenSpaceRenderPass::update(float deltaTime) noexcept
	{
		_basicUniforms.update(deltaTime);
		return {};
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept
	{
		_uniformTextures.emplace(Texture::createUniformKey(name, stage), texture);
		return *this;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setUniform(const std::string& name, std::optional<UniformValue> value) noexcept
	{
		if (value.has_value())
		{
			_uniformValues.emplace(name, *value);
		}
		else
		{
			_uniformValues.erase(name);
		}
		return *this;
	}

	expected<void, std::string> ScreenSpaceRenderPass::render(bgfx::Encoder& encoder) noexcept
	{
		if (!_viewId)
		{
			return unexpected<std::string>{"empty viewId"};
		}
		auto viewId = _viewId.value();
		if (!_mesh || !_program || !_readTex)
		{
			encoder.touch(viewId);
			return {};
		}

		auto result = _mesh->render(encoder);
		if (!result)
		{
			return result;
		}

		encoder.setTexture(0, _texUniform, _readTex->getTexture()->getHandle());
		_basicUniforms.configure(encoder);
		_uniformHandles.configure(encoder, _uniformValues);
		_uniformHandles.configure(encoder, _uniformTextures);

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_BLEND_ALPHA
			;
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());

		return {};
	}
}