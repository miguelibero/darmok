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

	FrameBuffer::FrameBuffer(const glm::uvec2& size, bool depth) noexcept
		: _colorTex{ std::make_shared<Texture>(createColorConfig(size), BGFX_TEXTURE_RT) }
		, _depthTex{ depth ? std::make_shared<Texture>(createDepthConfig(size), BGFX_TEXTURE_RT) : nullptr }
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles{ _colorTex->getHandle() };
		if (_depthTex)
		{
			handles.push_back(_depthTex->getHandle());
		}
		_handle = bgfx::createFrameBuffer(handles.size(), &handles.front());
	}

	FrameBuffer::~FrameBuffer() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	FrameBuffer::FrameBuffer(FrameBuffer&& other)
		: _handle{ other._handle }
		, _colorTex{ std::move(other._colorTex) }
		, _depthTex{ std::move(other._depthTex) }
	{
		other._handle.idx = bgfx::kInvalidHandle;
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other)
	{
		_handle = other._handle;
		other._handle.idx = bgfx::kInvalidHandle;
		_colorTex = std::move(other._colorTex);
		_depthTex = std::move(other._depthTex);
		return *this;
	}

	const std::shared_ptr<Texture>& FrameBuffer::getTexture() const noexcept
	{
		return _colorTex;
	}

	const std::shared_ptr<Texture>& FrameBuffer::getDepthTexture() const noexcept
	{
		return _depthTex;
	}

	const bgfx::FrameBufferHandle& FrameBuffer::getHandle() const noexcept
	{
		return _handle;
	}

	glm::uvec2 FrameBuffer::getSize() const noexcept
	{
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

	expected<void, std::string> RenderChain::shutdown()
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

	expected<void, std::string> RenderChain::update(float deltaTime)
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
		if (_running)
		{
			auto size = _steps.size();
			if (size > 0)
			{
				auto result = updateStep(size - 1);
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	std::shared_ptr<FrameBuffer> RenderChain::getOutput() const noexcept
	{
		return _output;
	}

	void RenderChain::beforeRenderReset() noexcept
	{
		auto amount = _buffers.size();
		_buffers.clear();
		for (size_t i = 0; i < amount; ++i)
		{
			addBuffer();
		}		
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
		bgfx::setViewName(viewId, getViewName("clear").c_str());

		static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL | BGFX_CLEAR_COLOR;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 1);

		if (auto input = getInput())
		{
			input->configureView(viewId);
		}
		auto vp = _delegate.getRenderChainViewport();
		vp.configureView(viewId);
		_viewId = viewId;
		return ++viewId;
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

	void RenderChain::configureView(bgfx::ViewId viewId, const std::string& name, OptionalRef<const FrameBuffer> writeBuffer) const
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

	FrameBuffer& RenderChain::addBuffer() noexcept
	{
		auto vp = _delegate.getRenderChainViewport();
		auto size = vp.origin + vp.size;
		return *_buffers.emplace_back(std::make_unique<FrameBuffer>(size));
	}

	expected<void, std::string> RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		auto& readBuffer = addBuffer();
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

	expected<void, std::string> RenderChain::render() noexcept
	{
		if (empty())
		{
			return {};
		}
		if (!_viewId)
		{
			return unexpected<std::string>{"empty viewId"};
		}

		std::vector<std::string> errors;
		auto encoder = bgfx::begin();
		for (auto& step : _steps)
		{
			auto result = step->render(*encoder);
			if(!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		encoder->touch(_viewId.value());
		bgfx::end(encoder);
		return StringUtils::joinExpectedErrors(errors);
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name, int priority) noexcept
		: _program{ prog }
		, _name{ name }
		, _priority{ priority }
		, _texUniform{ bgfx::kInvalidHandle }
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
		if (isValid(_texUniform))
		{
			bgfx::destroy(_texUniform);
		}
		_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		_basicUniforms.init();

		const Rectangle screen{ glm::uvec2{2} };
		_mesh = std::make_unique<Mesh>(MeshData{ screen }.createMesh(_program->getVertexLayout()));
		return {};
	}

	expected<void, std::string> ScreenSpaceRenderPass::shutdown() noexcept
	{
		if (isValid(_texUniform))
		{
			bgfx::destroy(_texUniform);
			_texUniform.idx = bgfx::kInvalidHandle;
		}
		_basicUniforms.shutdown();
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

		_mesh->render(encoder);

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