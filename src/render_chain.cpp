#include <darmok/render_chain.hpp>
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	TextureConfig FrameBuffer::createColorConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::RGBA16F;
		return config;
	}

	TextureConfig FrameBuffer::createDepthConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::D16F;
		return config;
	}

	FrameBuffer::FrameBuffer(const glm::uvec2& size, bool depth) noexcept
		: _colorTex(std::make_shared<Texture>(createColorConfig(size), BGFX_TEXTURE_RT))
		, _depthTex(depth ? std::make_shared<Texture>(createDepthConfig(size), BGFX_TEXTURE_RT) : nullptr)
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles = { _colorTex->getHandle() };
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
		: _handle(other._handle)
		, _colorTex(std::move(other._colorTex))
		, _depthTex(std::move(other._depthTex))
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

	const glm::uvec2& FrameBuffer::getSize() const noexcept
	{
		return _colorTex->getSize();
	}

	void FrameBuffer::configureView(bgfx::ViewId viewId) const noexcept
	{
		bgfx::setViewFrameBuffer(viewId, _handle);
	}

	RenderChain::RenderChain(IRenderChainDelegate& dlg) noexcept
		: _delegate(dlg)
		, _running(false)
	{
	}

	RenderChain::RenderChain(std::unique_ptr<IRenderChainDelegate>&& dlg) noexcept
		: _delegate(*dlg)
		, _delegatePointer(std::move(dlg))
		, _running(false)
	{
	}

	void RenderChain::init()
	{
		_running = true;
		for (size_t i = 0; i < _steps.size(); ++i)
		{
			auto& step = _steps[i];
			step->init(*this);
			step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
		}
	}

	void RenderChain::shutdown()
	{
		for (auto itr = _steps.rbegin(); itr != _steps.rend(); ++itr)
		{
			(*itr)->shutdown();
		}
		_running = false;
		_viewId.reset();
		_steps.clear();
		_buffers.clear();
		_output.reset();
	}

	void RenderChain::update(float deltaTime)
	{
		for (auto& step : _steps)
		{
			step->update(deltaTime);
		}
	}

	RenderChain& RenderChain::setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept
	{
		if (_output == fb)
		{
			return *this;
		}
		_output = fb;
		if (_running)
		{
			auto size = _steps.size();
			if (size > 0)
			{
				updateStep(size - 1);
			}
		}
		return *this;
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

	bgfx::ViewId RenderChain::renderReset(bgfx::ViewId viewId) noexcept
	{
		if (_running)
		{
			for (size_t i = 0; i < _steps.size(); ++i)
			{
				auto& step = _steps[i];
				step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
				viewId = step->renderReset(viewId);
			}
		}

		bgfx::setViewName(viewId, "RenderChain clear");

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

	void RenderChain::configureView(bgfx::ViewId viewId, OptionalRef<const FrameBuffer> writeBuffer) const
	{
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

	RenderChain& RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		auto& readBuffer = addBuffer();
		auto& ref = *step;
		auto i = _steps.size();
		_steps.push_back(std::move(step));
		if (!_running)
		{
			return *this;
		}

		ref.init(*this);
		ref.updateRenderChain(readBuffer, getWriteBuffer(i));

		if (i > 0)
		{
			updateStep(i - 1);
		}
		else
		{
			_delegate.onRenderChainInputChanged();
		}

		return *this;
	}

	void RenderChain::updateStep(size_t i)
	{
		if (i < 0 || i >= _steps.size())
		{
			return;
		}
		auto& step = _steps[i];
		step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
	}

	bool RenderChain::removeStep(const IRenderChainStep& step) noexcept
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
			(*itr)->shutdown();
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
			updateStep(j);
		}
		if (i > 0)
		{
			updateStep(i - 1);
		}
		else
		{
			_delegate.onRenderChainInputChanged();
		}

		return true;
	}

	bool RenderChain::empty() const noexcept
	{
		return _buffers.empty() && !_output;
	}

	void RenderChain::render() noexcept
	{
		if (empty() || !_viewId)
		{
			return;
		}

		auto encoder = bgfx::begin();
		for (auto& step : _steps)
		{
			step->render(*encoder);
		}
		encoder->touch(_viewId.value());
		bgfx::end(encoder);
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name, int priority)
		: _program(prog)
		, _name(name)
		, _priority(priority)
		, _texUniform{ bgfx::kInvalidHandle }
	{
		if (!prog)
		{
			throw std::runtime_error("empty program");
		}
		if (_name.empty())
		{
			_name = "ScreenSpaceRenderPass " + prog->getName();
		}
	}

	ScreenSpaceRenderPass::~ScreenSpaceRenderPass() noexcept
	{
		// empty on purpose
		shutdown();
	}

	void ScreenSpaceRenderPass::init(RenderChain& chain) noexcept
	{
		_chain = chain;

		if (isValid(_texUniform))
		{
			bgfx::destroy(_texUniform);
		}
		_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		_basicUniforms.init();

		Rectangle screen(glm::uvec2(2));
		_mesh = MeshData(screen).createMesh(_program->getVertexLayout());
	}

	void ScreenSpaceRenderPass::shutdown() noexcept
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
	}

	void ScreenSpaceRenderPass::updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept
	{
		_readTex = read;
		_writeTex = write;
		if (write && _viewId)
		{
			write->configureView(_viewId.value());
		}
	}

	bgfx::ViewId ScreenSpaceRenderPass::renderReset(bgfx::ViewId viewId) noexcept
	{
		bgfx::setViewName(viewId, _name.c_str());
		if (_chain)
		{
			_chain->configureView(viewId, _writeTex);
		}
		_viewId = viewId;
		return ++viewId;
	}

	void ScreenSpaceRenderPass::update(float deltaTime) noexcept
	{
		_basicUniforms.update(deltaTime);
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept
	{
		_textureUniforms.set(name, stage, texture);
		return *this;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setUniform(const std::string& name, std::optional<UniformValue> value) noexcept
	{
		_uniforms.set(name, value);
		return *this;
	}

	void ScreenSpaceRenderPass::render(bgfx::Encoder& encoder) noexcept
	{
		if (!_viewId)
		{
			return;
		}
		auto viewId = _viewId.value();
		if (!_mesh || !_program || !_readTex)
		{
			encoder.touch(viewId);
			return;
		}

		_mesh->render(encoder);

		encoder.setTexture(0, _texUniform, _readTex->getTexture()->getHandle());
		_basicUniforms.configure(encoder);
		_uniforms.configure(encoder);
		_textureUniforms.configure(encoder);

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_BLEND_ALPHA
			;
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}
}